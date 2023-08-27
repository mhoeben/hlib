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
#include "hlib/endian.hpp"

using namespace hlib;

//
// Public (be::Serializer)
//
be::Serializer::Serializer(void* data, [[maybe_unused]] std::size_t capacity)
    : m_data{ data }
#ifndef NDEBUG
    , m_capacity{ capacity }
#endif
{
}

void* be::Serializer::data() const noexcept
{
    return m_data;
}

//
// Public (be::BufferSerializer)
//
be::BufferSerializer::BufferSerializer(Buffer& buffer)
    : m_buffer(buffer)
{
}

//
// Public (be::Deserializer)
//
be::Deserializer::Deserializer(void* data, [[maybe_unused]] std::size_t size)
    : m_data{ data }
#ifndef NDEBUG
    , m_size{ size}
#endif
{
}

//
// Public (be::BufferDeserializer)
//
be::BufferDeserializer::BufferDeserializer(Buffer const& buffer, std::size_t offset)
    : m_buffer(buffer)
    , m_offset{ offset }
{
}

//
// Public (le::Serializer)
//
le::Serializer::Serializer(void* data, [[maybe_unused]] std::size_t capacity)
    : m_data{ data }
#ifndef NDEBUG
    , m_capacity{ capacity }
#endif
{
}

void* le::Serializer::data() const noexcept
{
    return m_data;
}

//
// Public (le::BufferSerializer)
//
le::BufferSerializer::BufferSerializer(Buffer& buffer)
    : m_buffer(buffer)
{
}

//
// Public (le::Deserializer)
//
le::Deserializer::Deserializer(void* data, [[maybe_unused]] std::size_t size)
    : m_data{ data }
#ifndef NDEBUG
    , m_size{ size}
#endif
{
}

//
// Public (be::BufferDeserializer)
//
le::BufferDeserializer::BufferDeserializer(Buffer const& buffer, std::size_t offset)
    : m_buffer(buffer)
    , m_offset{ offset }
{
}
