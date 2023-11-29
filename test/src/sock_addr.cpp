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
    auto test = [](sa_family_t family, std::string const& address, std::uint16_t port, std::string const& string)
    {
        SockAddr sa(string);

        REQUIRE(family == sa.family());
        REQUIRE(address == sa.address());
        REQUIRE(port == sa.port());
        REQUIRE(string == to_string(sa));
    };

    REQUIRE(true == SockAddr().empty());

    test(AF_INET, "0.0.0.0",                        0, "0.0.0.0");
    test(AF_INET, "0.0.0.0",                     8443, "0.0.0.0:8443");
    test(AF_INET, "127.0.0.1",                      0, "127.0.0.1");
    test(AF_INET, "127.0.0.1",                     80, "127.0.0.1:80");

    test(AF_INET6, "::",                            0, "::");
    test(AF_INET6, "::1",                           0, "::1");
    test(AF_INET6, "::1",                          80, "[::1]:80");
    test(AF_INET6, "fe80::1b39:432b:a559:b42c",     0, "fe80::1b39:432b:a559:b42c");
    test(AF_INET6, "fe80::1b39:432b:a559:b42c",    80, "[fe80::1b39:432b:a559:b42c]:80");
    test(AF_INET6, "::ffff:192.168.1.251",          0, "::ffff:192.168.1.251");
    test(AF_INET6, "::ffff:192.168.1.251",         80, "[::ffff:192.168.1.251]:80");
}

