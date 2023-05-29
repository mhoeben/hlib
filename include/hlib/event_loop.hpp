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
#include "hlib/log.hpp"
#include <array>
#include <functional>
#include <mutex>
#include <sys/epoll.h>
#include <unordered_map>

namespace hlib
{

class EventLoop final
{
    HLIB_NOT_COPYABLE(EventLoop);
    HLIB_NOT_MOVABLE(EventLoop);

public:
    static constexpr uint32_t kRead{ EPOLLIN };
    static constexpr uint32_t kWrite{ EPOLLOUT };

    typedef std::function<void(int fd, uint32_t events)> Callback;

public:
    EventLoop(std::string m_name = std::string());
    ~EventLoop();

    int fd() const noexcept;

    void add(int fd, uint32_t events, Callback callback);
    void modify(int fd, uint32_t events);
    void remove(int fd);

    void dispatch(int timeout_ms = 0);
    void interrupt();

private:
    log::Domain const m_logger;
    std::string m_name;
    int m_fd{ -1 };
    std::array<int, 2> m_pipe{ -1, -1 };
    bool m_interrupt{ false };

    std::mutex m_mutex;
    std::unordered_map<int, Callback> m_callbacks;
};

} // namespace hlib

