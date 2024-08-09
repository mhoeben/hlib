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
#include "hlib/fdio.hpp"
#include "hlib/lock.hpp"
#include "hlib/error.hpp"
#include "hlib/event_loop.hpp"
#include "hlib/file.hpp"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

using namespace hlib;

//
// Implementation
//
void FileDescriptorIO::updateEventsLocked(std::uint32_t events) noexcept
{
    // Lock event loop and modify events.
    with_weak_ptr_locked(m_event_loop, [&](EventLoop& loop) {
        loop.modify(m_fd.get(), events);
    });

    m_events = events;
}

void FileDescriptorIO::onEvent(int fd, std::uint32_t events)
{
    assert(m_fd.get() == fd);

    if (0 != (EventLoop::Read & events)) {
        HLIB_UNIQUE_LOCK(lock, m_mutex);

        auto completed = [this, &lock]
        {
            // Completed receive, disable read event.
            updateEventsLocked(m_events & ~(EventLoop::Read));

            auto sink = std::move(m_read_sink);
            auto callback = std::move(m_read_callback);

            lock.unlock();

            if (nullptr != callback) {
                callback(sink);
            }
        };

        assert(nullptr != m_read_sink);

        // Get number of bytes available.
        int available;
        if (-1 == ioctl(fd, FIONREAD, &available)) {
            lock.unlock();
            callbackAndClose(errno);
            return;
        }

        // Resize sink by headroom, limited by the available bytes in the receive buffer.
        std::size_t headroom = m_read_sink->headroom(available);
        std::size_t unextended = m_read_sink->size();
        void* ptr = m_read_sink->produce(headroom);
        if (nullptr == ptr) {
            lock.unlock();

            // Something went wrong.
            callbackAndClose(ENOMEM);
            return;
        }

        // Progressively receive to sink.
        ssize_t size = ::read(fd, ptr, headroom);
        switch (size) {
        case -1:
            lock.unlock();

            // Something went wrong.
            callbackAndClose(errno);
            return;

        case 0:
            // FileDescriptorIO closed.
            completed();
            callbackAndClose(0);
            return;

        default:
            assert(size > 0);

            // Resize to actual size.
            m_read_sink->resize(unextended + size);

            // All data received?
            if (true == m_read_sink->full()) {
                completed();
            }
        }
    }

    if (0 != (EventLoop::Write & events)) {
        HLIB_UNIQUE_LOCK(lock, m_mutex);

        // Get source at the front of the queue.
        assert(false == m_write_queue.empty());
        Source& source = *m_write_queue.front().source;

        // Progressively send from source.
        ssize_t size = ::write(fd, source.peek(source.available()), source.available());
        if (-1 == size) {
            lock.unlock();

            // Something went wrong.
            callbackAndClose(errno);
            return;
        }

        // Consume bytes sent.
        (void)source.consume(size);

        // Everything sent?
        if (true == source.empty()) {
            auto completed = m_write_queue.front();
            m_write_queue.pop_front();

            // Disable write events on empty send queue.
            if (true == m_write_queue.empty()) {
                updateEventsLocked(m_events & ~(EventLoop::Write));
            }

            lock.unlock();

            // Callback completed sink.
            if (nullptr != completed.callback) {
                completed.callback(completed.source);
            }
        }
    }

    if (0 != ((EventLoop::Error|EventLoop::Hup) & events)) {
        // A socket error occurred, callback and close socket.
        callbackAndClose(get_socket_error(fd));
        return;
    }

}

void FileDescriptorIO::callbackAndClose(int error)
{
    if (nullptr != m_on_close) {
        m_on_close(error);
    }

    close();
}

//
// Public
//
FileDescriptorIO::FileDescriptorIO(std::weak_ptr<EventLoop> event_loop) noexcept
    : m_event_loop(std::move(event_loop))
    , m_fd(file::fd_close)
{
}

FileDescriptorIO::FileDescriptorIO(std::weak_ptr<EventLoop> event_loop, Handle<int, -1> fd) noexcept
    : FileDescriptorIO(std::move(event_loop))
{
    open(std::move(fd));
}

FileDescriptorIO::~FileDescriptorIO()
{
    close();
}

int FileDescriptorIO::fd() const noexcept
{
    return m_fd.get();
}

void FileDescriptorIO::setCloseCallback(OnClose callback) noexcept
{
    m_on_close = std::move(callback);
}

Result<> FileDescriptorIO::open(Handle<int, -1> fd, std::nothrow_t) noexcept
{
    using namespace std::placeholders;

    close();

    // Add socket's file descriptor to event loop.
    bool success = with_weak_ptr_locked(m_event_loop, [&](EventLoop& loop) {
        loop.add(fd.get(), m_events, std::bind(&FileDescriptorIO::onEvent, this, _1, _2));
    });
    if (false == success) {
        return make_system_error(ENODEV, "Failed to lock event loop");
    }

    // Store socket's file descriptor and signal it is connected.
    m_fd = std::move(fd);
    return {};
}

void FileDescriptorIO::open(Handle<int, -1> fd)
{
    success_or_throw<>(FileDescriptorIO::open(std::move(fd), std::nothrow));
}

Result<> FileDescriptorIO::open(std::string const& filepath, int flags, std::nothrow_t) noexcept
{
    Handle<int, -1> fd(
        ::open(filepath.c_str(), flags),
        file::fd_close
    );
    return open(std::move(fd), std::nothrow);
}

void FileDescriptorIO::open(std::string const& filepath, int flags)
{
    success_or_throw(FileDescriptorIO::open(filepath, flags, std::nothrow));
}

void FileDescriptorIO::read(std::shared_ptr<Sink> sink, OnRead callback)
{
    HLIB_LOCK_GUARD(lock, m_mutex);

    std::uint32_t events = m_events;
    if (nullptr != sink) {
        events |= EventLoop::Read;
    }
    else {
        events &= ~EventLoop::Read;
    }

    if (events != m_events) {
        updateEventsLocked(events);
    }

    m_read_sink = std::move(sink);
    m_read_callback = std::move(callback);
}

void FileDescriptorIO::read(std::shared_ptr<Sink> sink)
{
    read(std::move(sink), OnRead());
}

void FileDescriptorIO::write(std::shared_ptr<Source> source, OnWritten callback)
{
    HLIB_LOCK_GUARD(lock, m_mutex);

    std::uint32_t events = m_events;
    if (true == m_write_queue.empty()) {
        events |= EventLoop::Write;
    }

    m_write_queue.emplace_back(SendTuple{ std::move(source), std::move(callback) });

    if (events != m_events) {
        updateEventsLocked(events);
    }
}

void FileDescriptorIO::write(std::shared_ptr<Source> source)
{
    write(std::move(source), OnWritten());
}

void FileDescriptorIO::close() noexcept
{
    HLIB_LOCK_GUARD(lock, m_mutex);

    if (-1 == m_fd.get()) {
        return;
    }

    with_weak_ptr_locked(m_event_loop, [&](EventLoop& loop) {
        loop.remove(m_fd.get());
    });

    m_fd.reset();

    m_events = 0;

    m_read_sink.reset();
    m_read_callback = nullptr;

    m_write_queue.clear();
}

