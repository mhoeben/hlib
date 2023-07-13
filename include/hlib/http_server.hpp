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
#include "hlib/log.hpp"
#include "hlib/sock_addr.hpp"
#include <functional>
#include <memory>
#include <openssl/ssl.h>
#include <optional>
#include <vector>
#include <unordered_map>

extern "C"
{

struct hserv_s;
struct hserv_session_s;

} // extern "C"

namespace hlib
{

class EventLoop;

namespace http
{

enum class StatusCode
{
    Continue = 100,
    SwitchingProtocols = 101,
    EarlyHints = 103,

    Ok = 200,
    Created = 201,
    Accepted = 202,
    NonAuthoritativeInformation = 203,
    NoContent = 204,
    ResetContent = 205,
    PartialContent = 206,

    MultipleChoices = 300,
    MovedPermanently = 301,
    Found = 302,
    SeeOther = 303,
    NotModified = 304,
    TemporaryRedirect = 307,
    PermanentRedirect = 308,

    BadRequest = 400,
    Unauthorized = 401,
    PaymentRequired = 402,
    Forbidden = 403,
    NotFound = 404,
    MethodNotAllowed = 405,
    NotAcceptable = 406,
    ProxyAuthenticationRequired = 407,
    RequestTimeout = 408,
    Conflict = 409,
    Gone = 410,
    LengthRequired = 411,
    PreconditionFailed = 412,
    PayloadTooLarge = 413,
    UriTooLong = 414,
    UnsupportedMediaType = 415,
    RangeNotSatisfiable = 416,
    ExpectationFailed = 417,
    ImA_teapot = 418,
    UnprocessableEntity = 422,
    TooEarly = 425,
    UpgradeRequired = 426,
    PreconditionRequired = 428,
    TooManyRequests = 429,
    RequestHeaderFieldsTooLarge = 431,
    UnavailableForLegalReasons = 451,

    InternalServerError = 500,
    NotImplemented = 501,
    BadGateway = 502,
    ServiceUnavailable = 503,
    GatewayTimeout = 504,
    HttpVersionNotSupported = 505,
    VariantAlsoNegotiates = 506,
    InsufficientStorage = 507,
    LoopDetected = 508,
    NotExtended = 510,
    NetworkAuthenticationRequired = 511
};

std::string to_string(StatusCode status_code);

struct HeaderField final
{
    std::string name;
    std::string value;
};

std::string to_string(HeaderField const& header_field);

static constexpr std::size_t ChunkedTransferEncoding{ ~std::size_t(0) };

class Upgrade final
{
    HLIB_NOT_COPYABLE(Upgrade);

public:
    std::string subprotocol;
    int fd{ -1 };
    SSL* ssl{ nullptr };

    Upgrade() noexcept = default;
    Upgrade(std::string a_subprotocol, int a_fd, SSL* a_ssl) noexcept;
    Upgrade(Upgrade&& that) noexcept;
    ~Upgrade();

    Upgrade& operator = (Upgrade&& that) noexcept;
};

class Server final
{
    HLIB_NOT_COPYABLE(Server);
    HLIB_NOT_MOVABLE(Server);

public:
    class Transaction;

    typedef std::function<void(Transaction& transaction)> StartTransactionCallback;
    typedef std::function<void(Transaction& transaction, bool failed)> EndTransactionCallback;

    typedef std::function<void(Transaction& transaction, std::shared_ptr<Buffer> buffer, std::size_t more)> RequestContentCallback;
    typedef std::function<void(Transaction& transaction, std::shared_ptr<Buffer const> buffer, std::size_t more)> ResponseContentCallback;

    class Transaction final
    {
        friend class Server;

        HLIB_NOT_COPYABLE(Transaction);
        HLIB_NOT_MOVABLE(Transaction);

    public:
        typedef uint64_t Id;

        Server& server;
        Transaction::Id const id;
        std::string const request_method;
        std::string const request_target;
        std::string const request_version;
        std::size_t const request_content_length;

        std::shared_ptr<void> user;

        std::optional<std::string> getRequestValue(std::string const& name, std::size_t index = 0) const;
        bool containsRequestValue(std::string const& name, std::string const& value, std::string const& delim) const noexcept;

        void receive(std::shared_ptr<Buffer> content, RequestContentCallback callback);

        void respond(StatusCode status_code, std::string reason, std::vector<HeaderField> const& header_fields, std::size_t content_length);
        void respond(StatusCode status_code, std::vector<HeaderField> const& header_fields, std::shared_ptr<Buffer const> content = nullptr);

        void send(std::shared_ptr<Buffer const> content, ResponseContentCallback callback);

        Upgrade upgraded();

    private:
        struct hserv_s* m_hserv;
        struct hserv_session_s* m_session;

        Buffer m_request_fields;

        std::shared_ptr<Buffer> m_request_content;
        RequestContentCallback m_on_request_content;

        std::shared_ptr<Buffer const> m_response_content;
        ResponseContentCallback m_on_response_content;

        EndTransactionCallback m_on_end_transaction;

        Transaction(Server& a_server, struct hserv_s* a_hserv, struct hserv_session_s* a_session,
            Id a_id, EndTransactionCallback a_on_end_transaction);

        std::vector<char const*> toFieldsArray(std::vector<HeaderField> const& header_fields) const;
        int onRequestContent(void* buffer, std::size_t size, std::size_t more);
        int onResponseContent(void const* buffer, std::size_t size, std::size_t more);

        void finish(bool failed);
    };

    struct Config
    {
        SockAddr binding;

        bool secure;
        std::string certificate_file;
        std::string private_key_file;

        StartTransactionCallback on_start_transaction;
        EndTransactionCallback on_end_transaction;

        Config();
    };

public:
    Server(log::Domain a_logger, std::weak_ptr<EventLoop> a_event_loop);
    ~Server();

    std::shared_ptr<EventLoop> getEventLoop() const;
    std::optional<std::reference_wrapper<Transaction>> getTransaction(Transaction::Id id) const;

    void addPath(std::string path, StartTransactionCallback on_start_transaction, EndTransactionCallback on_end_transaction);
    void removePath(std::string const& path);

    void start(Config const& config);
    void stop();

private:
    log::Domain const m_logger;
    std::weak_ptr<EventLoop> const m_event_loop;
    struct hserv_s* m_hserv{ nullptr };

    Transaction::Id m_transaction_id{ 0 };
    std::unordered_map<Transaction::Id, std::unique_ptr<Transaction>> m_transactions;

    struct Callbacks
    {
        StartTransactionCallback on_start_transaction;
        EndTransactionCallback on_end_transaction;
    };
    Callbacks m_callbacks;

    std::unordered_map<std::string, Callbacks> m_path_callbacks;

    void onPoll(int fd, std::uint32_t events);
    int onRequestStart(struct hserv_session_s* session);
    void onRequestEnd(struct hserv_session_s* session, int failed);
};

std::optional<std::string> canonicalize(std::string target);

std::optional<std::string> is_upgrade(Server::Transaction const& transaction);


} // namespace http
} // namespace hlib
