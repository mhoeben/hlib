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
#pragma once

#include "hlib/codec.hpp"

struct hlib_encoder_s;
struct hlib_decoder_s;

namespace hlib
{
namespace codec
{

class BinaryEncoder final : public Encoder
{
public:
    BinaryEncoder(Buffer& buffer);
    ~BinaryEncoder();

    void open(char const* name, Type const& value) override;
    void open(char const* name, Array const& value) override;
    void open(char const* name, Map const& value) override;
    void encode(char const* name, bool const& value) override;
    void encode(char const* name, std::int32_t const& value) override;
    void encode(char const* name, std::int64_t const& value) override;
    void encode(char const* name, float const& value) override;
    void encode(char const* name, double const& value) override;
    void encode(char const* name, std::string const& value) override;
    void encode(char const* name, Blob const& value) override;
    void close() override;

private:
    struct hlib_encoder_s* m_encoder;
};

class BinaryDecoder : public Decoder
{
public:
    BinaryDecoder(void const* data, std::size_t size);
    ~BinaryDecoder();

    void open(char const* name, Type& value) override;
    void open(char const* name, Array& value) override;
    void open(char const* name, Map& value) override;
    void decode(char const* name, bool& value) override;
    void decode(char const* name, std::int32_t& value) override;
    void decode(char const* name, std::int64_t& value) override;
    void decode(char const* name, float& value) override;
    void decode(char const* name, double& value) override;
    void decode(char const* name, std::string& value) override;
    void decode(char const* name, Blob& value) override;
    void close() override;

    int peek() const override;

private:
    struct hlib_decoder_s* m_decoder;
};

} // namespace codec
} // namespace hlib

