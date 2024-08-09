//
// MIT License
//
// Copyright (c) 2024 Maarten Hoeben
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
#include "hlib/event_loop.hpp"
#include "hlib/memory.hpp"
#include "hlib/sink.hpp"
#include "hlib/source.hpp"
#include <deque>
#include <string>

namespace hlib
{

class FileDescriptorIO
{
    HLIB_NOT_COPYABLE(FileDescriptorIO);
    HLIB_NOT_MOVABLE(FileDescriptorIO);

public:
    typedef std::function<void(std::shared_ptr<Sink> const& sink)> OnRead;
    typedef std::function<void(std::shared_ptr<Source> const& source)> OnWritten;
    typedef std::function<void(int error)> OnClose;

public:
    std::shared_ptr<void> user;

    FileDescriptorIO(std::weak_ptr<EventLoop> event_loop) noexcept;
    FileDescriptorIO(std::weak_ptr<EventLoop> event_loop, Handle<int, -1> fd) noexcept;
    virtual ~FileDescriptorIO();

    int fd() const noexcept;

    void setCloseCallback(OnClose callback) noexcept;

    virtual Result<> open(Handle<int, -1> fd, std::nothrow_t) noexcept;
    void open(Handle<int, -1> fd);

    Result<> open(std::string const& filepath, int flags, std::nothrow_t) noexcept;
    void open(std::string const& filepath, int flags);

    virtual void read(std::shared_ptr<Sink> sink, OnRead callback);
            void read(std::shared_ptr<Sink> sink);
    virtual void write(std::shared_ptr<Source> source, OnWritten callback);
            void write(std::shared_ptr<Source> source);

    virtual void close() noexcept;

private:
    std::weak_ptr<EventLoop> m_event_loop;
    Handle<int, -1> m_fd;

    OnClose m_on_close;

    std::mutex m_mutex;
    std::uint32_t m_events{ 0 };

    std::shared_ptr<Sink> m_read_sink;
    OnRead m_read_callback;

    struct SendTuple
    {
        std::shared_ptr<Source> source;
        OnWritten callback;
    };
    std::deque<SendTuple> m_write_queue;

    void updateEventsLocked(std::uint32_t events) noexcept;
    void onEvent(int fd, std::uint32_t events);
    void callbackAndClose(int error);
};

} // namespace hlib

