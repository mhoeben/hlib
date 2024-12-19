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
#include "hlib/socket.hpp"
#include "hlib/lock.hpp"
#include "hlib/error.hpp"
#include "hlib/event_loop.hpp"
#include "hlib/file.hpp"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

using namespace hlib;

//
// Implementation
//
namespace
{

bool set_option(int fd, int option, int value = 1) noexcept
{
    return 0 == setsockopt(fd, SOL_SOCKET, option, &value, sizeof(value));
}

Result<> set_reuse(int fd, std::uint32_t options) noexcept
{
    if (0 != (Socket::ReuseAddr & options)
     && false == set_option(fd, SO_REUSEADDR, 1)) {
        return make_system_error(errno, "setsocktopt(SO_REUSEADDR) failed");
    }

    if (0 != (Socket::ReusePort & options)
     && false == set_option(fd, SO_REUSEPORT, 1)) {
        return make_system_error(errno, "setsocktopt(SO_REUSEPORT) failed");
    }

    return {};
}

} // namespace

void Socket::updateEventsLocked(std::uint32_t events) noexcept
{
    if (true == m_connected) {
        // Lock event loop and modify events.
        with_weak_ptr_locked(m_event_loop, [&](EventLoop& loop) {
            loop.modify(m_fd.get(), events);
        });
    }

    m_events = events;
}

void Socket::onAccept(int fd, std::uint32_t events)
{
    assert(m_fd.get() == fd);
    assert(nullptr != m_on_accept);

    // Error condition while listening?
    if (0 != (EPOLLERR & events)) {
        callbackAndClose(errno);
    }

    SockAddr address;
    socklen_t length = address.length();

    // Accept socket and peer address.
    Handle<int, -1> socket(
        accept(fd, static_cast<sockaddr*>(address), &length),
        file::fd_close
    );
    if (-1 == socket.get()) {
        callbackAndClose(errno);
        return;
    }

    // Make non-blocking.
    if (true == file::fd_set_non_blocking(socket.get(), true, std::nothrow).failure()) {
        callbackAndClose(errno);
        return;
    }

    // Callback.
    m_on_accept(std::move(socket), address);
}

void Socket::onConnect(int fd, std::uint32_t /* events */)
{
    using namespace std::placeholders;

    // Determine whether socket connect was successful.
    int error = get_socket_error(fd);
    if (0 != error) {
        callbackAndClose(error);
        return;
    }

    // Change event loop registration.
    bool success = with_weak_ptr_locked(m_event_loop, [&](EventLoop& loop) {
        loop.remove(fd);
        loop.add(fd, m_events, std::bind(&Socket::onEvent, this, _1, _2));
    });
    if (false == success) {
        callbackAndClose(ENODEV);
        return;
    }

    // Connected.
    m_connected = true;

    // Callback.
    if (nullptr != m_on_connected) {
        m_on_connected();
    }
}

void Socket::onEvent(int fd, std::uint32_t events)
{
    assert(m_fd.get() == fd);
    assert(true == m_connected);

    if (0 != ((EventLoop::Error|EventLoop::Hup) & events)) {
        // A socket error occurred, callback and close socket.
        callbackAndClose(get_socket_error(fd));
        return;
    }

    if (0 != (EventLoop::Read & events)) {
        HLIB_UNIQUE_LOCK(lock, m_mutex);

        auto completed = [this, &lock]
        {
            // Completed receive, disable read event.
            updateEventsLocked(m_events & ~(EventLoop::Read));

            auto sink = std::move(m_receive_sink);
            auto callback = std::move(m_receive_callback);

            lock.unlock();

            if (nullptr != callback) {
                callback(sink);
            }
        };

        assert(nullptr != m_receive_sink);

        // Get number of bytes available.
        int available;
        if (-1 == ioctl(fd, FIONREAD, &available)) {
            lock.unlock();
            callbackAndClose(errno);
            return;
        }

        // Resize sink by headroom, limited by the available bytes in the receive buffer.
        std::size_t headroom = m_receive_sink->headroom(available);
        std::size_t unextended = m_receive_sink->size();
        void* ptr = m_receive_sink->produce(headroom);
        if (nullptr == ptr) {
            lock.unlock();

            // Something went wrong.
            callbackAndClose(ENOMEM);
            return;
        }

        // Progressively receive to sink.
        ssize_t size = ::recv(fd, ptr, headroom, 0);
        switch (size) {
        case -1:
            lock.unlock();

            // Something went wrong.
            callbackAndClose(errno);
            return;

        case 0:
            // Socket closed.
            completed();
            callbackAndClose(0);
            return;

        default:
            assert(size > 0);

            // Resize to actual size.
            m_receive_sink->resize(unextended + size);

            // All data received?
            if (true == m_receive_sink->full()) {
                completed();
            }
        }
    }

    if (0 != (EventLoop::Write & events)) {
        HLIB_UNIQUE_LOCK(lock, m_mutex);

        // Get source at the front of the queue.
        assert(false == m_send_queue.empty());
        Source& source = *m_send_queue.front().source;

        // Progressively send from source.
        ssize_t size = ::send(fd, source.peek(source.available()), source.available(), 0);
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
            auto completed = m_send_queue.front();
            m_send_queue.pop_front();

            // Disable write events on empty send queue.
            if (true == m_send_queue.empty()) {
                updateEventsLocked(m_events & ~(EventLoop::Write));
            }

            lock.unlock();

            // Callback completed sink.
            if (nullptr != completed.callback) {
                completed.callback(completed.source);
            }
        }
    }
}

