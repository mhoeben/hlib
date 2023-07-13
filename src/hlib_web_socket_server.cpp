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
using namespace hlib::ws;

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
Server::Socket::Frame::Frame(Opcode a_opcode)
    : opcode{ a_opcode }
    , fin{ false }
{
}

Server::Socket::Frame::Frame(Opcode a_opcode, bool a_fin, Message a_message)
    : opcode{ a_opcode }
    , fin{ a_fin }
    , message(std::move(a_message))
{
}

//
// Implementation (Socket)
//
Server::Socket::Socket(Server& a_server, Socket::Id a_id, struct hws_s* a_hws, http::Upgrade a_upgrade)
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

    m_socket = hws_socket_create(a_hws, a_upgrade.fd, a_upgrade.ssl, &callbacks);
    if (nullptr == m_socket) {
        throwf<std::runtime_error>("Failed to create websocket");
    }
    hws_socket_set_user_data(m_socket, this);

    // Reset HTTP upgrade resources.
    a_upgrade.fd = -1;
    a_upgrade.ssl = nullptr;
}

void Server::Socket::restart_locked()
{
    if (m_send_queue.size() < 1) {
        return;
    }

    hverify(-1 != hws_socket_interrupt(m_socket));
}

void Server::Socket::onPingTimer()
{
    ping();
}

int Server::Socket::onInterrupt()
{
    try {
        std::lock_guard<std::mutex> lock(m_send_queue_mutex);
        if (true == m_send_queue.empty()) {
            return 0;
        }

        // Get frame from head of send queue and send.
        Frame const& frame = m_send_queue.front();
        return std::visit(Overloaded{
            [&](std::monostate const&) noexcept
            {
                return hws_socket_send(m_hws, m_socket, static_cast<hws_opcode_t>(frame.opcode), nullptr, 0, false);
            },
            [&](auto const& message) noexcept
            {
                return hws_socket_send(m_hws, m_socket, static_cast<hws_opcode_t>(frame.opcode), message.data(), message.size(), frame.fin);
            }
        }, frame.message);
        return 0;
    }
    catch (...) {
        HLOGE(server.m_logger, "Exception in interrupt callback");
        return -1;
    }
}

int Server::Socket::onReceiveHeader(Opcode opcode, std::size_t size, bool fin)
{
    try {
        Buffer* buffer = nullptr;

        switch (opcode) {
        case Opcode::Continuation:
            if (true == m_receive_queue.empty()) {
                HLOGE(server.m_logger, "Invalid first Continuation frame");
                return -1;
            }

            // Create a frame on the receive queue.
            m_receive_queue.emplace_back(opcode, fin, Buffer(size));

            // Get pointer to buffer.
            buffer = &std::get<Buffer>(m_receive_queue.back().message);
            break;

        case Opcode::Text:
        case Opcode::Binary:
            if (false == m_receive_queue.empty()) {
                HLOGE(server.m_logger, "Invalid non-first {} frame", to_string(opcode));
                return -1;
            }

            // Create a frame on the receive queue.
            m_receive_queue.emplace_back(opcode, fin, Buffer(size));

            // Get pointer to buffer.
            buffer = &std::get<Buffer>(m_receive_queue.back().message);
            break;

        case Opcode::Ping:
        case Opcode::Pong:
            if (0 != size) {
                HLOGE(server.m_logger, "Invalid non-empty {} frame", to_string(opcode));
                return -1;
            }

            HLIB_FALLTHROUGH;

        case Opcode::Close:
            // Initialize a control frame.
            m_receive_control = Frame(opcode, fin, Buffer(size));

            // Get pointer to buffer.
            buffer = &std::get<Buffer>(m_receive_control->message);
            break;

        default:
            HLOGE(server.m_logger, "Invalid or unsupported WebSocket opcode {}", static_cast<int>(opcode));
            return -1;
        }

        return hws_socket_receive(m_hws, m_socket, buffer->reserve(size), size);
    }
    catch (...) {
        HLOGE(server.m_logger, "Exception in receive header callback");
        return -1;
    }
}

