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
#include "hlib/config.hpp"
#include "hlib/lock.hpp"
#include "hlib/error.hpp"
#include "hlib/event_loop.hpp"
#include "hlib/file.hpp"
#include "hlib/format.hpp"

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

bool set_reuse(int fd, std::uint32_t options) noexcept
{
    if (0 != (Socket::ReuseAddr & options)
     && false == set_option(fd, SO_REUSEADDR, 1)) {
        return false;
    }

    if (0 != (Socket::ReusePort & options)
     && false == set_option(fd, SO_REUSEPORT, 1)) {
        return false;
    }

    return true;
}

} // namespace

void Socket::updateEventsLocked(std::uint32_t events) noexcept
{
    if (true == m_connected) {
        // Lock event loop and modify events.
        with_weak_ptr_locked(m_event_loop, [&](EventLoop& loop) {
            loop.modify(m_fd.value(), events);
        });
    }

    m_events = events;
}

void Socket::onAccept(int fd, std::uint32_t events)
{
    assert(m_fd.value() == fd);

    // Error condition while listening?
    if (0 != (EPOLLERR & events)) {
        callbackAndClose(errno);
    }

    SockAddr address;
    socklen_t length = address.length();

    // Accept socket and peer address.
    UniqueOwner<int, -1> socket(
        accept(fd, as<sockaddr>(address), &length),
        file::fd_close
    );
    if (-1 == socket.value()) {
        callbackAndClose(errno);
    }

    if (nullptr == m_on_accept) {
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
    assert(m_fd.value() == fd);
    assert(true == m_connected);

    auto callback_receive = [this]
    {
        // Callback with received buffer.
        assert (nullptr != m_on_receive);
        m_on_receive(m_receive_buffer);

        // Clear receive buffer.
        m_receive_buffer.clear();
    };

    if (0 != ((EventLoop::Error|EventLoop::Hup) & events)) {
        // A socket error occurred, callback and close socket.
        callbackAndClose(get_socket_error(fd));
        return;
    }

    if (0 != (EventLoop::Read & events)) {
        // Reserve up to configured size.
        std::uint8_t* ptr = reserve_as<std::uint8_t>(m_receive_buffer, m_receive_buffer_size);
        std::size_t offset = m_receive_buffer.size();

        assert(offset < m_receive_buffer_size);

        // Progressively receive to buffer.
        ssize_t size = ::recv(fd, ptr + offset, m_receive_buffer_size - offset, 0);
        switch (size) {
        case -1:
            // Something went wrong, callback and close socket.
            callbackAndClose(errno);
            return;

        case 0:
            // Socket closed, callback with buffered data.
            if (0 != m_receive_buffer.size()) {
                callback_receive();
            }

            // Callback no error and close socket.
            callbackAndClose(0);
            break;

        default:
            // Adjust receive buffer for additionally received bytes.
            m_receive_buffer.resize(offset + size);

            // Callback immediately when not gathering or with full buffer
            // when gathering.
            if (false == m_receive_buffer_gather
             || m_receive_buffer.size() == m_receive_buffer_size) {
                callback_receive();
            }
            break;
        }
    }

    if (0 != (EventLoop::Write & events)) {
        HLIB_LOCK_GUARD(lock, m_mutex);

        // Get buffer at the front of the queue.
        assert(false == m_send_queue.empty());
        Buffer& buffer = m_send_queue.front();
        assert(m_send_buffer_offset < buffer.size());

        // Progressively send queued buffers.
        ssize_t size = ::send(fd, buffer.get(m_send_buffer_offset), buffer.size() - m_send_buffer_offset, 0);
        if (-1 == size) {
            callbackAndClose(errno);
            return;
        }

        // Add sent number of bytes and check whether there is more to send.
        m_send_buffer_offset += size;
        if (m_send_buffer_offset < buffer.size()) {
            return;
        }

        // All sent, restart process for next buffer, if available.
        m_send_buffer_offset = 0;
        m_send_queue.pop_front();
        if (true == m_send_queue.empty()) {
            // Send queue is empty, disable write event.
            updateEventsLocked(m_events & ~(EventLoop::Write));
            return;
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
    , m_receive_buffer_size(Config::socketReceiveBufferSize())
    , m_receive_buffer_gather(Config::socketReceiveBufferGather())
{
}

Socket::Socket(std::weak_ptr<EventLoop> event_loop, UniqueOwner<int, -1> fd) noexcept
    : m_event_loop(std::move(event_loop))
    , m_fd(std::move(fd))
    , m_receive_buffer_size(Config::socketReceiveBufferSize())
    , m_receive_buffer_gather(Config::socketReceiveBufferGather())
{
}

Socket::~Socket()
{
    close();
}

int Socket::fd() const noexcept
{
    return m_fd.value();
}

bool Socket::connected() const noexcept
{
    return m_connected;
}

SockAddr Socket::getPeerAddress() const noexcept
{
    sockaddr_storage storage{};
    socklen_t length = sizeof(storage);

    if (-1 == getpeername(m_fd.value(), reinterpret_cast<sockaddr*>(&storage), &length)) {
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

void Socket::setReceiveCallback(OnReceive callback) noexcept
{
    assert(nullptr != callback);

    m_on_receive = std::move(callback);
}

void Socket::setCloseCallback(OnClose callback) noexcept
{
    m_on_close = std::move(callback);
}

void Socket::setReceiveBufferSize(std::size_t size, bool gather) noexcept
{
    assert(size > 0);

    m_receive_buffer_size = size;
    m_receive_buffer_gather = gather;
}

bool Socket::open(UniqueOwner<int, -1> fd, std::nothrow_t) noexcept
{
    using namespace std::placeholders;

    close();

    m_events = EventLoop::Read;

    // Add socket's file descriptor to event loop.
    bool success = with_weak_ptr_locked(m_event_loop, [&](EventLoop& loop) {
        loop.add(fd.value(), m_events, std::bind(&Socket::onEvent, this, _1, _2));
    });
    if (false == success) {
        errno = ENODEV;
        return false;
    }

    // Store socket's file descriptor and signal it is connected.
    m_fd = std::move(fd);
    m_connected = true;
    return true;
}

void Socket::open(UniqueOwner<int, -1> fd)
{
    if (false == Socket::open(std::move(fd), std::nothrow)) {
        throwf<std::runtime_error>("Socket::open() failed ({})", get_error_string());
    }
}

bool Socket::listen(SockAddr const& address, int type, int protocol, int backlog, std::uint32_t options, std::nothrow_t) noexcept
{
    using namespace std::placeholders;

    close();

    m_events = EventLoop::Read;

    // Create socket.
    UniqueOwner<int, -1> fd(
        ::socket(address.family(), type, protocol),
        file::fd_close
    );
    if (-1 == fd.value()) {
        return false;
    }

    // Make non-blocking.
    if (false == file::fd_set_nonblocking(fd.value(), true)) {
        return false;
    }

    // Set reuse options.
    if (false == set_reuse(fd.value(), options)) {
        return false;
    }

    // Bind socket to address.
    if (-1 == ::bind(fd.value(), as<sockaddr>(address), address.length())) {
        return false;
    }

    // Listen for incoming connections
    if (-1 == ::listen(fd.value(), backlog)) {
        return false;
    }

    // Add file descriptor to event loop.
    bool success = with_weak_ptr_locked(m_event_loop, [&](EventLoop& loop) {
        loop.add(fd.value(), m_events, std::bind(&Socket::onAccept, this, _1, _2));
    });
    if (false == success) {
        errno = ENODEV;
        return false;
    }

    // Commit file descriptor.
    m_fd = std::move(fd);
    return true;
}

void Socket::listen(SockAddr const& address, int type, int protocol, int backlog, std::uint32_t options)
{
    if (false == Socket::listen(address, type, protocol, backlog, options, std::nothrow)) {
        throwf<std::runtime_error>("Socket::listen() failed ({})", get_error_string());
    }
}

bool Socket::connect(SockAddr const& address, int type, int protocol, std::uint32_t options, std::nothrow_t) noexcept
{
    using namespace std::placeholders;

    close();

    EventLoop::Callback callback = std::bind(&Socket::onEvent, this, _1, _2);
    std::uint32_t events = EventLoop::Read;

    m_events = events;

    // Create socket.
    UniqueOwner<int, -1> fd(
        ::socket(address.family(), type, protocol),
        file::fd_close
    );
    if (-1 == fd.value()) {
        return false;
    }

    // Make non-blocking.
    if (false == file::fd_set_nonblocking(fd.value(), true)) {
        return false;
    }

    // Set reuse options.
    if (false == set_reuse(fd.value(), options)) {
        return false;
    }

    // Connect to peer address.
    if (-1 == ::connect(fd.value(), as<sockaddr>(address), address.length())) {
        if (EINPROGRESS != errno) {
            return false;
        }

        callback = std::bind(&Socket::onConnect, this, _1, _2);
        events |= EventLoop::Write;
    }
    else {
        m_connected = true;
    }

    // Add file descriptor to event loop.
    bool success = with_weak_ptr_locked(m_event_loop, [&](EventLoop& loop) {
        loop.add(fd.value(), events, std::move(callback));
    });
    if (false == success) {
        errno = ENODEV;
        return false;
    }

    // Commit file descriptor.
    m_fd = std::move(fd);

    // Callback connected, if connected.
    if (true == m_connected && nullptr != m_on_connected) {
        m_on_connected();
    }
    return true;
}

void Socket::connect(SockAddr const& address, int type, int protocol, std::uint32_t options)
{
    if (false == Socket::connect(address, type, protocol, options, std::nothrow)) {
        throwf<std::runtime_error>("Socket::connect() failed ({})", get_error_string());
    }
}

void Socket::receive(bool enable)
{
    HLIB_LOCK_GUARD(lock, m_mutex);

    std::uint32_t events = m_events;
    if (true == enable) {
        events |= EventLoop::Read;
    }
    else {
        events &= ~EventLoop::Read;
    }

    if (events != m_events) {
        updateEventsLocked(events);
    }
}

void Socket::send(Buffer buffer)
{
    HLIB_LOCK_GUARD(lock, m_mutex);

    std::uint32_t events = m_events;
    if (true == m_send_queue.empty()) {
        events |= EventLoop::Write;
    }

    m_send_queue.emplace_back(std::move(buffer));

    if (events != m_events) {
        updateEventsLocked(events);
    }
}

void Socket::close() noexcept
{
    if (-1 == m_fd.value()) {
        return;
    }

    with_weak_ptr_locked(m_event_loop, [&](EventLoop& loop) {
        loop.remove(m_fd.value());
    });

    m_fd.reset();

    m_receive_buffer.reset();

    m_send_buffer_offset = 0;
    m_send_queue.clear();
}

