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
#include "hlib/file.hpp"
#include "hlib/result.hpp"
#include "hlib/time.hpp"
#include <condition_variable>
#include <functional>
#include <mutex>
#include <sys/epoll.h>
#include <thread>
#include <unordered_map>

namespace hlib
{

class EventLoop final
{
    HLIB_NOT_COPYABLE(EventLoop);
    HLIB_NOT_MOVABLE(EventLoop);

public:
    static constexpr std::uint32_t Read{ EPOLLIN };
    static constexpr std::uint32_t Write{ EPOLLOUT };
    static constexpr std::uint32_t Error{ EPOLLERR };
    static constexpr std::uint32_t Hup{ EPOLLHUP };
    static constexpr std::uint32_t RdHup{ EPOLLRDHUP };

    typedef std::function<void(int fd, std::uint32_t events)> Callback;

public:
    EventLoop();

    int fd() const noexcept;
    std::thread::id threadId() const noexcept;

    void add(int fd, std::uint32_t events, Callback callback);
    Result<> modify(int fd, std::uint32_t events, std::nothrow_t) noexcept;
    void modify(int fd, std::uint32_t events);
    void change(int fd, Callback callback);
    void remove(int fd);

    void dispatch();
    void dispatch(time::Duration const& timeout);

    Result<> interrupt(std::nothrow_t) noexcept;
    void interrupt();
    void flush() noexcept;

private:
    Handle<int, -1> m_fd;
    file::Pipe m_pipe;
    bool m_interrupt{ false };

    std::mutex m_mutex;

    std::thread::id m_thread_id;
    std::unordered_map<int, std::shared_ptr<Callback>> m_callbacks;

    void dispatch(time::Duration const* timeout);
};

bool callback_from(EventLoop const& event_loop);
bool callback_from(std::weak_ptr<EventLoop> const& event_loop);

} // namespace hlib

