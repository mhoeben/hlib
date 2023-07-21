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
#include "test.hpp"
#include "hlib/event_loop.hpp"
#include "hlib/http_server.hpp"
#include "hlib/subprocess.hpp"
#include <thread>

using namespace hlib;

TEST_CASE("HTTP Canonicalize", "[http]")
{
    REQUIRE("/" == http::canonicalize("").value());
    REQUIRE("/" == http::canonicalize("/").value());
    REQUIRE("/" == http::canonicalize("//").value());
    REQUIRE("/" == http::canonicalize("/.").value());
    REQUIRE("/" == http::canonicalize("/./").value());
    REQUIRE("/" == http::canonicalize("/./.").value());
    REQUIRE("/foo" == http::canonicalize("/foo").value());
    REQUIRE("/foo" == http::canonicalize("/foo/").value());
    REQUIRE("/foo" == http::canonicalize("/foo/.").value());
    REQUIRE("/foo" == http::canonicalize("/foo//").value());
    REQUIRE("/foo" == http::canonicalize("/foo/./").value());
    REQUIRE("/foo/bar" == http::canonicalize("/foo/bar").value());
    REQUIRE("/foo/bar" == http::canonicalize("/foo/bar/").value());
    REQUIRE("/foo/bar" == http::canonicalize("/foo/bar/.").value());
    REQUIRE("/foo/bar" == http::canonicalize("/foo/bar//").value());
    REQUIRE("/foo/bar" == http::canonicalize("/foo/bar//.").value());
    REQUIRE("/foo/bar" == http::canonicalize("/foo//bar").value());
    REQUIRE("/foo/bar" == http::canonicalize("/foo/./bar").value());
    REQUIRE("/foo/bar" == http::canonicalize("/foo/./bar/").value());
    REQUIRE("/foo/bar" == http::canonicalize("/foo/./bar/.").value());
    REQUIRE("/foo/bar" == http::canonicalize("/foo/./bar//").value());
    REQUIRE("/foo" == http::canonicalize("/foo/bar/..").value());
    REQUIRE("/foo" == http::canonicalize("/foo/bar/../").value());
    REQUIRE("/foo" == http::canonicalize("/foo//bar/..").value());
    REQUIRE("/foo" == http::canonicalize("/foo//bar/../").value());
    REQUIRE("/foo" == http::canonicalize("/foo/./bar/..").value());
    REQUIRE("/foo" == http::canonicalize("/foo/./bar/../").value());
    REQUIRE("/foo" == http::canonicalize("/foo/bar/../.").value());
    REQUIRE("/foo" == http::canonicalize("/foo/bar/..//").value());
    REQUIRE("/bar" == http::canonicalize("/foo/../bar").value());
    REQUIRE("/bar" == http::canonicalize("/foo//../bar").value());
    REQUIRE("/bar" == http::canonicalize("/foo/..//bar").value());
    REQUIRE("/bar" == http::canonicalize("/foo/.././bar").value());
    REQUIRE("/bar" == http::canonicalize("/foo/../bar/").value());
    REQUIRE("/bar" == http::canonicalize("/foo/../bar/.").value());
    REQUIRE("/bar" == http::canonicalize("/foo/../bar//").value());

    REQUIRE(false == http::canonicalize("/..").has_value());
    REQUIRE(false == http::canonicalize("/foo/../..").has_value());
    REQUIRE(false == http::canonicalize("/foo/../../bar").has_value());
    REQUIRE(false == http::canonicalize("/foo/../../bar/..").has_value());
    REQUIRE(false == http::canonicalize("/foo/.././..").has_value());
    REQUIRE(false == http::canonicalize("/foo/.././../bar").has_value());
    REQUIRE(false == http::canonicalize("/foo/.././../bar/").has_value());
    REQUIRE(false == http::canonicalize("/foo/.././../bar/.").has_value());
    REQUIRE(false == http::canonicalize("/foo/../bar/../..").has_value());
}

TEST_CASE("HTTP Server", "[http]")
{
    REQUIRE(true == test::is_curl_installed());

    auto event_loop = std::make_shared<EventLoop>();
    http::Server server("HTTP_SERVER", event_loop);

    http::Server::Config config;
    config.binding = SockAddr("127.0.0.1:6502");
    config.secure = false;
    config.on_transaction_start = [&](http::Server::Transaction& transaction) noexcept
    {
        REQUIRE(http::Method::Get == transaction.request_method);
        REQUIRE("/" == transaction.request_target);
        REQUIRE("HTTP/1.1" == transaction.request_version);
        REQUIRE(0 == transaction.request_content_length);

        transaction.respond(http::StatusCode::Ok, {}, std::make_unique<Buffer>("Hello World"));
    };
    config.on_transaction_end = [&](http::Server::Transaction const& transaction, bool failed) noexcept
    {
        REQUIRE(false == failed);
        REQUIRE(http::StatusCode::Ok == transaction.responseStatusCode());
        event_loop->interrupt();
    };

    server.start(config);

    std::thread thread([&] {
        event_loop->dispatch();
    });

    Subprocess curl("curl", { "http://localhost:6502" });
    REQUIRE(0 == curl.returnCode());
    REQUIRE("Hello World" == to_string(curl.output()));

    thread.join();
}

