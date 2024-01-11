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
#include "hlib/web_socket_server.hpp"
#include "hlib/config.hpp"
#include "hlib/container.hpp"
#include "hlib/lock.hpp"
#include "hlib/event_loop.hpp"
#include "hlib/error.hpp"
#include "hlib/format.hpp"
#include "hlib/log.hpp"
#include "hlib/memory.hpp"
#include "hlib/string.hpp"
#include <algorithm>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

#define HWS_VISIBILITY_STATIC
#define HWS_HAVE_OPENSSL
#define HWS_IMPL
#include "../third_party/hserv/include/hws.h"

#pragma GCC diagnostic pop

using namespace hlib;

namespace
{

template<class... Ts>
struct Overloaded : Ts... { using Ts::operator()...; };

template<class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

} // namespace

//
// Public (Frame)
//
ws::Server::Socket::Frame::Frame(Opcode a_opcode)
    : opcode{ a_opcode }
    , fin{ true }
{
}

ws::Server::Socket::Frame::Frame(Opcode a_opcode, bool a_fin, Message a_message)
    : opcode{ a_opcode }
    , fin{ a_fin }
    , message(std::move(a_message))
{
}

//
// Implementation (Socket)
//
ws::Server::Socket::Socket(Server& a_server, Socket::Id a_id, struct hws_s* a_hws, http::Socket a_http_socket)
    : server(a_server)
    , id{ a_id }
    , m_hws(a_hws)
    , m_socket(nullptr)
    , m_ping_timer(server.m_event_loop, std::bind(&Socket::onPingTimer, this))
    , m_max_receive_message_size{ Config::wsMaxReceiveMessageSize() }
    , m_fragment_message_threshold{ Config::wsFragmentMessageThreshold() }
{
    hws_socket_callbacks_t callbacks = {
        [](hws_t* /* hws */, hws_socket_t* socket) -> int
        {
            return static_cast<Socket*>(hws_socket_get_user_data(socket))->onInterrupt();
        },
        [](hws_t* /* hws */, hws_socket_t* socket, hws_opcode_t opcode, std::size_t size, int fin) -> int
        {
            return static_cast<Socket*>(hws_socket_get_user_data(socket))->onReceiveHeader(static_cast<Opcode>(opcode), size, !!fin);
        },
        [](hws_t* /* hws */, hws_socket_t* socket, void* buffer, std::size_t size) -> int
        {
            return static_cast<Socket*>(hws_socket_get_user_data(socket))->onReceived(buffer, size);
        },
        [](hws_t* /* hws */, hws_socket_t* socket, void const* buffer, std::size_t size) -> int
        {
            return static_cast<Socket*>(hws_socket_get_user_data(socket))->onSent(buffer, size);
        },
        [](hws_t* /* hws */, hws_socket_t* socket, int clean)
        {
            static_cast<Socket*>(hws_socket_get_user_data(socket))->onClosed(!!clean);
        }
    };

    m_socket = hws_socket_create(a_hws, a_http_socket.fd, a_http_socket.ssl, &callbacks);
    if (nullptr == m_socket) {
        throwf<std::runtime_error>("hws_socket_create() failed");
    }
    hws_socket_set_user_data(m_socket, this);

    // Reset HTTP upgrade resources.
    a_http_socket.fd = -1;
    a_http_socket.ssl = nullptr;
}

void ws::Server::Socket::restart_locked()
{
    if (m_send_queue.size() < 1) {
        return;
    }

    hverify(-1 != hws_socket_interrupt(m_socket));
}

void ws::Server::Socket::onPingTimer()
{
    ping();
}

int ws::Server::Socket::onInterrupt()
{
    try {
        HLIB_LOCK_GUARD(lock, m_send_queue_mutex);
        if (true == m_send_queue.empty()) {
            return 0;
        }

        // Get frame from head of send queue and send.
        Frame const& frame = m_send_queue.front();
        return std::visit(Overloaded{
            [&](std::monostate const&) noexcept
            {
                return hws_socket_send(m_hws, m_socket, static_cast<hws_opcode_t>(frame.opcode), nullptr, 0, frame.fin);
            },
            [&](auto const& message) noexcept
            {
                return hws_socket_send(m_hws, m_socket, static_cast<hws_opcode_t>(frame.opcode), message.data(), message.size(), frame.fin);
            }
        }, frame.message.data);
        return 0;
    }
    catch (...) {
        HLOGE(server.m_logger, "{}: Exception in interrupt callback", id);
        return -1;
    }
}

