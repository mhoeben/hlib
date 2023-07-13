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
#include "hlib/http_server.hpp"
#include "hlib/config.hpp"
#include "hlib/error.hpp"
#include "hlib/event_loop.hpp"
#include "hlib/format.hpp"
#include "hlib/memory.hpp"
#include "hlib/string.hpp"
#include <limits>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

#define HSERV_VISIBILITY_STATIC
#define HSERV_HAVE_OPENSSL
#define HSERV_IMPL
#include "../third_party/hserv/include/hserv.h"

#pragma GCC diagnostic pop

using namespace hlib;
using namespace hlib::http;

//
// Public (Upgrade)
//
Upgrade::Upgrade(std::string a_subprotocol, int a_fd, SSL* a_ssl) noexcept
    : subprotocol(std::move(a_subprotocol))
    , fd{ a_fd }
    , ssl(a_ssl)
{
}

Upgrade::Upgrade(Upgrade&& that) noexcept
    : subprotocol(std::move(that.subprotocol))
    , fd{ that.fd }
    , ssl(that.ssl)
{
    that.fd = -1;
    that.ssl = nullptr;
}

Upgrade::~Upgrade()
{
    if (nullptr != ssl) {
        SSL_free(ssl);
    }
    if (fd >= 0) {
        close(fd);
    }
}

Upgrade& Upgrade::operator = (Upgrade&& that) noexcept
{
    if (nullptr != ssl) {
        SSL_free(ssl);
        ssl = nullptr;
    }
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
    subprotocol.clear();

    std::swap(subprotocol, that.subprotocol);
    std::swap(fd, that.fd);
    std::swap(ssl, that.ssl);
    return *this;
}

//
// Implementation (Transaction)
//
Server::Transaction::Transaction(Server& a_server, hserv_s* a_hserv, hserv_session_t* a_session,
    Id a_id, EndTransactionCallback a_on_end_transaction)
    : server(a_server)
    , id{ a_id }
    , request_method(hserv_request_get_method(a_session))
    , request_target(hserv_request_get_target(a_session))
    , request_version(hserv_request_get_version(a_session))
    , request_content_length{
        HSERV_CHUNKED == hserv_request_get_content_length(a_session)
        ? std::numeric_limits<std::size_t>::max()
        : hserv_request_get_content_length(a_session) }
    , m_hserv(a_hserv)
    , m_session(a_session)
    , m_on_end_transaction(std::move(a_on_end_transaction))
{
    int size = hserv_header_fields_copy(
        hserv_request_get_header_fields(a_session),
        static_cast<char*>(m_request_fields.reserve(HSERV_MAX_HEADERS_LENGTH)),
        HSERV_MAX_HEADERS_LENGTH
    );
    assert(size >= 0);
    m_request_fields.resize(size);
}

std::vector<char const*> Server::Transaction::toFieldsArray(std::vector<HeaderField> const& header_fields) const
{
    std::vector<char const*> fields;
    fields.reserve(header_fields.size() * 2 + 1);

    // Convert header fields to hserv's representation.
    for (auto const& header_field : header_fields) {
        fields.push_back(header_field.name.c_str());
        fields.push_back(header_field.value.c_str());
    }
    fields.push_back(nullptr);
    return fields;
}

int Server::Transaction::onRequestContent(void* buffer, std::size_t size, std::size_t more)
{
    try {
        assert(static_cast<std::uint8_t const*>(m_request_content->data()) + m_request_content->size() == buffer);
        assert(m_request_content->size() + size <= m_request_content->capacity());

        // Remove callback and content buffer from transaction.
        RequestContentCallback callback(std::move(m_on_request_content));
        std::shared_ptr<Buffer> content(std::move(m_request_content));

        // Resize content to received size.
        content->resize(content->size() + size);

        // Callback to user.
        callback(*this, std::move(content), more);
        return 0;
    }
    catch (...) {
        HLOGE(server.m_logger, "Exception in request content callback");
        return -1;
    }
}

