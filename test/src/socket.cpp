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
#include "hlib/socket.hpp"

using namespace hlib;

TEST_CASE("Socket", "[socket]")
{
    auto event_loop = std::make_shared<EventLoop>();

    Socket server_connection(event_loop);
    server_connection.receive(std::make_shared<BufferSink>(0), [&](auto const& sink) {
        auto const& buffer = std::static_pointer_cast<BufferSink>(sink)->buffer();

        REQUIRE("So Long, and Thanks for All the Fish" == to_string(*buffer));
        event_loop->interrupt();
    });

    Socket server(event_loop);
    server.listen(SockAddr("0.0.0.0:6502"), SOCK_STREAM, 0, 1, Socket::ReusePort);
    server.setAcceptCallback([&](UniqueHandle<int, -1> fd, SockAddr const& /*address*/) {
        server_connection.open(std::move(fd));
        server_connection.send(std::make_shared<BufferSource>("Hello World!"), nullptr);
    });

    Socket client(event_loop);
    client.connect(SockAddr("0.0.0.0:6502"), SOCK_STREAM, 0, 0);
    client.receive(std::make_shared<BufferSink>(0), [&](auto const& sink) {
        auto const& buffer = std::static_pointer_cast<BufferSink>(sink)->buffer();

        REQUIRE("Hello World!" == to_string(*buffer));
        client.send(std::make_shared<BufferSource>("So Long, and Thanks for All the Fish"), nullptr);
    });

    event_loop->dispatch();
}

TEST_CASE("Socket Fixed Size", "[socket]")
{
    auto event_loop = std::make_shared<EventLoop>();

    Socket server_connection(event_loop);
    server_connection.receive(std::make_shared<BufferSink>(3), [&](auto const& sink) {
        auto const& buffer = std::static_pointer_cast<BufferSink>(sink)->buffer();

        REQUIRE("123" == to_string(*buffer));
        event_loop->interrupt();
    });

    Socket server(event_loop);
    server.listen(SockAddr("0.0.0.0:6502"), SOCK_STREAM, 0, 1, Socket::ReusePort);
    server.setAcceptCallback([&](UniqueHandle<int, -1> fd, SockAddr const& /*address*/) {
        server_connection.open(std::move(fd));
        server_connection.send(std::make_shared<BufferSource>("test"), nullptr);
    });

    Socket client(event_loop);
    client.connect(SockAddr("0.0.0.0:6502"), SOCK_STREAM, 0, 0);
    client.receive(std::make_shared<BufferSink>(4), [&](auto const& sink) {
        auto const& buffer = std::static_pointer_cast<BufferSink>(sink)->buffer();

        REQUIRE("test" == to_string(*buffer));
        client.send(std::make_shared<BufferSource>("123"), nullptr);
    });

    event_loop->dispatch();
}