int ws::Server::Socket::onReceiveHeader(Opcode opcode, std::size_t size, bool fin)
{
    try {
        Buffer* buffer = nullptr;

        switch (opcode) {
        case Opcode::Continuation:
            if (true == m_receive_queue.empty()) {
                HLOGE(server.m_logger, "{}: Invalid first Continuation frame", id);
                return -1;
            }

            // Create a frame on the receive queue.
            m_receive_queue.emplace_back(opcode, fin, Buffer(size));

            // Get pointer to buffer.
            buffer = &std::get<Buffer>(m_receive_queue.back().message.data);
            break;

        case Opcode::Text:
        case Opcode::Binary:
            if (false == m_receive_queue.empty()) {
                HLOGE(server.m_logger, "{}: Invalid non-first {} frame", id, to_string(opcode));
                return -1;
            }

            // Create a frame on the receive queue.
            m_receive_queue.emplace_back(opcode, fin, Buffer(size));

            // Get pointer to buffer.
            buffer = &std::get<Buffer>(m_receive_queue.back().message.data);
            break;

        case Opcode::Ping:
        case Opcode::Pong:
            if (0 != size) {
                HLOGE(server.m_logger, "{}: Invalid non-empty {} frame", id, to_string(opcode));
                return -1;
            }

            HLIB_FALLTHROUGH;

        case Opcode::Close:
            // Initialize a control frame.
            m_receive_control = Frame(opcode, fin, Buffer(size));

            // Get pointer to buffer.
            buffer = &std::get<Buffer>(m_receive_control->message.data);
            break;

        default:
            HLOGE(server.m_logger, "{}: Invalid or unsupported WebSocket opcode {}", id, static_cast<int>(opcode));
            return -1;
        }

        size_t queued_size = 0;
        for (Frame& frame : m_receive_queue) {
            queued_size += std::get<Buffer>(frame.message.data).size();
        }

        if (m_max_receive_message_size > 0 && queued_size + size > m_max_receive_message_size) {
            HLOGE(server.m_logger, "{}: Unfragmented message size exceeds maximum", id);
            return -1;
        }

        return size > 0 ? hws_socket_receive(m_hws, m_socket, buffer->reserve(size), size) : 0;
    }
    catch (...) {
        HLOGE(server.m_logger, "{}: Exception in receive header callback", id);
        return -1;
    }
}

