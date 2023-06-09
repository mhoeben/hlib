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
#include "hlib/codec.hpp"
#include "hlib/buffer.hpp"
#include "hlib/memory.hpp"
#include "hlib_codec_binary.hpp"
#include "hlib_codec_json.hpp"
#include "hlib/c/codec.h"

using namespace hlib;
using namespace hlib::codec;

//
// Public
//
std::unique_ptr<Encoder> Encoder::create(std::string const& kind, Buffer& buffer)
{
    std::unique_ptr<Encoder> encoder;

    if ("binary" == kind) {
        encoder.reset(new BinaryEncoder(buffer));
    }
    else if ("json" == kind) {
        encoder.reset(new JSONEncoder(buffer));
    }

    return encoder;
}

void Encoder::wrap(Type const& type)
{
    open(nullptr, Array(2));
    encode(nullptr, static_cast<Type::Id>(type));
    type(*this);
    close();
}

std::unique_ptr<Decoder> Decoder::create(std::string const& kind, void const* data, std::size_t size)
{
    std::unique_ptr<Decoder> decoder;

    if ("binary" == kind) {
        decoder.reset(new BinaryDecoder(data, size));
    }
    else if ("json" == kind) {
        decoder.reset(new JSONDecoder(data, size));
    }

    return decoder;
}

void Decoder::unwrap(Type& type)
{
    Array array;
    Type::Id id;

    open(nullptr, array);
    assert(2 == array.size);
    decode(nullptr, id);
    type(*this);
    assert(static_cast<Type::Id>(type) == id);
    close();
}