void Socket::callbackAndClose(int error)
{
    if (nullptr != m_on_close) {
        m_on_close(error);
    }

    close();
}

//
// Public
//
Socket::Socket(std::weak_ptr<EventLoop> event_loop) noexcept
    : m_event_loop(std::move(event_loop))
    , m_fd(file::fd_close)
{
}

Socket::Socket(std::weak_ptr<EventLoop> event_loop, Handle<int, -1> fd) noexcept
    : Socket(std::move(event_loop))
{
    open(std::move(fd));
}

Socket::~Socket()
{
    close();
}

int Socket::fd() const noexcept
{
    return m_fd.get();
}

bool Socket::connected() const noexcept
{
    return m_connected;
}

SockAddr Socket::getPeerAddress() const noexcept
{
    sockaddr_storage storage{};
    socklen_t length = sizeof(storage);

    if (-1 == getpeername(m_fd.get(), reinterpret_cast<sockaddr*>(&storage), &length)) {
        return SockAddr();
    }

    return SockAddr(storage);
}

void Socket::setAcceptCallback(OnAccept callback) noexcept
{
    m_on_accept = std::move(callback);
}

void Socket::setConnectedCallback(OnConnected callback) noexcept
{
    m_on_connected = std::move(callback);
}

void Socket::setCloseCallback(OnClose callback) noexcept
{
    m_on_close = std::move(callback);
}

Result<> Socket::open(Handle<int, -1> fd, std::nothrow_t) noexcept
{
    using namespace std::placeholders;

    close();

    m_events = EventLoop::Read;

    // Add socket's file descriptor to event loop.
    bool success = with_weak_ptr_locked(m_event_loop, [&](EventLoop& loop) {
        loop.add(fd.get(), m_events, std::bind(&Socket::onEvent, this, _1, _2));
    });
    if (false == success) {
        return make_system_error(ENODEV, "Failed to lock event loop");
    }

    // Store socket's file descriptor and signal it is connected.
    m_fd = std::move(fd);
    m_connected = 0 == get_socket_error(m_fd.get());
    return {};
}

void Socket::open(Handle<int, -1> fd)
{
    success_or_throw<>(Socket::open(std::move(fd), std::nothrow));
}

Result<> Socket::listen(SockAddr const& address, int type, int protocol, int backlog, std::uint32_t options, std::nothrow_t) noexcept
{
    using namespace std::placeholders;

    close();

    m_events = EventLoop::Read;

    // Create socket.
    Handle<int, -1> fd(
        ::socket(address.family(), type, protocol),
        file::fd_close
    );
    if (-1 == fd.get()) {
        return make_system_error(errno, "socket() failed");
    }

    // Set close-on-exec on listening sockets.
    if (-1 == fcntl(fd.get(), F_SETFD, FD_CLOEXEC)) {
        return make_system_error(errno, "fcntl() failed");
    }

    Result<> result;

    // Make non-blocking.
    result = file::fd_set_non_blocking(fd.get(), true, std::nothrow);
    if (true == result.failure()) {
        return result;
    }

    // Set reuse options.
    result = set_reuse(fd.get(), options);
    if (true == result.failure()) {
        return result;
    }

    // Bind socket to address.
    if (-1 == ::bind(fd.get(), static_cast<sockaddr const*>(address), address.length())) {
        return make_system_error(errno, "bind() failed");
    }

    // Listen for incoming connections
    if (-1 == ::listen(fd.get(), backlog)) {
        return make_system_error(errno, "listen() failed");
    }

    // Add file descriptor to event loop.
    bool success = with_weak_ptr_locked(m_event_loop, [&](EventLoop& loop) {
        loop.add(fd.get(), m_events, std::bind(&Socket::onAccept, this, _1, _2));
    });
    if (false == success) {
        return make_system_error(ENODEV, "Event loop not available");
    }

    // Commit file descriptor.
    m_fd = std::move(fd);
    return {};
}

