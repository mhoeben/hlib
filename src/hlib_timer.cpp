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
#include "hlib/format.hpp"
#include "hlib/memory.hpp"
#include "hlib/utility.hpp"
#include <sys/timerfd.h>
#include <unistd.h>

using namespace hlib;

//
// Implementation
//
void Timer::onExpire(int fd, std::uint32_t /* events */)
{
    std::uint64_t data;
    ssize_t r = read(fd, &data, sizeof(data));
    if (-1 == r) {
        throwf<std::runtime_error>("read failed ({})", get_error_string(errno));
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

Timer::Timer(std::weak_ptr<EventLoop> event_loop, Callback callback, Duration const& expire, Duration const& interval)
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

void Timer::clear()
{
    set({}, {});
}

void Timer::set(Duration const& expire, Duration const& interval)
{
    itimerspec ts = {};
    ts.it_value = expire.timespec;
    ts.it_interval = interval.timespec;

    if (-1 == timerfd_settime(m_fd, 0, &ts, nullptr)) {
        throwf<std::runtime_error>("timerfd_settime failed ({})", get_error_string(errno));
    }
}