int ws::Server::Socket::onReceived(void* buffer, std::size_t size)
{
    try {
        // Received a control frame?
        if (true == m_receive_control.has_value()) {
            // Set control frame buffer's size.
            std::get<Buffer>(m_receive_control->message.data).resize(size);

            // Selectively handle control frame.
            switch (m_receive_control->opcode) {
            case Opcode::Close:
              {
                StatusCode const pre_close_code = m_close_code;

                uint8_t const* ptr = static_cast<uint8_t*>(buffer);
                if (size >= 2) {
                    std::uint16_t status_code = static_cast<uint16_t>(ptr[0]) << 8
                                              | static_cast<uint16_t>(ptr[1]) << 0;
                    // Decode close code.
                    m_close_code = static_cast<StatusCode>(status_code);
                    if (size > 2) {
                        // Copy close reason.
                        m_close_reason.assign(ptr + 2, size - 2);
                    }
                }
                else {
                    // Assume normal closure.
                    m_close_code = StatusCode::Normal;
                    m_close_reason.clear();
                }

                // Respond with close frame, when close code was not already set.
                if (StatusCode::NoStatus == pre_close_code) {
                    // RFC 6455: https://www.rfc-editor.org/rfc/rfc6455.html#page-36.
                    // If an endpoint receives a Close frame and did not previously
                    // send a Close frame, the endpoint MUST send a Close frame
                    // in response. (When sending a Close frame in response, the
                    // endpoint typically echos the status code it received.)
                    //
                    // TODO prune send queue and queue after last fragment of
                    // fragmented frame being sent.
                    //
                    close(m_close_code, m_close_reason.copy());
                }
                break;
              }
            case Opcode::Ping:
              {
                HLIB_LOCK_GUARD(lock, m_send_queue_mutex);
                m_send_queue.emplace_back(Opcode::Pong);

                restart_locked();
                break;
              }

            case Opcode::Pong:
                // Update pong timestamp when a pong timeout is configured.
                if (m_pong_timeout > 0.0) {
                    m_pong_last_timestamp = time::now();
                }

                // Callback?
                if (nullptr != m_on_pong) {
                    m_on_pong();
                }
                break;

            default:
                assert(false);
                break;
            }

            m_receive_control.reset();
            return 0;
        }

        assert(false == m_receive_queue.empty());

        // Set frame buffer's size.
        std::get<Buffer>(m_receive_queue.back().message.data).resize(size);

        // Continue receiving frames until the fin bit is set.
        if (false == m_receive_queue.back().fin) {
            return 0;
        }

        // Only do work if there is a message callback.
        if (nullptr != m_on_message) {
            Frame& front = m_receive_queue.front();

            if (1 == m_receive_queue.size()) {
                assert(true == std::holds_alternative<Buffer>(front.message.data));

                // Convert to text message?
                if (Opcode::Text == front.opcode) {
                    // Convert buffer to a string.
                    front.message.data = to_string(std::get<Buffer>(front.message.data));
                }
            }
            else {
                // Determine total, unfragmented message length.
                std::size_t total = 0;
                for (Frame const& frame : m_receive_queue) {
                    total += std::get<Buffer>(frame.message.data).size();
                }

                // Reserve memory in front frame's buffer.
                Buffer& front_buffer = std::get<Buffer>(front.message.data);
                front_buffer.reserve(total);

                // Append subsequent frame's buffers to front buffer.
                for (std::size_t i = 1; i < m_receive_queue.size(); ++i) {
                    Buffer& fragment = std::get<Buffer>(m_receive_queue[i].message.data);
                    front_buffer.append(fragment.data(), fragment.size());
                }
            }

            // Callback with front frame's (defragmented) message.
            m_on_message(front.message);
        }

        // Clear receive queue for next frame(s).
        m_receive_queue.clear();
        return 0;
    }
    catch (...) {
        HLOGE(server.m_logger, "{}: Exception in received callback", id);
        return -1;
    }
}

int ws::Server::Socket::onSent(void const* /* buffer */, std::size_t /* size */)
{
    try {
        HLIB_LOCK_GUARD(lock, m_send_queue_mutex);

        // Remove head of send queue.
        if (false == m_send_queue.empty()) {
            m_send_queue.pop_front();
        }

        // Continue sending other frames.
        restart_locked();
        return 0;
    }
    catch (...) {
        HLOGE(server.m_logger, "{}: Exception in sent callback", id);
        return -1;
    }
}

void ws::Server::Socket::onClosed(bool clean)
{
    assert(HWS_STATE_CLOSED == hws_socket_get_state(m_socket));

    try {
        if (false == clean) {
            // Set abnormale close code.
            m_close_code = StatusCode::Abnormal;
            m_close_reason.clear();

            // Emit an error callback?
            if (nullptr != m_on_error) {
                m_on_error();
            }
        }

        // Emit close callback?
        if (nullptr != m_on_close) {
            m_on_close(m_close_code, m_close_reason);
        }
    }
    catch (...) {
        HLOGE(server.m_logger, "{}: Exception in closed callback", id);
    }

    // Remove socket.
    server.remove(id);
}

//
// Public (Socket)
//
ws::State ws::Server::Socket::state() const noexcept
{
    return static_cast<State>(hws_socket_get_state(m_socket));
}

ws::StatusCode ws::Server::Socket::closeCode() const noexcept
{
    return m_close_code;
}

std::string ws::Server::Socket::closeReason() const
{
    return to_string(m_close_reason);
}

SockAddr ws::Server::Socket::getPeerAddress() const
{
    sockaddr_storage storage{};
    socklen_t length = sizeof(storage);

    hws_socket_get_peer(m_socket, reinterpret_cast<sockaddr*>(&storage), &length);
    return SockAddr(storage);
}

void ws::Server::Socket::setPongCallback(OnPong callback)
{
    assert(true == callback_from(server.m_event_loop));

    m_on_pong = std::move(callback);
}

void ws::Server::Socket::setMessageCallback(OnMessage callback)
{
    assert(true == callback_from(server.m_event_loop));

    m_on_message = std::move(callback);
}

void ws::Server::Socket::setErrorCallback(OnError callback)
{
    assert(true == callback_from(server.m_event_loop));

    m_on_error = std::move(callback);
}