int Server::Transaction::onResponseContent(void const* buffer, std::size_t size, std::size_t more)
{
    try {
        assert(m_response_content->data() == buffer);
        assert(m_response_content->size() == size);

        // Remove callback and content buffer from transaction.
        ResponseContentCallback callback(std::move(m_on_response_content));
        std::shared_ptr<Buffer const> content(std::move(m_response_content));

        // Callback to user.
        callback(*this, std::move(content), more);
        return 0;
    }
    catch (...) {
        HLOGE(server.m_logger, "Exception in response content callback");
        return -1;
    }
}

void Server::Transaction::finish(bool failed)
{
    if (nullptr == m_on_end_transaction) {
        return;
    }

    m_on_end_transaction(*this, failed);
}

//
// Public (Transaction)
//
SockAddr Server::Transaction::getPeerAddress() const
{
    sockaddr_storage storage;
    socklen_t length = sizeof(storage);

    hverify(-1 != hserv_session_get_peer(m_session, reinterpret_cast<sockaddr*>(&storage), &length));
    return SockAddr(storage);
}

std::optional<std::string> Server::Transaction::getRequestValue(std::string const& name, std::size_t index) const
{
    assert(true == callback_from(server.m_event_loop));

    char const* it = static_cast<char const*>(m_request_fields.data());
    char const* value = nullptr;

    do {
        it = hserv_header_field_find(it, name.c_str(), &value);
        if (nullptr == it) {
            return std::optional<std::string>();
        }
    }
    while (index-- > 0);

    assert(nullptr != value);
    return value;
}

bool Server::Transaction::containsRequestValue(std::string const& name, std::string const& value, std::string const& delim) const noexcept
{
    assert(true == callback_from(server.m_event_loop));

    return hserv_header_field_contains(
        static_cast<char const*>(m_request_fields.data()),
        name.c_str(),
        value.c_str(),
        delim.c_str()
    ) > 0;
}

void Server::Transaction::receive(std::shared_ptr<Buffer> content, RequestContentCallback callback)
{
    assert(nullptr != content);
    assert(content->capacity() > content->size());
    assert(nullptr != callback);

    assert(request_content_length > 0);
    assert(nullptr == m_request_content);
    assert(nullptr == m_on_request_content);
    assert(nullptr == m_response_content);
    assert(nullptr == m_on_response_content);

    assert(true == callback_from(server.m_event_loop));

    // Move arguments to transaction.
    m_request_content = std::move(content);
    m_on_request_content = std::move(callback);

    // Receive to content buffer.
    hverify(hserv_request_receive(
        m_hserv,
        m_session, 
        static_cast<std::uint8_t*>(m_request_content->reserve(m_request_content->capacity())) + m_request_content->size(),
        m_request_content->capacity() - m_request_content->size(),
        [](hserv_t* /* hserv */, hserv_session_t* session, void* buffer, std::size_t size, std::size_t more) -> int
        {
            return static_cast<Transaction*>(hserv_session_get_user_data(session))->onRequestContent(buffer, size, more);
        }
    ) != -1);
}

void Server::Transaction::respond(StatusCode status_code, std::string reason, std::vector<HeaderField> const& header_fields, std::size_t content_length)
{
    assert(nullptr == m_response_content);
    assert(nullptr == m_on_response_content);

    assert(true == callback_from(server.m_event_loop));

    // Respond with content callbacks.
    hverify(hserv_respond(
        m_hserv,
        m_session,
        static_cast<hserv_status_code_t>(status_code),
        false == reason.empty() ? reason.c_str() : nullptr,
        toFieldsArray(header_fields).data(),
        content_length < HSERV_CHUNKED ? content_length : HSERV_CHUNKED,
        nullptr
    ) != -1);
}

void Server::Transaction::respond(StatusCode status_code, std::vector<HeaderField> const& header_fields, std::shared_ptr<Buffer const> content)
{
    assert(nullptr == m_response_content);
    assert(nullptr == m_on_response_content);

    assert(true == callback_from(server.m_event_loop));

    m_response_content = std::move(content);

    // Respond with content.
    hverify(hserv_respond(
        m_hserv,
        m_session,
        static_cast<hserv_status_code_t>(status_code),
        nullptr,
        toFieldsArray(header_fields).data(),
        m_response_content ? m_response_content->size() : 0,
        m_response_content ? m_response_content->data() : nullptr
    ) != -1);
}

