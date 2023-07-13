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
#include "hlib/web_socket.hpp"

using namespace hlib;

namespace {

bool is_uwsc_installed()
{
    return 0 == Subprocess("uwsc", { "--version" }).returnCode();
}

} // namespace

TEST_CASE("WebSocket", "[websocket]")
{
    REQUIRE(true == is_uwsc_installed());

    auto event_loop = std::make_shared<EventLoop>(log::Domain("EVENTLOOP"));

    ws::Server ws_server(log::Domain("WS-SERVER"), event_loop);
    ws_server.start();

    http::Server http_server(log::Domain("HTTP-SERVER"), event_loop);
    http::Server::Config config;
    config.binding = SockAddr("127.0.0.1:6502");
    config.secure = false;
    config.on_start_transaction = [&](http::Server::Transaction& transaction) noexcept
    {
        std::optional<std::vector<std::string>> protocols = ws::is_upgrade(transaction);
        REQUIRE("test" == protocols->at(0));

        ws::upgrade(transaction, protocols->at(0));
    };
    config.on_end_transaction = [&](http::Server::Transaction& transaction, bool failed) noexcept
    {
        REQUIRE(false == failed);

        ws::Server::Socket& socket = ws_server.add(transaction.upgraded());

        socket.setMessageCallback([&](ws::Server::Socket& sock, ws::Message& message) {
            REQUIRE("Hello World" == std::get<std::string>(message));
            sock.close();
        });

        socket.setCloseCallback([&](ws::Server::Socket const& /* sock */, bool /* clean */, uint16_t /* code */, Buffer const& /* reason */) {
            // TODO REQUIRE(true == clean);
            event_loop->interrupt();
        });
    };
    http_server.start(config);

    std::thread thread([&] {
        event_loop->dispatch();
    });

    system("echo \"Hello World\" | uwsc -p test ws://localhost:6502/ >/dev/null");

    thread.join();
}

