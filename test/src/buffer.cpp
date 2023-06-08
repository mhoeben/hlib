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
#include "hlib/buffer.hpp"
#include "hlib/format.hpp"

using namespace hlib;

TEST_CASE("Buffer", "[buffer]")
{
    Buffer buffer;

    REQUIRE(nullptr == buffer.data());
    REQUIRE(0 == buffer.capacity());
    REQUIRE(0 == buffer.size());
    REQUIRE(true == buffer.empty());

    REQUIRE(nullptr != buffer.reserve(8));
    REQUIRE(nullptr != buffer.data());
    REQUIRE(8 == buffer.capacity());
    REQUIRE(0 == buffer.size());
    REQUIRE(true == buffer.empty());

    buffer.assign("ab", 2);
    REQUIRE(8 == buffer.capacity());
    REQUIRE(2 == buffer.size());
    REQUIRE(false == buffer.empty());

    buffer.append("gh", 2);
    REQUIRE(8 == buffer.capacity());
    REQUIRE(4 == buffer.size());

    buffer.insert(2, "cdef", 4);
    REQUIRE(8 == buffer.capacity());
    REQUIRE(8 == buffer.size());
    REQUIRE(0 == memcmp("abcdefgh", buffer.data(), 8));

    buffer.erase(2, 4);
    REQUIRE(8 == buffer.capacity());
    REQUIRE(4 == buffer.size());
    REQUIRE(0 == memcmp("abgh", buffer.data(), 4));

    buffer.shrink();
    REQUIRE(4 == buffer.capacity());
    REQUIRE(4 == buffer.size());
    REQUIRE(0 == memcmp("abgh", buffer.data(), 4));
}

TEST_CASE("Buffer Iterator", "[buffer,format]")
{
    Buffer buffer;
    buffer.append("foo", 3);

    append_to(buffer, " bar {}", "baz");

    REQUIRE("foo bar baz" == to_string(buffer));
}