void Server::Transaction::send(std::shared_ptr<Buffer const> content, ResponseContentCallback callback)
{
    assert(nullptr != content);
    assert(content->size() > 0);
    assert(nullptr != callback);

    assert(nullptr == m_response_content);
    assert(nullptr == m_on_response_content);

    assert(true == callback_from(server.m_event_loop));

    // Move arguments to transaction.
    m_response_content = std::move(content);
    m_on_response_content = std::move(callback);

    // Send content.
    hverify(hserv_response_send(
        m_hserv,
        m_session,
        m_response_content->data(),
        m_response_content->size(), 
        [](hserv_t* /* hserv */, hserv_session_t* session, void const* buffer, std::size_t size, std::size_t more) -> int
        {
            return static_cast<Transaction*>(hserv_session_get_user_data(session))->onResponseContent(buffer, size, more);
        }
    ) != -1);
}

Upgrade Server::Transaction::upgraded()
{
    SSL* ssl = hserv_session_get_ssl(m_session);

    return Upgrade(
        getRequestValue("Upgrade").value(),
        hserv_session_upgraded(m_hserv, m_session)
        , ssl
    );
}

//
// Implementation (Server)
//
void Server::onPoll(int fd, std::uint32_t events)
{
    assert(hserv_get_fd(m_hserv) == fd); (void)fd;
    assert(EventLoop::Read == events);   (void)events;

    if (hserv_poll(m_hserv) < 0) {
        throwf<std::runtime_error>("HTTP server poll failed ({})", get_error_string(errno));
    }
}

int Server::onRequestStart(hserv_session_t* session)
{
    try {
        Transaction::Id const transaction_id = ++m_transaction_id;
        Callbacks const* callbacks = &m_callbacks;

        // Canonicalize target and lookup callbacks.
        std::optional<std::string> target = canonicalize(hserv_request_get_target(session));
        if (true == target.has_value()) {
            auto it = m_path_callbacks.find(target.value());
            if (m_path_callbacks.end() != it) {
                callbacks = &it->second;
            }
        }

        // Create a new transaction and callback user.
        std::unique_ptr<Transaction> transaction(new Transaction(*this, m_hserv, session,
            transaction_id, callbacks->on_end_transaction));
        callbacks->on_start_transaction(*transaction);

        // Set transaction as session's user data.
        hserv_session_set_user_data(session, transaction.get());

        // Store transaction.
        m_transactions.emplace(transaction_id, std::move(transaction));
        return 0;
    }
    catch (...) {
        HLOGE(m_logger, "Exception in start transaction callback");
        return -1;
    }
}

void Server::onRequestEnd(hserv_session_t* session, int failed)
{
    try {
        auto transaction = static_cast<Transaction*>(hserv_session_get_user_data(session));

        // Callback.
        transaction->finish(!!failed);

        // Erase transaction from map.
        m_transactions.erase(transaction->id);
    }
    catch (...) {
        HLOGE(m_logger, "Exception in start transaction callback");
    }
}

//
// Public
//
Server::Config::Config()
    : binding{ hlib::Config::httpServerBinding() }
    , secure{ hlib::Config::httpServerSecure() }
{
}

Server::Server(log::Domain logger, std::weak_ptr<EventLoop> event_loop)
    : m_logger(std::move(logger))
    , m_event_loop(std::move(event_loop))
{
}

Server::~Server()
{
    stop();

    hserv_destroy(m_hserv);
}

std::shared_ptr<EventLoop> Server::getEventLoop() const
{
    return m_event_loop.lock();
}

std::optional<std::reference_wrapper<Server::Transaction>> Server::getTransaction(Transaction::Id id) const
{
    assert(true == callback_from(m_event_loop));

    auto it = m_transactions.find(id);
    return m_transactions.end() == it
        ? *it->second
        : std::optional<std::reference_wrapper<Transaction>>();
}

