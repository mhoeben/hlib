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
#include "hlib_codec_binary.hpp"
#include "hlib/buffer.hpp"
#include "hlib/format.hpp"
#include "hlib/c/codec_binary.h"

using namespace hlib;
using namespace hlib::codec;

namespace
{

void check_error(char const* operation, hlib_error_t error)
{
    switch (error) {
    case HLIB_ERROR_NONE:
        break;

    case HLIB_ERROR_BAD_ALLOC:
        throw std::bad_alloc();

    default:
        throwf<std::runtime_error>("Error while {} ({})", operation, hlib_error_to_string(error)); 
    }
}

} // namespace

//
// Public (BinaryEncoder)
//
BinaryEncoder::BinaryEncoder(Buffer& buffer)
    : m_encoder(hlib_codec_encoder_create("binary", buffer.buffer()))
{
}

BinaryEncoder::~BinaryEncoder()
{
    hlib_codec_encoder_binary_destroy(m_encoder);
}

bool BinaryEncoder::isBinary() const
{
    return true;
}

void BinaryEncoder::open(char const* name, Type const& value)
{
    hlib_codec_type_t const type{ static_cast<Type::Id>(value), static_cast<std::size_t>(value) };
    hlib_codec_encoder_binary_open_type(m_encoder, name, &type);
    check_error("encoding", m_encoder->error);
}

void BinaryEncoder::open(char const* name, Array const& value)
{
    hlib_codec_encoder_binary_open_array(m_encoder, name, value.size);
    check_error("encoding", m_encoder->error);
}

void BinaryEncoder::open(char const* name, Map const& value)
{
    hlib_codec_encoder_binary_open_map(m_encoder, name, value.size);
    check_error("encoding", m_encoder->error);
}

void BinaryEncoder::encode(char const* name, bool const& value)
{
    hlib_codec_encoder_binary_encode_bool(m_encoder, name, value);
    check_error("encoding", m_encoder->error);
}

void BinaryEncoder::encode(char const* name, std::int32_t const& value)
{
    hlib_codec_encoder_binary_encode_int32(m_encoder, name, value);
    check_error("encoding", m_encoder->error);
}

void BinaryEncoder::encode(char const* name, std::int64_t const& value)
{
    hlib_codec_encoder_binary_encode_int64(m_encoder, name, value);
    check_error("encoding", m_encoder->error);
}

void BinaryEncoder::encode(char const* name, float const& value)
{
    hlib_codec_encoder_binary_encode_float(m_encoder, name, value);
    check_error("encoding", m_encoder->error);
}

void BinaryEncoder::encode(char const* name, double const& value)
{
    hlib_codec_encoder_binary_encode_double(m_encoder, name, value);
    check_error("encoding", m_encoder->error);
}

void BinaryEncoder::encode(char const* name, std::string const& value)
{
    hlib_codec_string_t string{ value.data(), value.length() };
    hlib_codec_encoder_binary_encode_string(m_encoder, name, &string);
    check_error("encoding", m_encoder->error);
}

void BinaryEncoder::encode(char const* name, Binary const& value)
{
    hlib_codec_binary_t binary{ value.data, value.size };
    hlib_codec_encoder_binary_encode_binary(m_encoder, name, &binary);
    check_error("encoding", m_encoder->error);
}

void BinaryEncoder::close()
{
    hlib_codec_encoder_binary_close(m_encoder);
    check_error("encoding", m_encoder->error);
}

//
// Public (BinaryDecoder)
//
BinaryDecoder::BinaryDecoder(void const* data, std::size_t size)
    : m_decoder(hlib_codec_decoder_create("binary", data, size))
{
}

BinaryDecoder::~BinaryDecoder()
{
    hlib_codec_decoder_binary_destroy(m_decoder);
}

void BinaryDecoder::reset(void const* data, std::size_t size)
{
    hlib_codec_decoder_binary_reset(m_decoder, data, size);
}

void BinaryDecoder::open(char const* name, Type& value)
{
    hlib_codec_type_t type{ static_cast<Type::Id>(value), 0 };
    hlib_codec_decoder_binary_open_type(m_decoder, name, &type);
    check_error("decoding", m_decoder->error);

    assert(static_cast<Type::Id>(value) == type.id_);
}

void BinaryDecoder::open(char const* name, Array& value)
{
    hlib_codec_decoder_binary_open_array(m_decoder, name, &value.size);
    check_error("decoding", m_decoder->error);
}

void BinaryDecoder::open(char const* name, Map& value)
{
    hlib_codec_decoder_binary_open_map(m_decoder, name, &value.size);
    check_error("decoding", m_decoder->error);
}

void BinaryDecoder::decode(char const* name, bool& value)
{
    hlib_codec_decoder_binary_decode_bool(m_decoder, name, &value);
    check_error("decoding", m_decoder->error);
}

void BinaryDecoder::decode(char const* name, std::int32_t& value)
{
    hlib_codec_decoder_binary_decode_int32(m_decoder, name, &value);
    check_error("decoding", m_decoder->error);
}

void BinaryDecoder::decode(char const* name, std::int64_t& value)
{
    hlib_codec_decoder_binary_decode_int64(m_decoder, name, &value);
    check_error("decoding", m_decoder->error);
}

void BinaryDecoder::decode(char const* name, float& value)
{
    hlib_codec_decoder_binary_decode_float(m_decoder, name, &value);
    check_error("decoding", m_decoder->error);
}

void BinaryDecoder::decode(char const* name, double& value)
{
    hlib_codec_decoder_binary_decode_double(m_decoder, name, &value);
    check_error("decoding", m_decoder->error);
}

void BinaryDecoder::decode(char const* name, std::string& value)
{
    hlib_codec_string_t string;
    hlib_codec_decoder_binary_decode_string(m_decoder, name, &string);
    value.assign(string.data, string.length);
}

void BinaryDecoder::decode(char const* name, Binary& value)
{
    hlib_codec_binary_t binary;
    hlib_codec_decoder_binary_decode_binary(m_decoder, name, &binary);
    value.data = binary.data;
    value.size = binary.size;
}

void BinaryDecoder::close()
{
    hlib_codec_decoder_binary_close(m_decoder);
}

bool BinaryDecoder::more() const
{
    return !!hlib_codec_decoder_binary_more(m_decoder);
}

Type::Id BinaryDecoder::peek() const
{
    Type::Id id = hlib_codec_decoder_binary_peek(m_decoder);
    if (INT_MIN == id) {
        throwf<std::runtime_error>("Data is not a wrapped codec::Type");
    }
    return id;
}

