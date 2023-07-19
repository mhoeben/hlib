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
#include "hlib/sock_addr.hpp"

using namespace hlib;

TEST_CASE("SockAddr", "[sockaddr]")
{
    SockAddr sa;

    REQUIRE("127.0.0.1"     == to_string(SockAddr("127.0.0.1")));
    REQUIRE("127.0.0.1:80"  == to_string(SockAddr("127.0.0.1:80")));

    REQUIRE("::1"                               == to_string(SockAddr("::1")));
    REQUIRE("[::1]:80"                          == to_string(SockAddr("[::1]:80")));
    REQUIRE("fe80::1b39:432b:a559:b42c"         == to_string(SockAddr("fe80::1b39:432b:a559:b42c")));
    REQUIRE("[fe80::1b39:432b:a559:b42c]:80"    == to_string(SockAddr("[fe80::1b39:432b:a559:b42c]:80")));
    REQUIRE("::ffff:192.168.1.251"              == to_string(SockAddr("::ffff:192.168.1.251")));
    REQUIRE("[::ffff:192.168.1.251]:80"         == to_string(SockAddr("[::ffff:192.168.1.251]:80")));
}