void ws::Server::Socket::setCloseCallback(OnClose callback)
{
    assert(true == callback_from(server.m_event_loop));

    m_on_close = std::move(callback);
}

void ws::Server::Socket::setNoDelay(bool enable)
{
    hverify(-1 != hws_socket_set_nodelay(m_socket, enable));
}

void ws::Server::Socket::setPingInterval(time::Duration interval)
{
    if (time::Duration(0) == interval) {
        m_ping_timer.clear();
    }
    else {
        m_ping_timer.set(interval, interval);
    }

    m_pong_last_timestamp = time::now();
}

void ws::Server::Socket::setPongTimeout(time::Duration timeout)
{
    m_pong_timeout = timeout;
    m_pong_last_timestamp = time::now();
}

void ws::Server::Socket::setMaxReceiveMessageSize(std::size_t size)
{
    m_max_receive_message_size = size;
}

void ws::Server::Socket::setFragmentMessageThreshold(std::size_t size)
{
    m_fragment_message_threshold = size;
}

void ws::Server::Socket::ping()
{
    HLIB_LOCK_GUARD(lock, m_send_queue_mutex);

    // Place a Ping control frame at the end of the queue.
    m_send_queue.emplace_back(Opcode::Ping);

    // Restart send process.
    restart_locked();
}

void ws::Server::Socket::receive(bool enable)
{
    if (true == enable) {
        hws_socket_receive_enable(m_socket);
    }
    else {
        hws_socket_receive_disable(m_socket);
    }
}

void ws::Server::Socket::send(Message message)
{
    assert(false == std::holds_alternative<std::monostate>(message.data));

    HLIB_LOCK_GUARD(lock, m_send_queue_mutex);

    Opcode opcode = std::holds_alternative<std::string>(message.data) ? Opcode::Text : Opcode::Binary;
    std::size_t size = std::visit(Overloaded{
        [](std::monostate&) noexcept { return std::size_t(0); },
        [](auto& msg) noexcept { return msg.size(); }
    }, message.data);

    if (size < m_fragment_message_threshold) {
        // Forward message unfragmented.
        m_send_queue.emplace_back(opcode, true, std::move(message));
    }
    else {
        size_t offset = 0;

        // Fragment message using continuation frames.
        do {
            size_t const fragment_size = std::min(size - offset, m_fragment_message_threshold);

            m_send_queue.emplace_back(
                opcode,
                offset + fragment_size < size ? false : true,
                std::visit(Overloaded{
                    [&](std::monostate&) noexcept
                    {
                        return Message();
                    },
                    [&](std::string& text)
                    {
                        return Message(text.substr(offset, fragment_size));
                    },
                    [&](Buffer& binary)
                    {
                        return Message(Buffer(binary.get(offset), fragment_size));
                    }
                }, message.data)
            );

            opcode = Opcode::Continuation;
            offset += fragment_size;
        }
        while (offset < size);
    }

    // Restart send process.
    restart_locked();
}

void ws::Server::Socket::close(StatusCode code, Buffer reason)
{
    assert(StatusCode::NoStatus != code);
    assert(StatusCode::Abnormal != code);
    assert(StatusCode::TLSHandshakeFailed != code);

    HLIB_LOCK_GUARD(lock, m_send_queue_mutex);

    // Reserve space for code bytes, obtaining a pointer to the buffer.
    std::uint8_t* ptr = static_cast<std::uint8_t*>(reason.reserve(reason.size() + 2));
    // Insert space for code bytes.
    reason.insert(0, nullptr, 2);
    // Encode code.
    ptr[0] = static_cast<std::uint16_t>(code) >> 8;
    ptr[1] = static_cast<std::uint16_t>(code) >> 0;

    // Send close frame.
    m_send_queue.emplace_back(Opcode::Close, false, std::move(reason));

    // Restart send process.
    restart_locked();
}

//
// Implementation (Server)
//
void ws::Server::onPoll(int fd, std::uint32_t events)
{
    assert(hws_get_fd(m_hws) == fd); (void)fd;
    assert(EventLoop::Read == events); (void)events;

    if (hws_poll(m_hws) < 0) {
        throw make_system_error(errno, "hws_poll() failed");
    }
}