int Server::Socket::onReceived(void* buffer, std::size_t size)
{
    try {
        // Received a control frame?
        if (true == m_receive_control.has_value()) {
            // Set control frame buffer's size.
            std::get<Buffer>(m_receive_control->message).resize(size);

            // Selectively handle control frame.
            switch (m_receive_control->opcode) {
            case Opcode::Close:
              {
                uint8_t const* ptr = static_cast<uint8_t*>(buffer);
                if (size >= 2) {
                    // Decode close code.
                    m_close_code = static_cast<uint16_t>(ptr[0]) << 8
                                 | static_cast<uint16_t>(ptr[1]) << 0;
                    if (size > 2) {
                        // Copy close reason.
                        m_close_reason.assign(ptr + 2, size - 2);
                    }
                }
                break;
              }
            case Opcode::Ping:
              {
                std::lock_guard<std::mutex> lock(m_send_queue_mutex);
                m_send_queue.emplace_back(Opcode::Pong);

                restart_locked();
                break;
              }

            case Opcode::Pong:
                if (nullptr != m_on_pong) {
                    m_on_pong(*this);
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
        std::get<Buffer>(m_receive_queue.back().message).resize(size);

        // Continue receiving frames until the fin bit is set.
        if (false == m_receive_queue.back().fin) {
            return 0;
        }

        // Only do work if there is a message callback.
        if (nullptr != m_on_message) {
            Frame& front = m_receive_queue.front();

            if (1 == m_receive_queue.size()) {
                assert(true == std::holds_alternative<Buffer>(front.message));

                // Convert to text message?
                if (Opcode::Text == front.opcode) {
                    // Convert buffer to a string.
                    front.message = to_string(std::get<Buffer>(front.message));
                }
            }
            else {
                // Determine total, unfragmented message length.
                std::size_t total = 0;
                for (Frame const& frame : m_receive_queue) {
                    total += std::get<Buffer>(frame.message).size();
                }

                // Reserve memory in front frame's buffer.
                Buffer& front_buffer = std::get<Buffer>(front.message);
                front_buffer.reserve(total);

                // Append subsequent frame's buffers to front buffer.
                for (std::size_t i = 1; i < m_receive_queue.size(); ++i) {
                    Buffer& fragment = std::get<Buffer>(m_receive_queue[i].message);
                    front_buffer.append(fragment.data(), fragment.size());
                }
            }

            // Callback with front frame's (defragmented) message.
            m_on_message(*this, front.message);
        }

        // Clear receive queue for next frame(s).
        m_receive_queue.clear();
        return 0;
    }
    catch (...) {
        HLOGE(server.m_logger, "Exception in received callback");
        return -1;
    }
}

int Server::Socket::onSent(void const* /* buffer */, std::size_t /* size */)
{
    try {
        std::lock_guard<std::mutex> lock(m_send_queue_mutex);

        // Remove head of send queue.
        if (false == m_send_queue.empty()) {
            m_send_queue.pop_front();
        }

        // Continue sending other frames.
        restart_locked();
        return 0;
    }
    catch (...) {
        HLOGE(server.m_logger, "Exception in sent callback");
        return -1;
    }
}

void Server::Socket::onClosed(bool clean)
{
    assert(HWS_STATE_CLOSED == hws_socket_get_state(m_socket));

    try {
        if (false == clean) {
            // Set abnormale close code.
            m_close_code = 1006;
            m_close_reason.clear();

            // Emit an error callback?
            if (nullptr != m_on_error) {
                m_on_error(*this);
            }
        }

        // Emit close callback?
        if (nullptr != m_on_close) {
            m_on_close(*this, clean, m_close_code, m_close_reason);
        }
    }
    catch (...) {
        HLOGE(server.m_logger, "Exception in closed callback");
    }
}

//
// Public (Socket)
//
State Server::Socket::state() const
{
    return static_cast<State>(hws_socket_get_state(m_socket));
}

SockAddr Server::Socket::getPeerAddress() const
{
    sockaddr_storage storage{};
    socklen_t length = sizeof(storage);

    hws_socket_get_peer(m_socket, reinterpret_cast<sockaddr*>(&storage), &length);
    return SockAddr(storage);
}

void Server::Socket::setPongCallback(PongCallback callback)
{
    assert(true == callback_from(server.m_event_loop));

    m_on_pong = std::move(callback);
}

void Server::Socket::setMessageCallback(MessageCallback callback)
{
    assert(true == callback_from(server.m_event_loop));

    m_on_message = std::move(callback);
}

void Server::Socket::setErrorCallback(ErrorCallback callback)
{
    assert(true == callback_from(server.m_event_loop));

    m_on_error = std::move(callback);
}

void Server::Socket::setCloseCallback(CloseCallback callback)
{
    assert(true == callback_from(server.m_event_loop));

    m_on_close = std::move(callback);
}

void Server::Socket::setNoDelay(bool enable)
{
    hverify(-1 != hws_socket_set_nodelay(m_socket, enable));
}

void Server::Socket::setPingInterval(Duration interval)
{
    m_ping_timer.set(interval, interval);
}

void Server::Socket::setMaxReceiveMessageSize(std::size_t size)
{
    m_max_receive_message_size = size;
}

void Server::Socket::setFragmentMessageThreshold(std::size_t size)
{
    m_fragment_message_threshold = size;
}

void Server::Socket::ping()
{
    std::lock_guard<std::mutex> lock(m_send_queue_mutex);

    // Place a Ping control frame at the end of the queue.
    m_send_queue.emplace_back(Opcode::Ping);

    // Restart send process.
    restart_locked();
}

void Server::Socket::send(Message message)
{
    assert(false == std::holds_alternative<std::monostate>(message));

    std::lock_guard<std::mutex> lock(m_send_queue_mutex);

    std::size_t size = std::visit(Overloaded{
        [](std::monostate&) noexcept { return std::size_t(0); },
        [](auto& msg) noexcept { return msg.size(); }
    }, message);

    if (size < m_fragment_message_threshold) {
        // Forward message unfragmented.
        m_send_queue.emplace_back(Opcode::Text, true, std::move(message));
    }
    else {
        Opcode opcode = std::holds_alternative<std::string>(message) ? Opcode::Text : Opcode::Binary;
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
                        return Message(Buffer(binary[offset], fragment_size));
                    }
                }, message)
            );

            opcode = Opcode::Continuation;
            offset += fragment_size;
        }
        while (offset < size);
    }

    // Restart send process.
    restart_locked();
}

