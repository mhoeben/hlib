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

#include "hlib/http_server.hpp"
#include "hlib/time.hpp"
#include "hlib/timer.hpp"
#include <list>
#include <mutex>
#include <variant>

extern "C"
{

struct hws_s;
struct hws_socket_s;

} // extern "C"

namespace hlib
{
namespace ws
{

enum class State
{
    Connecting,
    Open,
    Closing,
    Closed
};

enum class Opcode
{
    Continuation = 0,
    Text = 1,
    Binary = 2,
    Close = 8,
    Ping = 9,
    Pong = 10
};

// https://datatracker.ietf.org/doc/html/rfc6455#section-7.4
enum class StatusCode : uint16_t
{
    Normal = 1000,
    GoingAway = 1001,
    ProtocolError = 1002,
    Unsupported = 1003,
    Reserved = 1004,
    NoStatus = 1005,            // MUST NOT be send.
    Abnormal = 1006,            // MUST NOT be send.
    UnsupportedPayload = 1007,
    PolicyViolation = 1008,
    TooLarge = 1009,
    MandatoryExtension = 1010,
    ServerError = 1011,
    ServiceRestart = 1012,
    TryAgain = 1013,
    BadGateway = 1014,
    TLSHandshakeFailed = 1015     // MUST NOT be send.
};

struct Message
{
    typedef std::variant<std::monostate, std::string, Buffer> Data;
    typedef std::shared_ptr<void> User;

    Data data;
    User user;

    Message() = default;

    Message(std::string string) noexcept;
    Message(Buffer buffer) noexcept;
    Message(Data data) noexcept;
    Message(User user) noexcept;
};


class Server final
{
    HLIB_NOT_COPYABLE(Server);
    HLIB_NOT_MOVABLE(Server);

public:
    class Socket final
    {
        friend Server;

        HLIB_NOT_COPYABLE(Socket);
        HLIB_NOT_MOVABLE(Socket);

    public:
        typedef std::uint64_t Id;

        typedef std::function<void()> OnPong;
        typedef std::function<void(Message& message)> OnMessage;
        typedef std::function<void()> OnError;
        typedef std::function<void(StatusCode code, Buffer const& reason)> OnClose;

    public:
        Server& server;
        Id const id;

        std::shared_ptr<void> user;

        State state() const noexcept;
        StatusCode closeCode() const noexcept;
        std::string closeReason() const;

        SockAddr getPeerAddress() const;

        void setPongCallback(OnPong callback);
        void setMessageCallback(OnMessage callback);
        void setErrorCallback(OnError callback);
        void setCloseCallback(OnClose callback);

        void setNoDelay(bool enable);
        void setPingInterval(time::Duration interval);
        void setPongTimeout(time::Duration timeout);
        void setMaxReceiveMessageSize(std::size_t size);
        void setFragmentMessageThreshold(std::size_t size);

        void ping();
        void receive(bool enable);
        void send(Message message);
        void close(StatusCode code = StatusCode::Normal, Buffer reason = Buffer());

    private:
        struct hws_s* m_hws;
        struct hws_socket_s* m_socket;

        OnPong m_on_pong;
        OnMessage m_on_message;
        OnError m_on_error;
        OnClose m_on_close;

        Timer m_ping_timer;
        time::Duration m_pong_timeout;
        time::Clock m_pong_last_timestamp;
        std::size_t m_max_receive_message_size;
        std::size_t m_fragment_message_threshold;

        struct Frame
        {
            Opcode opcode;
            bool fin;
            Message message;

            Frame(Opcode a_opcode);
            Frame(Opcode a_opcode, bool a_fin, Message a_mesage);
        };

        std::optional<Frame> m_receive_control;
        std::vector<Frame> m_receive_queue;

        std::mutex m_send_queue_mutex;
        std::list<Frame> m_send_queue;

        StatusCode m_close_code{ StatusCode::NoStatus };
        Buffer m_close_reason;

        Socket(Server& server, Socket::Id a_id, struct hws_s* a_hws, http::Socket a_http_socket);

        void restart_locked();

        void onPingTimer();
        int onInterrupt();
        int onReceiveHeader(Opcode opcode, std::size_t size, bool fin);
        int onReceived(void* buffer, std::size_t size);
        int onSent(void const* buffer, std::size_t size);
        void onClosed(bool error);
    };

public:
    Server(log::Domain logger, std::weak_ptr<EventLoop> event_loop);
    ~Server();

    std::shared_ptr<EventLoop> getEventLoop() const;
    std::optional<std::reference_wrapper<Socket>> getSocket(Socket::Id id) const;

    void start();
    void stop();

    Socket& add(http::Socket socket);
    void remove(Socket::Id socket_id);

private:
    log::Domain const m_logger;
    std::weak_ptr<EventLoop> const m_event_loop;
    struct hws_s* m_hws{ nullptr };

    Socket::Id m_socket_id{ 0 };
    Timer m_sockets_timer;
    mutable std::mutex m_sockets_mutex;
    std::unordered_map<Socket::Id, std::unique_ptr<Socket>> m_sockets;

    void onPoll(int fd, std::uint32_t events);
    void onSocketsTimer();
};

std::optional<std::vector<std::string>> is_upgrade(http::Server::Transaction const& transaction);
void upgrade(http::Server::Transaction& transaction, std::string subprotocol = "");

} // namespace ws

std::string const& to_string(ws::State state);
std::string const& to_string(ws::Opcode opcode);
std::string const& to_string(ws::StatusCode status_code);

} // namespace hlib

