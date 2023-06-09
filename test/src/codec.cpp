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
#include "hlib/codec.hpp"

using namespace hlib;
using namespace hlib::codec;

namespace
{

void test(std::string kind)
{
    Buffer buffer;
    std::unique_ptr<Encoder> encoder = Encoder::create(kind, buffer);

    encoder->open("", codec::Map(6));
    encoder->encode("foo", std::string("bar"));
    encoder->encode("xxx", true);
    encoder->encode("yyy", 13);
    encoder->encode("zzz", 11.0);
    encoder->open("xyz", codec::Array(4));
    encoder->encode("", 1);
    encoder->encode("", 9);
    encoder->encode("", 7);
    encoder->encode("", 1);
    encoder->close();
    encoder->open("empty", codec::Array(0));
    encoder->close();
    encoder->close();

    std::unique_ptr<Decoder> decoder = Decoder::create(kind, buffer.data(), buffer.size());

    codec::Map root;
    std::string foo;
    bool xxx;
    int32_t yyy;
    double zzz;
    codec::Array xyz;
    int32_t xyz0;
    int32_t xyz1;
    int32_t xyz2;
    int32_t xyz3;
    codec::Array empty;

    decoder->open("", root);
    decoder->decode("foo", foo);
    decoder->decode("xxx", xxx);
    decoder->decode("yyy", yyy);
    decoder->decode("zzz", zzz);
    decoder->open("xyz", xyz);
    decoder->decode("", xyz0);
    decoder->decode("", xyz1);
    decoder->decode("", xyz2);
    decoder->decode("", xyz3);
    decoder->close();
    decoder->open("empty", empty);
    decoder->close();
    decoder->close();

    REQUIRE(6 == root.size);
    REQUIRE("bar" == foo);
    REQUIRE(true == xxx);
    REQUIRE(13 == yyy);
    REQUIRE(11.0 == zzz);
    REQUIRE(4 == xyz.size);
    REQUIRE(1 == xyz0);
    REQUIRE(9 == xyz1);
    REQUIRE(7 == xyz2);
    REQUIRE(1 == xyz3);
    REQUIRE(0 == empty.size);
}


} // namespace

TEST_CASE("Binary Codec", "[codec]")
{
    test("binary");
}

TEST_CASE("JSON Codec", "[codec,json]")
{
    test("json");
}