void Server::Socket::close(uint16_t code, Buffer reason)
{
    std::lock_guard<std::mutex> lock(m_send_queue_mutex);

    // If code is not the reserved 'no present" value of 1005, insert
    // code before the reason. Else, make sure reason is empty.
    if (1005 != code) {
        // Reserve space for code bytes, obtaining a pointer to the buffer.
        uint8_t* ptr = static_cast<uint8_t*>(reason.reserve(reason.size() + 2));
        // Insert space for code bytes.
        reason.insert(0, nullptr, 2);
        // Encode code.
        ptr[0] = static_cast<uint8_t>(code >> 8);
        ptr[1] = static_cast<uint8_t>(code >> 0);
    }
    else {
        assert(true == reason.empty());
        reason.clear();
    }

    // Send close frame.
    m_send_queue.emplace_back(Opcode::Close, false, std::move(reason));

    // Restart send process.
    restart_locked();
}

//
// Implementation (Server)
//
void Server::onPoll(int fd, std::uint32_t events)
{
    assert(hws_get_fd(m_hws) == fd); (void)fd;
    assert(EventLoop::Read == events); (void)events;

    if (hws_poll(m_hws) < 0) {
        throwf<std::runtime_error>("WebSocket server poll failed ({})", get_error_string(errno));
    }
}

