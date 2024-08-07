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
#pragma once

#include "hlib/base.hpp"
#include "hlib/time.hpp"
#include <ctime>
#include <functional>
#include <memory>

namespace hlib
{

class EventLoop;

class Timer final
{
    HLIB_NOT_COPYABLE(Timer);
    HLIB_NOT_MOVABLE(Timer);

public:
    typedef std::function<void()> Callback;

public:
    Timer(std::weak_ptr<EventLoop> event_loop, Callback callback);
    Timer(std::weak_ptr<EventLoop> event_loop, Callback callback,
        time::Duration const& expire, time::Duration const& interval = {});
    ~Timer();

    bool clear() noexcept;
    bool set(time::Duration const& expire, time::Duration const& interval = {}) noexcept;

private:
    std::weak_ptr<EventLoop> m_event_loop;
    Callback m_callback;

    int m_fd{ -1 };

    void onExpire(int fd, std::uint32_t events);
};

} // namespace hlib

