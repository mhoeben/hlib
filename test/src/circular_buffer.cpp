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
#include "hlib/circular_buffer.hpp"

using namespace hlib;

TEST_CASE("CircularBuffer", "[buffer]")
{
    CircularBuffer buffer(8);
    char data[8];

    REQUIRE(8 == buffer.capacity());
    REQUIRE(0 == buffer.size());
    REQUIRE(true == buffer.empty());

    REQUIRE(4 == buffer.produce("abcd", 4, false));
    REQUIRE(4 == buffer.size());
    REQUIRE(false == buffer.empty());

    REQUIRE(4 == buffer.produce("efghijkl", 8, false));
    REQUIRE(8 == buffer.size());
    REQUIRE(false == buffer.empty());

    REQUIRE(4 == buffer.consume(data, 4, false));
    REQUIRE(4 == buffer.size());
    REQUIRE(false == buffer.empty());

    REQUIRE(4 == buffer.produce("ijkl", 4, false));
    REQUIRE(8 == buffer.size());
    REQUIRE(false == buffer.empty());

    REQUIRE(8 == buffer.consume(data, 8, false));
    REQUIRE(0 == buffer.size());
    REQUIRE(true == buffer.empty());
    REQUIRE(0 == memcmp("efghijkl", data, 8));

    REQUIRE(4 == buffer.produce("abcd", 4, false));
    REQUIRE(4 == buffer.consume(data, 4, false));
    REQUIRE(0 == memcmp("abcd", data, 4));
    REQUIRE(8 == buffer.produce("efghijkl", 8, false));
    REQUIRE(2 == buffer.consume(data, 2, false));
    REQUIRE(0 == memcmp("ef", data, 2));
    REQUIRE(4 == buffer.consume(data, 4, false));
    REQUIRE(0 == memcmp("ghij", data, 4));
    REQUIRE(2 == buffer.consume(data, 2, false));
    REQUIRE(0 == memcmp("kl", data, 2));
}