void ws::Server::onSocketsTimer()
{
    HLIB_LOCK_GUARD(lock, m_sockets_mutex);

    time::Clock const now = time::now();

    // Check for pong timeouts.
    for (auto it = m_sockets.begin(); m_sockets.end() != it;) {
        Socket& socket = *it->second;

        // Is a timeout configured?
        if (socket.m_pong_timeout <= 0) {
            ++it;
            continue;
        }

        // Timeout expired?
        if (now - socket.m_pong_last_timestamp > socket.m_pong_timeout) {
            // Call close callback?
            if (nullptr != socket.m_on_close) {
                // Callback.
                socket.m_on_close(StatusCode::Abnormal, Buffer());
            }

            // Remove socket.
            HLOGE(m_logger, "{}: Socket pong timedout", it->second->id);
            it = m_sockets.erase(it);
        }
        else {
            ++it;
        }
    }

    // Remove closed sockets.
    for (auto it = m_sockets.begin(); m_sockets.end() != it;) {
        if (State::Closed == it->second->state()) {
            HLOGD(m_logger, "{}: Removing closed socket", it->second->id);
            it = m_sockets.erase(it);
        }
        else {
            ++it;
        }
    }
}

//
// Public (Server)
//
ws::Server::Server(log::Domain logger, std::weak_ptr<EventLoop> event_loop)
    : m_logger(std::move(logger))
    , m_event_loop(event_loop)
    , m_sockets_timer(std::move(event_loop), std::bind(&Server::onSocketsTimer, this))
{
}

ws::Server::~Server()
{
    stop();

    hws_destroy(m_hws);
}

std::shared_ptr<EventLoop> ws::Server::getEventLoop() const
{
    return m_event_loop.lock();
}

std::optional<std::reference_wrapper<ws::Server::Socket>> ws::Server::getSocket(Socket::Id id) const
{
    HLIB_LOCK_GUARD(lock, m_sockets_mutex);

    auto it = m_sockets.find(id);
    if (m_sockets.end() == it) {
        return std::nullopt;
    }

    return *it->second;
}

void ws::Server::start()
{
    using namespace std::placeholders;

    hws_config_t config{};
    config.user_data = this;

    // Create hws server.
    m_hws = hws_create(&config);
    if (nullptr == m_hws) {
        throwf<std::runtime_error>("hws_create failed ({})", config.error_string);
    }

    // Add hws's poll fd to event loop.
    hverify(true == with_weak_ptr_locked(m_event_loop, [this](EventLoop& event_loop) {
        event_loop.add(
            hws_get_fd(m_hws),
            EventLoop::Read,
            std::bind(&Server::onPoll, this, _1, _2)
        );
    }));

    // Set socket maintenance timer.
    m_sockets_timer.set(
        Config::wsServerMaintenanceInterval(),
        Config::wsServerMaintenanceInterval()
    );
}

void ws::Server::stop()
{
    m_sockets_timer.clear();

    if (nullptr == m_hws) {
        return;
    }

    hverify(true == with_weak_ptr_locked(m_event_loop, [this](EventLoop& event_loop) {
        event_loop.remove(hws_get_fd(m_hws));
    }));
}

ws::Server::Socket& ws::Server::add(http::Socket http_socket)
{
    assert(true == iequals("websocket", http_socket.protocol));

    Socket::Id const socket_id = ++m_socket_id;
    std::unique_ptr<Socket> socket(new Socket(*this, socket_id, m_hws, std::move(http_socket)));

    HLIB_LOCK_GUARD(lock, m_sockets_mutex);
    m_sockets.emplace(socket_id, std::move(socket));

    return *m_sockets[socket_id];
}

void ws::Server::remove(Socket::Id socket_id)
{
    HLIB_LOCK_GUARD(lock, m_sockets_mutex);
    m_sockets.erase(socket_id);
}

//
// Public (Message)
//
ws::Message::Message(std::string string) noexcept
    : data(std::move(string))
{
}

ws::Message::Message(Buffer buffer) noexcept
    : data(std::move(buffer))
{
}

ws::Message::Message(Data a_data) noexcept
    : data(std::move(a_data))
{
}

ws::Message::Message(User a_user) noexcept
    : user(std::move(a_user))
{
}

