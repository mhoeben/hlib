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
#include "hlib/file.hpp"
#include "hlib/format.hpp"
#include "hlib/file_server.hpp"
#include "hlib/subprocess.hpp"
#include <filesystem>

using namespace hlib;

TEST_CASE("File Server", "[file,http]")
{
    REQUIRE(true == test::is_curl_installed());

    auto event_loop = std::make_shared<EventLoop>();
    http::Server server("SERVER", event_loop);

    http::Server::Config config;
    config.binding = SockAddr("127.0.0.1:6502");
    config.socket_options = http::Server::ReusePort;
    config.secure = false;
    config.on_transaction_start = [&](http::Server::Transaction& transaction) noexcept
    {
        REQUIRE("/test.txt" == transaction.request_target);

        std::filesystem::path filepath(fmt::format("/tmp/hlib_test{}", transaction.request_target));

        switch (transaction.request_method) {
        case http::Method::Get:
            transaction.delegateTo<file::server::GetFile>(std::move(filepath));
            break;

        case http::Method::Put:
            transaction.delegateTo<file::server::PutFile>(std::move(filepath));
            break;

        case http::Method::Delete:
            transaction.delegateTo<file::server::DeleteFile>(std::move(filepath));
            break;

        default:
            REQUIRE(false);
        }
    };

    server.start(config);

    std::thread thread([&] {
        event_loop->dispatch();
    });

    std::filesystem::create_directory("/tmp/hlib_test");

    Subprocess curl;

    curl.run("curl", { "-X", "PUT", "-H", "\"Content-Type: text/plain\"", "-d", "Hello World", "http://127.0.0.1:6502/test.txt" });
    REQUIRE(0 == curl.returnCode());

    curl.run("curl", { "http://127.0.0.1:6502/test.txt" });
    REQUIRE(0 == curl.returnCode());
    REQUIRE("Hello World" == to_string(curl.output()));

    curl.run("curl", { "-X", "DELETE", "http://127.0.0.1:6502/test.txt" });
    REQUIRE(0 == curl.returnCode());

    event_loop->interrupt();
    thread.join();
}

