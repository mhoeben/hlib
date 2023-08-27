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
#include "hlib/endian.hpp"

using namespace hlib;

TEST_CASE("Big Endian", "[endian]")
{
    Buffer buffer;

    be::BufferSerializer serializer(buffer);
    serializer.transform<std::int8_t>(-13)
              .transform<std::int16_t>(-11)
              .transform<std::int32_t>(-1971)
              .transform<std::int64_t>(-13111971)
              .transform<std::uint8_t>(13)
              .transform<std::uint16_t>(11)
              .transform<std::uint32_t>(1971)
              .transform<std::uint64_t>(13111971)
              .transform<float>(-3.14159f)
              .transform<double>(3.14159);

    std::int8_t i8;
    std::int16_t i16;
    std::int32_t i32;
    std::int64_t i64;
    std::uint8_t u8;
    std::uint16_t u16;
    std::uint32_t u32;
    std::uint64_t u64;
    float f;
    double d;
    be::BufferDeserializer deserializer(buffer);
    deserializer.transform<std::int8_t>(i8)
                .transform<std::int16_t>(i16)
                .transform<std::int32_t>(i32)
                .transform<std::int64_t>(i64)
                .transform<std::uint8_t>(u8)
                .transform<std::uint16_t>(u16)
                .transform<std::uint32_t>(u32)
                .transform<std::uint64_t>(u64)
                .transform<float>(f)
                .transform<double>(d);

    REQUIRE(-13 == i8);
    REQUIRE(-11 == i16);
    REQUIRE(-1971 == i32);
    REQUIRE(-13111971 == i64);
    REQUIRE( 13 == u8);
    REQUIRE( 11 == u16);
    REQUIRE( 1971 == u32);
    REQUIRE( 13111971 == u64);
    REQUIRE(-3.14159f == f);
    REQUIRE( 3.14159 == d);
}

TEST_CASE("Little Endian", "[endian]")
{
    Buffer buffer;

    le::BufferSerializer serializer(buffer);
    serializer.transform<std::int8_t>(-13)
              .transform<std::int16_t>(-11)
              .transform<std::int32_t>(-1971)
              .transform<std::int64_t>(-13111971)
              .transform<std::uint8_t>(13)
              .transform<std::uint16_t>(11)
              .transform<std::uint32_t>(1971)
              .transform<std::uint64_t>(13111971)
              .transform<float>(-3.14159f)
              .transform<double>(3.14159);

    std::int8_t i8;
    std::int16_t i16;
    std::int32_t i32;
    std::int64_t i64;
    std::uint8_t u8;
    std::uint16_t u16;
    std::uint32_t u32;
    std::uint64_t u64;
    float f;
    double d;
    le::BufferDeserializer deserializer(buffer);
    deserializer.transform<std::int8_t>(i8)
                .transform<std::int16_t>(i16)
                .transform<std::int32_t>(i32)
                .transform<std::int64_t>(i64)
                .transform<std::uint8_t>(u8)
                .transform<std::uint16_t>(u16)
                .transform<std::uint32_t>(u32)
                .transform<std::uint64_t>(u64)
                .transform<float>(f)
                .transform<double>(d);

    REQUIRE(-13 == i8);
    REQUIRE(-11 == i16);
    REQUIRE(-1971 == i32);
    REQUIRE(-13111971 == i64);
    REQUIRE( 13 == u8);
    REQUIRE( 11 == u16);
    REQUIRE( 1971 == u32);
    REQUIRE( 13111971 == u64);
    REQUIRE(-3.14159f == f);
    REQUIRE( 3.14159 == d);
}

