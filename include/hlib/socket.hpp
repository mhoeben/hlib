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

#include "hlib/buffer.hpp"
#include "hlib/event_loop.hpp"
#include "hlib/memory.hpp"
#include "hlib/sock_addr.hpp"
#include <list>
#include <new>

namespace hlib
{

class Socket final
{
    HLIB_NOT_COPYABLE(Socket);
    HLIB_NOT_MOVABLE(Socket);

public:
    static constexpr std::uint32_t ReuseAddr{ 0x01 };
    static constexpr std::uint32_t ReusePort{ 0x02 };

    typedef std::function<void(UniqueOwner<int, -1> fd, SockAddr const& address)> OnAccept;
    typedef std::function<void()> OnConnected;
    typedef std::function<void(Buffer& buffer)> OnReceive;
    typedef std::function<void(int error)> OnClose;

public:
    std::shared_ptr<void> user;

    Socket(std::weak_ptr<EventLoop> event_loop);
    Socket(std::weak_ptr<EventLoop> event_loop, UniqueOwner<int, -1> fd);
    ~Socket();

    int fd() const;

    SockAddr getPeerAddress() const;

    void setAcceptCallback(OnAccept callback);
    void setConnectedCallback(OnConnected callback);
    void setReceiveCallback(OnReceive callback);
    void setCloseCallback(OnClose callback);
    void setReceiveBufferSize(std::size_t size, bool gather = false);

    bool open(UniqueOwner<int, -1> fd, std::nothrow_t) noexcept;
    void open(UniqueOwner<int, -1> fd);

    bool listen(SockAddr const& address, int type, int protocol, int backlog, std::uint32_t options, std::nothrow_t) noexcept;
    void listen(SockAddr const& address, int type, int protocol, int backlog, std::uint32_t options);

    bool connect(SockAddr const& address, int type, int protocol, std::uint32_t options, std::nothrow_t) noexcept;
    void connect(SockAddr const& address, int type, int protocol, std::uint32_t options);

    void receive(bool enable);
    void send(Buffer buffer);
    void close() noexcept;

private:
    std::weak_ptr<EventLoop> m_event_loop;
    UniqueOwner<int, -1> m_fd;

    OnAccept m_on_accept;
    OnConnected m_on_connected;
    OnReceive m_on_receive;
    OnClose m_on_close;

    std::mutex m_mutex;

    bool m_connected{ false };
    std::uint32_t m_events{ 0 };

    std::size_t m_receive_buffer_size;
    bool m_receive_buffer_gather;
    Buffer m_receive_buffer;

    std::size_t m_send_buffer_offset{ 0 };
    std::list<Buffer> m_send_queue;

    void updateEventsLocked(std::uint32_t events) noexcept;

    void onAccept(int fd, std::uint32_t events);
    void onConnect(int fd, std::uint32_t events);
    void onEvent(int fd, std::uint32_t events);

    void callbackAndClose(int error);
};

} // namespace hlib