//
// Public (Utility)
//
std::optional<std::vector<std::string>> ws::is_upgrade(http::Server::Transaction const& transaction)
{
    std::optional<std::string> value;

    value = http::is_upgrade(transaction);
    if (false == value.has_value() || false == iequals("websocket", value.value())) {
        return std::nullopt;
    }

    if (0 != transaction.request_content_length) {
        return std::nullopt;
    }

    value = transaction.getRequestValue("Sec-WebSocket-Version");
    if (false == value.has_value() || "13" != value.value()) {
        return std::nullopt;
    }

    value = transaction.getRequestValue("Sec-WebSocket-Key");
    if (false == value.has_value()) {
        return std::nullopt;
    }

    value = transaction.getRequestValue("Sec-WebSocket-Protocol");
    if (false == value.has_value()) {
        // Return an empty vector when the field was not found.
        return std::vector<std::string>();
    }

    // Split subprotocols in vector.
    std::vector<std::string> subprotocols = split(value.value(), ',', true);

    // Strip whitespace.
    std::for_each(subprotocols.begin(), subprotocols.end(),
        [](std::string& subprotocol) { subprotocol = strip(subprotocol); });

    return subprotocols;
}

void ws::upgrade(http::Server::Transaction& transaction, std::string subprotocol)
{
    assert(true == ws::is_upgrade(transaction).has_value());

    // Get the Sec-WebSocket-Key from the request. */
    std::optional<std::string> key = transaction.getRequestValue("Sec-WebSocket-Key");
    assert(true == key.has_value());

    auto free_string = [](char* string)
    {
        free(string);
    };

    // Create a Sec-WebSocket-Accept value from the key.
    std::unique_ptr<char, decltype(free_string)> accept(
        hws_generate_sec_websocket_accept(key.value().data(), key.value().size()),
        free_string
    );

    // Response header fields.
    std::vector<http::HeaderField> fields =
    {
        { "Connection",             "upgrade" },
        { "Upgrade",                "websocket" },
        { "Sec-WebSocket-Accept",   accept.get() }
    };
    // Conditionally add Sec-WebSocket-Protocol header.
    if (false == subprotocol.empty()) {
        fields.emplace_back("Sec-WebSocket-Protocol", std::move(subprotocol));
    }

    // Respond.
    transaction.respond(http::StatusCode::SwitchingProtocols, fields);
}

std::string const& hlib::to_string(ws::State state)
{
    static std::unordered_map<ws::State, std::string> const table =
    {
        { ws::State::Connecting,    "Connecting" },
        { ws::State::Open,          "Open" },
        { ws::State::Closing,       "Closing" },
        { ws::State::Closed,        "Closed" }
    };
    static std::string const invalid("Invalid");

    auto it = table.find(state);
    return table.end() != it ? it->second : invalid;
}

std::string const& hlib::to_string(ws::Opcode opcode)
{
    static std::unordered_map<ws::Opcode, std::string> const table =
    {
        { ws::Opcode::Continuation, "Continuation" },
        { ws::Opcode::Text,         "Text" },
        { ws::Opcode::Binary,       "Binary" },
        { ws::Opcode::Close,        "Close" },
        { ws::Opcode::Ping,         "Ping" },
        { ws::Opcode::Pong,         "Pong" }
    };
    static std::string const invalid("Invalid");

    auto it = table.find(opcode);
    return table.end() != it ? it->second : invalid;
}

std::string const& hlib::to_string(ws::StatusCode status_code)
{
    static std::unordered_map<ws::StatusCode, std::string> const table =
    {
        { ws::StatusCode::Normal,               "Normal" },
        { ws::StatusCode::GoingAway,            "Going Away" },
        { ws::StatusCode::ProtocolError,        "Protocol Error" },
        { ws::StatusCode::Unsupported,          "Unsupported" },
        { ws::StatusCode::Reserved,             "Reserved" },
        { ws::StatusCode::NoStatus,             "No Status" },
        { ws::StatusCode::Abnormal,             "Abnormal Termination" },
        { ws::StatusCode::UnsupportedPayload,   "Unsupported Payload" },
        { ws::StatusCode::PolicyViolation,      "Policy Violation" },
        { ws::StatusCode::TooLarge,             "Message Too Large" },
        { ws::StatusCode::MandatoryExtension,   "Mandatory Extension" },
        { ws::StatusCode::ServerError,          "Server Error" },
        { ws::StatusCode::ServiceRestart,       "Service Restart" },
        { ws::StatusCode::TryAgain,             "Try Again" },
        { ws::StatusCode::BadGateway,           "Bad Gateway" },
        { ws::StatusCode::TLSHandshakeFailed,   "TLS Handshake Failed" }
    };
    static std::string const invalid("Invalid");

    auto it = table.find(status_code);
    return table.end() != it ? it->second : invalid;
}