void Server::addPath(std::string path, StartTransactionCallback on_start_transaction, EndTransactionCallback on_end_transaction)
{
    m_path_callbacks.emplace(std::move(path), Callbacks{ std::move(on_start_transaction), std::move(on_end_transaction) });
}

void Server::removePath(std::string const& path)
{
    m_path_callbacks.erase(path);
}

void Server::start(Config const& config)
{
    using namespace std::placeholders;

    assert(nullptr == m_hserv);

    hserv_config_t hserv_config;

    // Default initialize hserv config.
    hserv_init(
        &hserv_config,
        [](hserv_t* hserv, hserv_session_t* session) -> int
        {
            return static_cast<Server*>(hserv_get_user_data(hserv))->onRequestStart(session);
        },
        [](hserv_t* hserv, hserv_session_t* session, int failed) -> void
        {
            static_cast<Server*>(hserv_get_user_data(hserv))->onRequestEnd(session, failed);
        }
    );

    // Set binding.
    memcpy(&hserv_config.binding, static_cast<sockaddr const*>(config.binding), sizeof(sockaddr));

    // Set security.
    if (true == config.secure) {
        hserv_config.secure = true;

        assert(false == config.certificate_file.empty());
        assert(false == config.private_key_file.empty());

        hserv_config.certificate_file = config.certificate_file.c_str();
        hserv_config.private_key_file = config.private_key_file.c_str();
    }

    // Set this as user data.
    hserv_config.user_data = this;

    // Create hserv server.
    m_hserv = hserv_create(&hserv_config);
    if (nullptr == m_hserv) {
        throw std::runtime_error("Failed to create HTTP server");
    }

    // Add hserv's poll fd to event loop.
    hverify(true == with_weak_ptr_locked(m_event_loop, [this](EventLoop& event_loop) {
        event_loop.add(
            hserv_get_fd(m_hserv),
            EventLoop::Read,
            std::bind(&Server::onPoll, this, _1, _2)
        );
    }));

    assert(nullptr != config.on_start_transaction);
    m_callbacks.on_start_transaction = config.on_start_transaction;
    m_callbacks.on_end_transaction = config.on_end_transaction;
}

void Server::stop()
{
    if (nullptr == m_hserv) {
        return;
    }

    hverify(true == with_weak_ptr_locked(m_event_loop, [this](EventLoop& event_loop) {
        event_loop.remove(hserv_get_fd(m_hserv));
    }));
}