void Server::onSocketsTimer()
{
    std::lock_guard<std::mutex> lock(m_sockets_mutex);

    // Remove closed sockets.
    for (auto it = m_sockets.begin(); m_sockets.end() != it;) {
        if (State::Closed == it->second->state()) {
            HLOGD(m_logger, "{}: removing closed socket", it->second->id);
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
Server::Server(log::Domain logger, std::weak_ptr<EventLoop> event_loop)
    : m_logger(std::move(logger))
    , m_event_loop(event_loop)
    , m_sockets_timer(std::move(event_loop), std::bind(&Server::onSocketsTimer, this))
{
}

Server::~Server()
{
    stop();

    hws_destroy(m_hws);
}

std::shared_ptr<EventLoop> Server::getEventLoop() const
{
    return m_event_loop.lock();
}

std::optional<std::reference_wrapper<Server::Socket>> Server::getSocket(Socket::Id id) const
{
    std::lock_guard<std::mutex> lock(m_sockets_mutex);
    auto it = m_sockets.find(id);
    return m_sockets.end() == it
        ? *it->second
        : std::optional<std::reference_wrapper<Socket>>();
}

void Server::start()
{
    using namespace std::placeholders;

    // Create hws server.
    m_hws = hws_create(this);

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

void Server::stop()
{
    m_sockets_timer.clear();

    if (nullptr == m_hws) {
        return;
    }

    hverify(true == with_weak_ptr_locked(m_event_loop, [this](EventLoop& event_loop) {
        event_loop.remove(hws_get_fd(m_hws));
    }));
}

Server::Socket& Server::add(http::Upgrade upgrade)
{
    assert(true == iequals("websocket", upgrade.subprotocol));

    Socket::Id const socket_id = ++m_socket_id;
    std::unique_ptr<Socket> socket(new Socket(*this, socket_id, m_hws, std::move(upgrade)));

    std::lock_guard<std::mutex> lock(m_sockets_mutex);
    m_sockets.emplace(socket_id, std::move(socket));

    return *m_sockets[socket_id];
}

void Server::remove(Socket::Id socket_id)
{
    std::lock_guard<std::mutex> lock(m_sockets_mutex);
    m_sockets.erase(socket_id);
}

//
// Utility
//
std::optional<std::vector<std::string>> ws::is_upgrade(http::Server::Transaction const& transaction)
{
    std::optional<std::string> value;

    value = http::is_upgrade(transaction);
    if (false == value.has_value() || false == iequals("websocket", value.value())) {
        return std::optional<std::vector<std::string>>();
    }

    value = transaction.getRequestValue("Sec-WebSocket-Version");
    if (false == value.has_value() || "13" != value.value()) {
        return std::optional<std::vector<std::string>>();
    }

    value = transaction.getRequestValue("Sec-WebSocket-Key");
    if (false == value.has_value()) {
        return std::optional<std::vector<std::string>>();
    }

    value = transaction.getRequestValue("Sec-WebSocket-Protocol");
    if (false == value.has_value()) {
        return std::optional<std::vector<std::string>>();
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

    transaction.respond(
        http::StatusCode::SwitchingProtocols,
        std::vector<http::HeaderField>{
            { "Connection",             "upgrade" },
            { "Upgrade",                "websocket" },
            { "Sec-WebSocket-Accept",   accept.get() },
            { "Sec-WebSocket-Protocol", std::move(subprotocol) }
        }
    );
}

//
// Utility
//
std::string const& hlib::ws::to_string(State state)
{
    static std::unordered_map<State, std::string> const table =
    {
        { State::Connecting,    "Connecting" },
        { State::Open,          "Open" },
        { State::Closing,       "Closing" },
        { State::Closed,        "Closed" }
    };
    static std::string const invalid("Invalid");

    auto it = table.find(state);
    return table.end() != it ? it->second : invalid;
}

std::string const& hlib::ws::to_string(Opcode opcode)
{
    static std::unordered_map<Opcode, std::string> const table =
    {
        { Opcode::Continuation, "Continuation" },
        { Opcode::Text,         "Text" },
        { Opcode::Binary,       "Binary" },
        { Opcode::Close,        "Close" },
        { Opcode::Ping,         "Ping" },
        { Opcode::Pong,         "Pong" }
    };
    static std::string const invalid("Invalid");

    auto it = table.find(opcode);
    return table.end() != it ? it->second : invalid;
}

