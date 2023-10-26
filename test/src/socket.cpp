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

TEST_CASE("Sockets", "[socket]")
{
    auto event_loop = std::make_shared<EventLoop>();

    Socket accepted(event_loop);
    accepted.setReceiveCallback([&](Buffer& buffer) {
        REQUIRE("So Long, and Thanks for All the Fish" == to_string(buffer));
        event_loop->interrupt();
    });

    Socket server(event_loop);
    server.listen(SockAddr("0.0.0.0:6502"), SOCK_STREAM, 0, 1, Socket::ReusePort);
    server.setAcceptCallback([&](UniqueHandle<int, -1> fd, SockAddr const& /*address*/) {
        accepted.open(std::move(fd));
        accepted.send(Buffer("Hello World!"));
    });

    Socket client(event_loop);
    client.connect(SockAddr("0.0.0.0:6502"), SOCK_STREAM, 0, 0);
    client.setReceiveCallback([&](Buffer& buffer) {
        REQUIRE("Hello World!" == to_string(buffer));
        client.send(Buffer("So Long, and Thanks for All the Fish"));
    });

    event_loop->dispatch();
}

