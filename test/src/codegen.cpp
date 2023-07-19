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
#include "hlib/codec.hpp"

#include "codegen.hpp"

#define TEST_IMPL
#include "codegen.h"

using namespace hlib;
using namespace hlib::codec;

namespace cpp11
{

Buffer encode_Primitives(std::string const& kind)
{
    Buffer buffer;
    std::unique_ptr<Encoder> encoder = Encoder::create(kind, buffer);

    test::Primitives p;
    p.boolean = true;
    p.int32 = 12345678;
    p.int64 = 0x123456789abcdefLL;
    p.float32 = 3.14159265359f;
    p.float64 = 2.71828182845904523536;
    p.string = "foo bar";

    p(*encoder);
    return buffer;
}

void decode_Primitives(std::string const& kind, void const* data, size_t size)
{
    std::unique_ptr<Decoder> decoder = Decoder::create(kind, data, size);

    test::Primitives p;
    p(*decoder);

    REQUIRE(true == p.boolean);
    REQUIRE(12345678 == p.int32);
    REQUIRE(0x123456789abcdefLL == p.int64);
    REQUIRE(3.14159265359f == p.float32);
    REQUIRE(2.71828182845904523536 == p.float64);
    REQUIRE("foo bar" == p.string);
}

Buffer encode_PrimitiveArrays(std::string const& kind)
{
    Buffer buffer;
    std::unique_ptr<Encoder> encoder = Encoder::create(kind, buffer);

    test::PrimitiveArrays a;
    a.booleans = { true, false };
    a.int32s = { 12345678, 87654321 };
    a.int64s = { 0x123456789abcdef, 0xfedcba987654321 };
    a.float32s = { 3.141592f, 2.7182818f };
    a.float64s = { 3.14159265359, 2.71828182845904523536 };
    a.strings = { "foo", "bar" };

    a(*encoder);
    return buffer;
}

void decode_PrimitiveArrays(std::string const& kind, void const* data, size_t size)
{
    std::unique_ptr<Decoder> decoder = Decoder::create(kind, data, size);

    test::PrimitiveArrays a;
    a(*decoder);

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

} // namespace cpp11

namespace c99
{

hlib_buffer_t* encode_Primitives(char const* kind)
{
    hlib_buffer_t* buffer = hlib_buffer_create();
    REQUIRE(nullptr != buffer);

    hlib_codec_encoder_t* encoder = hlib_codec_encoder_create(kind, buffer);
    REQUIRE(nullptr != encoder);

    test_Primitives p;
    test_Primitives_init(&p);

    p.boolean = true;
    p.int32 = 12345678;
    p.int64 = 0x123456789abcdefLL;
    p.float32 = 3.14159265359f;
    p.float64 = 2.71828182845904523536;
    p.string = { "foo bar", 7 };

    test_Primitives_encode(encoder, &p);
    REQUIRE(HLIB_ERROR_NONE == encoder->error);
    encoder->destroy(encoder);

    test_Primitives_free(&p);
    return buffer;
}

void decode_Primitives(char const* kind, void const* data, size_t size)
{
    char string[8];

    hlib_codec_decoder_t* decoder = hlib_codec_decoder_create(kind, data, size);
    REQUIRE(nullptr != decoder);

    test_Primitives p;
    test_Primitives_init(&p);
    test_Primitives_decode(decoder, &p);
    REQUIRE(HLIB_ERROR_NONE == decoder->error);
    decoder->destroy(decoder);

    REQUIRE(true == p.boolean);
    REQUIRE(12345678 == p.int32);
    REQUIRE(0x123456789abcdefLL == p.int64);
    REQUIRE(3.14159265359f == p.float32);
    REQUIRE(2.71828182845904523536 == p.float64);
    REQUIRE(std::string("foo bar") == hlib_codec_string_copy(string, &p.string, sizeof(string)));

    test_Primitives_free(&p);
}

hlib_buffer_t* encode_PrimitiveArrays(char const* kind)
{
    hlib_buffer_t* buffer = hlib_buffer_create();
    REQUIRE(nullptr != buffer);

    hlib_codec_encoder_t* encoder = hlib_codec_encoder_create(kind, buffer);
    REQUIRE(nullptr != encoder);

    test_PrimitiveArrays a;
    test_PrimitiveArrays_init(&a);
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

    test_PrimitiveArrays_encode(encoder, &a);
    REQUIRE(HLIB_ERROR_NONE == encoder->error);
    encoder->destroy(encoder);

    test_PrimitiveArrays_free(&a);
    return buffer;
}

void decode_PrimitiveArrays(char const* kind, void const* data, size_t size)
{
    char string[4];

    hlib_codec_decoder_t* decoder = hlib_codec_decoder_create(kind, data, size);
    REQUIRE(nullptr != decoder);

    test_PrimitiveArrays a;
    test_PrimitiveArrays_init(&a);
    test_PrimitiveArrays_decode(decoder, &a);
    REQUIRE(HLIB_ERROR_NONE == decoder->error);
    decoder->destroy(decoder);

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
    REQUIRE(std::string("foo") == hlib_codec_string_copy(string, &HLIB_VECTOR_AT(&a.strings, hlib_codec_string_t, 0), sizeof(string)));
    REQUIRE(std::string("bar") == hlib_codec_string_copy(string, &HLIB_VECTOR_AT(&a.strings, hlib_codec_string_t, 1), sizeof(string)));

    test_PrimitiveArrays_free(&a);
}

} // namespace c99

TEST_CASE("Codegen CPP11 Binary", "[codec,codegen]")
{
    Buffer buffer;

    buffer = cpp11::encode_Primitives("binary");
             cpp11::decode_Primitives("binary", buffer.data(), buffer.size());

    buffer = cpp11::encode_PrimitiveArrays("binary");
             cpp11::decode_PrimitiveArrays("binary", buffer.data(), buffer.size());
}

TEST_CASE("Codegen C99 Binary", "[codec,codegen]")
{
    hlib_buffer_t* buffer = nullptr;

    buffer = c99::encode_Primitives("binary");
             c99::decode_Primitives("binary", buffer->data, buffer->size);
    hlib_buffer_destroy(buffer);

    buffer = c99::encode_PrimitiveArrays("binary");
             c99::decode_PrimitiveArrays("binary", buffer->data, buffer->size);
    hlib_buffer_destroy(buffer);
}

TEST_CASE("Codegen CPP11-G99 Binary", "[codec,codegen]")
{
    Buffer buffer;

    buffer = cpp11::encode_Primitives("binary");
             c99::decode_Primitives("binary", buffer.data(), buffer.size());

    buffer = cpp11::encode_PrimitiveArrays("binary");
             c99::decode_PrimitiveArrays("binary", buffer.data(), buffer.size());
}

TEST_CASE("Codegen C99-CPP11 Binary", "[codec,codegen]")
{
    hlib_buffer_t* buffer = nullptr;

    buffer = c99::encode_Primitives("binary");
             cpp11::decode_Primitives("binary", buffer->data, buffer->size);
    hlib_buffer_destroy(buffer);

    buffer = c99::encode_PrimitiveArrays("binary");
             cpp11::decode_PrimitiveArrays("binary", buffer->data, buffer->size);
    hlib_buffer_destroy(buffer);
}

TEST_CASE("Codegen CPP11 JSON", "[codec,codegen,json]")
{
    Buffer buffer;

    buffer = cpp11::encode_Primitives("json");
             cpp11::decode_Primitives("json", buffer.data(), buffer.size());

    buffer = cpp11::encode_PrimitiveArrays("json");
             cpp11::decode_PrimitiveArrays("json", buffer.data(), buffer.size());
}

