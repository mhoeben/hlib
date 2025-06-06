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
#include "hlib/buffer.hpp"

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

    std::byte* extended = static_cast<std::byte*>(buffer.extend(4));
    std::byte* data = static_cast<std::byte*>(buffer.reserve(buffer.capacity()));
    REQUIRE(8 == buffer.capacity());
    REQUIRE(4 == buffer.size());
    REQUIRE(data + buffer.size() == extended);
    REQUIRE(0 == memcmp("abgh", buffer.data(), 4));

    REQUIRE_NOTHROW(buffer.assign("foo bar baz"));
    REQUIRE("foo bar baz" == to_string(buffer.copy()));
    REQUIRE("bar" == to_string(buffer.copy(4, 3)));
    REQUIRE(" baz" == to_string(buffer.extract(7, 4)));
    REQUIRE("foo bar" == to_string(buffer));
    REQUIRE("foo" == to_string(buffer.extract(0, " bar", false)));
    REQUIRE(" bar" == to_string(buffer));

    REQUIRE_NOTHROW(buffer.assign("foo\nbar\nbaz\n"));
    REQUIRE("foo\n" == to_string(buffer.extract(0, "foo\n", true)));
    REQUIRE("bar\n" == to_string(buffer.extract(0, "bar\n", true)));
    REQUIRE("baz\n" == to_string(buffer.extract(0, "baz\n", true)));
}

TEST_CASE("Buffer Empty", "[buffer]")
{
    Buffer buffer;
    REQUIRE_NOTHROW(nullptr == buffer.reserve(0));
    REQUIRE_NOTHROW(nullptr == buffer.resize(0));
}

TEST_CASE("Buffer Zeroed", "[buffer]")
{
    auto const isZero = [](Buffer& buffer, std::size_t offset, std::size_t size)
    {
        std::uint8_t const* const data = static_cast<std::uint8_t const* const>(buffer.data()) + offset;
        REQUIRE(offset + size <= buffer.capacity());

        for (std::size_t i = 0; i < size; ++i) {
            if (0 != data[0]) {
                return false;
            }
        }

        return true;
    };

    Buffer buffer;
    void* data = buffer.reserveZeroed(8);
    REQUIRE(nullptr != data);
    REQUIRE(8 == buffer.capacity());
    REQUIRE(0 == buffer.size());
    REQUIRE(true == isZero(buffer, 0, 8));

    memset(data, 0xff, buffer.capacity());
    data = buffer.resizeZeroed(16);
    REQUIRE(nullptr != data);
    REQUIRE(16 == buffer.capacity());
    REQUIRE(16 == buffer.size());
    REQUIRE(true == isZero(buffer, 8, 8));
}