//
// Public (Utility)
//
std::string http::to_string(StatusCode status_code)
{
    static std::unordered_map<StatusCode, std::string> const table = {
        { StatusCode::Continue, "Continue" },
        { StatusCode::SwitchingProtocols, "SwitchingProtocols" },
        { StatusCode::EarlyHints, "EarlyHints" },

        { StatusCode::Ok, "Ok" },
        { StatusCode::Created, "Created" },
        { StatusCode::Accepted, "Accepted" },
        { StatusCode::NonAuthoritativeInformation, "NonAuthoritativeInformation" },
        { StatusCode::NoContent, "NoContent" },
        { StatusCode::ResetContent, "ResetContent" },
        { StatusCode::PartialContent, "PartialContent" },

        { StatusCode::MultipleChoices, "MultipleChoices" },
        { StatusCode::MovedPermanently, "MovedPermanently" },
        { StatusCode::Found, "Found" },
        { StatusCode::SeeOther, "SeeOther" },
        { StatusCode::NotModified, "NotModified" },
        { StatusCode::TemporaryRedirect, "TemporaryRedirect" },
        { StatusCode::PermanentRedirect, "PermanentRedirect" },

        { StatusCode::BadRequest, "BadRequest" },
        { StatusCode::Unauthorized, "Unauthorized" },
        { StatusCode::PaymentRequired, "PaymentRequired" },
        { StatusCode::Forbidden, "Forbidden" },
        { StatusCode::NotFound, "NotFound" },
        { StatusCode::MethodNotAllowed, "MethodNotAllowed" },
        { StatusCode::NotAcceptable, "NotAcceptable" },
        { StatusCode::ProxyAuthenticationRequired, "ProxyAuthenticationRequired" },
        { StatusCode::RequestTimeout, "RequestTimeout" },
        { StatusCode::Conflict, "Conflict" },
        { StatusCode::Gone, "Gone" },
        { StatusCode::LengthRequired, "LengthRequired" },
        { StatusCode::PreconditionFailed, "PreconditionFailed" },
        { StatusCode::PayloadTooLarge, "PayloadTooLarge" },
        { StatusCode::UriTooLong, "UriTooLong" },
        { StatusCode::UnsupportedMediaType, "UnsupportedMediaType" },
        { StatusCode::RangeNotSatisfiable, "RangeNotSatisfiable" },
        { StatusCode::ExpectationFailed, "ExpectationFailed" },
        { StatusCode::ImA_teapot, "ImA_teapot" },
        { StatusCode::UnprocessableEntity, "UnprocessableEntity" },
        { StatusCode::TooEarly, "TooEarly" },
        { StatusCode::UpgradeRequired, "UpgradeRequired" },
        { StatusCode::PreconditionRequired, "PreconditionRequired" },
        { StatusCode::TooManyRequests, "TooManyRequests" },
        { StatusCode::RequestHeaderFieldsTooLarge, "RequestHeaderFieldsTooLarge" },
        { StatusCode::UnavailableForLegalReasons, "UnavailableForLegalReasons" },

        { StatusCode::InternalServerError, "InternalServerError" },
        { StatusCode::NotImplemented, "NotImplemented" },
        { StatusCode::BadGateway, "BadGateway" },
        { StatusCode::ServiceUnavailable, "ServiceUnavailable" },
        { StatusCode::GatewayTimeout, "GatewayTimeout" },
        { StatusCode::HttpVersionNotSupported, "HttpVersionNotSupported" },
        { StatusCode::VariantAlsoNegotiates, "VariantAlsoNegotiates" },
        { StatusCode::InsufficientStorage, "InsufficientStorage" },
        { StatusCode::LoopDetected, "LoopDetected" },
        { StatusCode::NotExtended, "NotExtended" },
        { StatusCode::NetworkAuthenticationRequired, "NetworkAuthenticationRequired" },
    };

    auto it = table.find(status_code);
    return table.end() == it ? fmt::format("Unknown_{}", static_cast<int>(status_code)) : it->second;
}

std::string to_string(HeaderField const& header_field)
{
    return fmt::format("{}: {}", header_field.name, header_field.value);
}

std::optional<std::string> http::canonicalize(std::string target)
{
    std::vector<std::string> components = split(target, '/', true);
    if (true == components.empty()) {
        return "/";
    }

    std::vector<std::string> canonical;
    canonical.reserve(components.size());

    for (std::string& component : components) {
        if ("." == component) {
            continue;
        }

        if (".." == component) {
            if (true == canonical.empty()) {
                return std::optional<std::string>();
            }

            canonical.pop_back();
            continue;
        }

        canonical.emplace_back(std::move(component));
    }

    if (true == canonical.empty()) {
        return "/";
    }

    fmt::memory_buffer buffer;

    for (std::string& component : canonical) {
        format_to(buffer, "/{}", component);
    }

    return fmt::to_string(buffer);
}

std::optional<std::string> http::is_upgrade(Server::Transaction const& transaction)
{
    std::optional<std::string> value;

    if ("GET" != transaction.request_method) {
        return std::optional<std::string>();
    }

    value = transaction.getRequestValue("Connection");
    if (false == value.has_value() || false == iequals("upgrade", value.value())) {
        return std::optional<std::string>();
    }

    value = transaction.getRequestValue("Upgrade");
    if (false == value.has_value()) {
        return std::optional<std::string>();
    }

    if (0 != transaction.request_content_length) {
        return std::optional<std::string>();
    }

    return value;
}

