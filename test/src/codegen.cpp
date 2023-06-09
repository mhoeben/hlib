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

#include "codegen.hpp"

#define TEST_IMPL
#include "codegen.h"

using namespace hlib;
using namespace hlib::codec;

TEST_CASE("Codegen CPP11", "[codec,codegen]")
{
    Buffer buffer;
    std::unique_ptr<Encoder> encoder;
    std::unique_ptr<Decoder> decoder;

    test::Primitives p;
    p.boolean = true;
    p.int32 = 12345678;
    p.int64 = 0x123456789abcdefLL;
    p.float32 = 3.14159265359f;
    p.float64 = 2.71828182845904523536;
    p.string = "foo bar";

    buffer.clear();
    encoder = Encoder::create("binary", buffer);
    p(*encoder);

    decoder = Decoder::create("binary", buffer.data(), buffer.size());
    p(*decoder);

    REQUIRE(true == p.boolean);
    REQUIRE(12345678 == p.int32);
    REQUIRE(0x123456789abcdefLL == p.int64);
    REQUIRE(3.14159265359f == p.float32);
    REQUIRE(2.71828182845904523536 == p.float64);
    REQUIRE("foo bar" == p.string);

    test::PrimitiveArrays a;
    a.booleans = { true, false };
    a.int32s = { 12345678, 87654321 };
    a.int64s = { 0x123456789abcdef, 0xfedcba987654321 };
    a.float32s = { 3.141592f, 2.7182818f };
    a.float64s = { 3.14159265359, 2.71828182845904523536 };
    a.strings = { "foo", "bar" };

    buffer.clear();
    encoder = Encoder::create("binary", buffer);
    p(*encoder);

    decoder = Decoder::create("binary", buffer.data(), buffer.size());
    p(*decoder);

    REQUIRE(true == a.booleans[0]);
    REQUIRE(false == a.booleans[1]);
    REQUIRE(12345678 == a.int32s[0]);
    REQUIRE(87654321 == a.int32s[1]);
    REQUIRE(0x123456789abcdef == a.int64s[0]);
    REQUIRE(0xfedcba987654321 == a.int64s[1]);
    REQUIRE(3.141592f == a.float32s[0]);
    REQUIRE(2.7182818f == a.float32s[1]);
    REQUIRE(3.14159265359 == a.float64s[0]);
    REQUIRE(2.71828182845904523536 == a.float64s[1]);
    REQUIRE("foo" == a.strings[0]);
    REQUIRE("bar" == a.strings[1]);
}

TEST_CASE("Codegen C99", "[codec,codegen]")
{
    hlib_buffer_t buffer;
    hlib_encoder_t* encoder = nullptr;
    hlib_decoder_t* decoder = nullptr;

    test_Primitives p;
    test_Primitives_init(&p);

    test_PrimitiveArrays a;
    test_PrimitiveArrays_init(&a);

    auto reset = [&]()
    {
        test_PrimitiveArrays_free(&a);
        test_Primitives_free(&p);

        if (nullptr != decoder) {
            decoder->destroy(decoder);
            decoder = nullptr;
        }
        if (nullptr != encoder) {
            encoder->destroy(encoder);
            encoder = nullptr;
        }
        hlib_buffer_free(&buffer);
    };

    auto string_compare = [](std::string const& expected, hlib_codec_string_t const& test_string)
    { 
        return expected.length() == test_string.length
            && 0 == memcmp(expected.data(), test_string.data, test_string.length);
    };

    p.boolean = true;
    p.int32 = 12345678;
    p.int64 = 0x123456789abcdefLL;
    p.float32 = 3.14159265359f;
    p.float64 = 2.71828182845904523536;
    p.string = { "foo bar", 7 };

    hlib_buffer_init(&buffer);
    encoder = hlib_encoder_create("binary", &buffer);
    REQUIRE(nullptr != encoder);
    test_Primitives_encode(encoder, &p);

    decoder = hlib_decoder_create("binary", buffer.data, buffer.size);
    REQUIRE(nullptr != decoder);
    test_Primitives_decode(decoder, &p);

    REQUIRE(HLIB_ERROR_NONE == decoder->error);

    REQUIRE(true == p.boolean);
    REQUIRE(12345678 == p.int32);
    REQUIRE(0x123456789abcdefLL == p.int64);
    REQUIRE(3.14159265359f == p.float32);
    REQUIRE(2.71828182845904523536 == p.float64);
    REQUIRE(true == string_compare("foo bar", p.string));

    reset();

    HLIB_VECTOR_PUSH_BACK(&a.booleans, char, true);
    HLIB_VECTOR_PUSH_BACK(&a.booleans, char, false);
    HLIB_VECTOR_PUSH_BACK(&a.int32s, int32_t, 12345678);
    HLIB_VECTOR_PUSH_BACK(&a.int32s, int32_t, 87654321);
    HLIB_VECTOR_PUSH_BACK(&a.int64s, int64_t, 0x123456789abcdef);
    HLIB_VECTOR_PUSH_BACK(&a.int64s, int64_t, 0xfedcba987654321);
    HLIB_VECTOR_PUSH_BACK(&a.float32s, float, 3.141592f);
    HLIB_VECTOR_PUSH_BACK(&a.float32s, float, 2.7182818f);
    HLIB_VECTOR_PUSH_BACK(&a.float64s, double, 3.14159265359);
    HLIB_VECTOR_PUSH_BACK(&a.float64s, double, 2.71828182845904523536);

    hlib_codec_string_t const foo{ "foo", 3 };
    hlib_codec_string_t const bar{ "bar", 3 };
    HLIB_VECTOR_PUSH_BACK(&a.strings, hlib_codec_string_t, foo);
    HLIB_VECTOR_PUSH_BACK(&a.strings, hlib_codec_string_t, bar);

    encoder = hlib_encoder_create("binary", &buffer);
    REQUIRE(nullptr != encoder);
    test_Primitives_encode(encoder, &p);

    decoder = hlib_decoder_create("binary", buffer.data, buffer.size);
    REQUIRE(nullptr != decoder);
    test_Primitives_decode(decoder, &p);

    REQUIRE(HLIB_ERROR_NONE == decoder->error);

    REQUIRE(true == HLIB_VECTOR_AT(&a.booleans, char, 0));
    REQUIRE(false== HLIB_VECTOR_AT(&a.booleans, char, 1));
    REQUIRE(12345678 == HLIB_VECTOR_AT(&a.int32s, int32_t, 0));
    REQUIRE(87654321 == HLIB_VECTOR_AT(&a.int32s, int32_t, 1));
    REQUIRE(0x123456789abcdef == HLIB_VECTOR_AT(&a.int64s, int64_t, 0));
    REQUIRE(0xfedcba987654321 == HLIB_VECTOR_AT(&a.int64s, int64_t, 1));
    REQUIRE(3.141592f == HLIB_VECTOR_AT(&a.float32s, float, 0));
    REQUIRE(2.7182818f == HLIB_VECTOR_AT(&a.float32s, float, 1));
    REQUIRE(3.14159265359 == HLIB_VECTOR_AT(&a.float64s, double, 0));
    REQUIRE(2.71828182845904523536 == HLIB_VECTOR_AT(&a.float64s, double, 1));
    REQUIRE(true == string_compare("foo", HLIB_VECTOR_AT(&a.strings, hlib_codec_string_t, 0)));
    REQUIRE(true == string_compare("bar", HLIB_VECTOR_AT(&a.strings, hlib_codec_string_t, 1)));

    reset();
}

