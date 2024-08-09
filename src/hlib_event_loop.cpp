//
// MIT License
//
// Copyright (c) 2023 Maarten Hoeben
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include "hlib/event_loop.hpp"
#include "hlib/error.hpp"
#include "hlib/lock.hpp"
#include "hlib/memory.hpp"
#include "hlib/scope_guard.hpp"
#include "hlib/time.hpp"
#include "hlib/utility.hpp"
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>

using namespace hlib;

//
// Implementation
//
void EventLoop::dispatch(time::Duration const* timeout)
{
    ScopeGuard thread_scope(
        [this]{
            HLIB_LOCK_GUARD(lock, m_mutex);
            m_thread_id = std::this_thread::get_id();
        },
        [this]{
            HLIB_LOCK_GUARD(lock, m_mutex);
            m_thread_id = std::thread::id();
        }
    );

    time::MSec timeout_ms(-1);
    time::Clock expire;

    if (nullptr != timeout) {
        expire = time::now() + *timeout;
    }

    HLIB_UNIQUE_LOCK_DEFERRED(lock, m_mutex);
    std::shared_ptr<Callback> callback;

    m_interrupt = false;

    do {
        if (true == m_interrupt) {
            break;
        }

        if (nullptr != timeout) {
            time::Clock const current = time::now();
            if (current < expire) {
                timeout_ms = (expire - current).to<time::MSec>();
            }
            else {
                timeout_ms = time::MSec(0);
            }
        }

        epoll_event event;
        switch (epoll_wait(m_fd.get(), &event, 1, timeout_ms.value())) {
        case -1:
            if (EINTR != errno) {
                throw make_system_error(errno, "epoll_wait() failed");
            }
            break;

        case 1:
            lock.lock();
            {
                auto it = m_callbacks.find(event.data.fd);
                if (m_callbacks.end() == it) {
                    break;
                }

                callback = it->second;
            }
            lock.unlock();

            (*callback)(event.data.fd, event.events);
            break;

        default:
            assert(timeout_ms >= time::MSec(0));
            return;
        }
    }
    while (true);
}


//
// Public
//
EventLoop::EventLoop()
    : m_fd(epoll_create1(0), file::fd_close)
{
    if (-1 == m_fd.get()) {
        throw make_system_error(errno, "epoll_create() failed");
    }
    m_pipe.open();

    int flags = fcntl(m_pipe[0], F_GETFL, 0);
    if (-1 == flags
     || -1 == fcntl(m_pipe[0], F_SETFL, flags|O_NONBLOCK)) {
        throw make_system_error(errno, "fcntl() failed");
    }

    add(m_pipe[0], Read, [this](int fd, std::uint32_t events) {
        assert(0 != (EPOLLIN & events));
        (void)events;

        std::uint8_t cmd;
        HVERIFY(1 == read(fd, &cmd, 1));

        m_interrupt = true;
    });
}

int EventLoop::fd() const noexcept
{
    return m_fd.get();
}

std::thread::id EventLoop::threadId() const noexcept
{
    return m_thread_id;
}

void EventLoop::add(int fd, std::uint32_t events, Callback callback)
{
    assert(-1 != fd);
    assert(nullptr != callback);

    HLIB_LOCK_GUARD(lock, m_mutex);

    assert(m_callbacks.end() == m_callbacks.find(fd));

    try {
        m_callbacks.emplace(fd, std::make_shared<Callback>(std::move(callback)));

        epoll_event event{};
        event.events = events;
        event.data.fd = fd;
        if (-1 == epoll_ctl(m_fd.get(), EPOLL_CTL_ADD, fd, &event)) {
            throw make_system_error(errno, "epoll_ctl() failed");
        }
    }
    catch (...) {
        m_callbacks.erase(fd);
        throw;
    }
}

Result<> EventLoop::modify(int fd, std::uint32_t events, std::nothrow_t) noexcept
{
    assert(-1 != fd);

    epoll_event event{};
    event.events = events;
    event.data.fd = fd;
    if (-1 == epoll_ctl(m_fd.get(), EPOLL_CTL_MOD, fd, &event)) {
        return make_system_error(errno);
    }

    return {};
}

void EventLoop::modify(int fd, std::uint32_t events)
{
    success_or_throw<>(modify(fd, events, std::nothrow));
}

void EventLoop::change(int fd, Callback callback)
{
    assert(-1 != fd);

    HLIB_UNIQUE_LOCK(lock, m_mutex);

    auto it = m_callbacks.find(fd);
    assert(m_callbacks.end() != it);

    it->second = std::make_shared<Callback>(std::move(callback));
}

void EventLoop::remove(int fd)
{
    if (-1 == fd) {
        return;
    }

    HLIB_UNIQUE_LOCK(lock, m_mutex);

    assert(m_callbacks.end() != m_callbacks.find(fd));

    struct epoll_event event;
    event.events = 0;
    event.data.fd = fd;
    HVERIFY(-1 != epoll_ctl(m_fd.get(), EPOLL_CTL_DEL, fd, &event));

    m_callbacks.erase(fd);
}

void EventLoop::dispatch()
{
    dispatch(nullptr);
}

void EventLoop::dispatch(time::Duration const& timeout)
{
    dispatch(&timeout);
}

Result<> EventLoop::interrupt(std::nothrow_t) noexcept
{
    std::uint8_t const cmd = 0;

    if (1 != write(m_pipe[1], &cmd, 1)) {
        return make_system_error(errno);
    }

    return {};
}

void EventLoop::interrupt()
{
    success_or_throw<>(interrupt(std::nothrow));
}

void EventLoop::flush() noexcept
{
    std::uint8_t cmd;
    while (1 == read(m_pipe[0], &cmd, 1)) {
    }
}

//
// Utility
//
bool hlib::callback_from(EventLoop const& event_loop)
{
    return std::this_thread::get_id() == event_loop.threadId();
}

bool hlib::callback_from(std::weak_ptr<EventLoop> const& event_loop)
{
    bool result;

    with_weak_ptr_locked(event_loop, [&](EventLoop const& loop) {
        result = callback_from(loop);
    });

    return result;
}

