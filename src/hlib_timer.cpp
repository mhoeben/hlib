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
#include "hlib/timer.hpp"
#include "hlib/event_loop.hpp"
#include "hlib/error.hpp"
#include "hlib/memory.hpp"
#include "hlib/utility.hpp"
#include <sys/timerfd.h>
#include <unistd.h>

using namespace hlib;

namespace
{

constexpr long immediate_nsec = 1000;

} // namespace

//
// Implementation
//
void Timer::onExpire(int fd, std::uint32_t /* events */)
{
    std::uint64_t data;
    ssize_t r = read(fd, &data, sizeof(data));
    if (-1 == r) {
        throw make_system_error(errno, "read() failed");
    }
    assert(sizeof(data) == r);

    m_callback();
}

//
// Public
//
Timer::Timer(std::weak_ptr<EventLoop> event_loop, Callback callback)
    : m_event_loop(std::move(event_loop))
    , m_callback(std::move(callback))
    , m_fd{ timerfd_create(CLOCK_MONOTONIC, 0) }
{
    assert(-1 != m_fd);

    with_weak_ptr_locked<>(m_event_loop, [this](EventLoop& loop) {
        loop.add(
            m_fd,
            EventLoop::Read,
            std::bind(&Timer::onExpire, this, std::placeholders::_1, std::placeholders::_2)
        );
    });
}

Timer::Timer(std::weak_ptr<EventLoop> event_loop, Callback callback,
        time::Duration const& expire, time::Duration const& interval)
    : Timer(std::move(event_loop), std::move(callback))
{
    set(expire, interval);
}

Timer::~Timer()
{
    if (-1 == m_fd) {
        return;
    }

    with_weak_ptr_locked<>(m_event_loop, [this](EventLoop& loop) {
        loop.remove(m_fd);
    });

    if (-1 == m_fd) {
        close(m_fd);
    }
}

bool Timer::clear() noexcept
{
    itimerspec ts = {};

    if (-1 == timerfd_settime(m_fd, 0, &ts, nullptr)) {
        return false;
    }

    return true;
}

bool Timer::set(time::Duration const& expire, time::Duration const& interval) noexcept
{
    itimerspec ts = {};
    ts.it_value = expire;
    ts.it_interval = interval;

    // Replace 0 expire with a small timer value so instead as to cancel
    // the timer, it almost immediately expires.
    if (0 == ts.it_value.tv_sec && 0 == ts.it_value.tv_nsec) {
        ts.it_value.tv_nsec = immediate_nsec;
    }

    if (-1 == timerfd_settime(m_fd, 0, &ts, nullptr)) {
        return false;
    }

    return true;
}

