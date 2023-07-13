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
#include "catch2/catch_test_macros.hpp"
#include "hlib/event_loop.hpp"
#include "hlib/http_server.hpp"
#include "hlib/subprocess.hpp"
#include <thread>

using namespace hlib;

namespace
{

bool is_curl_installed()
{ 
    return 0 == Subprocess("curl", { "--version" }).returnCode();
}

} // namespace

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
    static std::string const& test_string = "Hello World";

    REQUIRE(true == is_curl_installed());

    auto event_loop = std::make_shared<EventLoop>(log::Domain("EVENTLOOP"));
    http::Server server(log::Domain("HTTP-SERVER"), event_loop);

    http::Server::Config config;
    config.binding = SockAddr("127.0.0.1:6502");
    config.secure = false;
    config.on_transaction_start = [&](http::Server::Transaction& transaction) noexcept
    {
        REQUIRE("GET" == transaction.request_method);
        REQUIRE("/" == transaction.request_target);
        REQUIRE("HTTP/1.1" == transaction.request_version);
        REQUIRE(0 == transaction.request_content_length);

        transaction.respond(http::StatusCode::Ok, {}, std::make_unique<Buffer>(test_string));
    };
    config.on_transaction_end = [&](http::Server::Transaction const& /* transaction */, bool failed) noexcept
    {
        REQUIRE(false == failed);
        event_loop->interrupt();
    };

    server.start(config);

    std::thread thread([&] {
        event_loop->dispatch();
    });

    Subprocess request("curl", { "http://localhost:6502" });
    REQUIRE(0 == request.returnCode());
    REQUIRE(test_string == to_string(request.output()));

    thread.join();
}