void Socket::listen(SockAddr const& address, int type, int protocol, int backlog, std::uint32_t options)
{
    success_or_throw<>(Socket::listen(address, type, protocol, backlog, options, std::nothrow));
}

Result<> Socket::connect(SockAddr const& address, int type, int protocol, std::uint32_t options, std::nothrow_t) noexcept
{
    using namespace std::placeholders;

    close();

    EventLoop::Callback callback = std::bind(&Socket::onEvent, this, _1, _2);
    std::uint32_t events = EventLoop::Read;

    m_events = events;

    // Create socket.
    Handle<int, -1> fd(
        ::socket(address.family(), type, protocol),
        file::fd_close
    );
    if (-1 == fd.get()) {
        return make_system_error(errno, "socket() failed");
    }

    Result<> result;

    // Make non-blocking.
    result = file::fd_set_non_blocking(fd.get(), true, std::nothrow);
    if (true == result.failure()) {
        return result;
    }

    // Set reuse options.
    result = set_reuse(fd.get(), options);
    if (true == result.failure()) {
        return result;
    }

    // Connect to peer address.
    if (-1 == ::connect(fd.get(), static_cast<sockaddr const*>(address), address.length())) {
        if (EINPROGRESS != errno) {
            return make_system_error(errno, "connect() failed");
        }

        callback = std::bind(&Socket::onConnect, this, _1, _2);
        events |= EventLoop::Write;
    }
    else {
        m_connected = true;
    }

    // Add file descriptor to event loop.
    bool success = with_weak_ptr_locked(m_event_loop, [&](EventLoop& loop) {
        loop.add(fd.get(), events, std::move(callback));
    });
    if (false == success) {
        return make_system_error(ENODEV, "Event loop not available");
    }

    // Commit file descriptor.
    m_fd = std::move(fd);

    // Callback connected, if connected.
    if (true == m_connected && nullptr != m_on_connected) {
        m_on_connected();
    }

    return {};
}

void Socket::connect(SockAddr const& address, int type, int protocol, std::uint32_t options)
{
    success_or_throw<>(connect(address, type, protocol, options, std::nothrow));
}

void Socket::receive(std::shared_ptr<Sink> sink, OnReceived callback)
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

    m_receive_sink = std::move(sink);
    m_receive_callback = std::move(callback);
}

void Socket::send(std::shared_ptr<Source> source, OnSent callback)
{
    HLIB_LOCK_GUARD(lock, m_mutex);

    std::uint32_t events = m_events;
    if (true == m_send_queue.empty()) {
        events |= EventLoop::Write;
    }

    m_send_queue.emplace_back(SendTuple{ std::move(source), std::move(callback) });

    if (events != m_events) {
        updateEventsLocked(events);
    }
}

void Socket::send(std::shared_ptr<Source> source)
{
    send(std::move(source), OnSent());
}

void Socket::close() noexcept
{
    HLIB_LOCK_GUARD(lock, m_mutex);

    if (-1 == m_fd.get()) {
        return;
    }

    with_weak_ptr_locked(m_event_loop, [&](EventLoop& loop) {
        loop.remove(m_fd.get());
    });

    m_fd.reset();

    m_connected = false;
    m_events = 0;

    m_receive_sink.reset();
    m_receive_callback = nullptr;

    m_send_queue.clear();
}

bool hlib::is_socket(int fd) noexcept
{
    struct stat st;

    if (-1 == fstat(fd, &st)) {
        return false;
    }

    return S_ISSOCK(st.st_mode);
}

int hlib::get_socket_error(int fd) noexcept
{
    int error;
    socklen_t length = sizeof(error);

    if (-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &length)) {
        return errno;
    }

    return error;
}

