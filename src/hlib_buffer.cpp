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
#include "hlib/buffer.hpp"
#include <string.h>

using namespace hlib;

//
// Implementation
//
void Buffer::realloc(size_t capacity, bool shrink)
{
    assert(0 == m_capacity || nullptr != m_data);
    assert(m_size <= m_capacity);

    if (false == shrink && capacity <= m_capacity) {
        return;
    }

    void* data = ::realloc(m_data, capacity);
    if (nullptr == data) {
        throw std::bad_alloc();
    }

    m_data = data;
    m_capacity = capacity;
    m_size = std::min(m_size, m_capacity);
}

//
// Public
//
Buffer::Buffer(Buffer&& that) noexcept
    : m_capacity{ that.m_capacity }
    , m_size{ that.m_size }
    , m_data{ that.m_data }
{
    that.m_capacity = 0;
    that.m_size = 0;
    that.m_data = nullptr;
}

Buffer::~Buffer()
{
    reset();
}

Buffer& Buffer::operator =(Buffer&& that) noexcept
{
    reset();

    m_capacity = that.m_capacity;
    m_size = that.m_size;
    m_data = that.m_data;

    that.m_capacity = 0;
    that.m_size = 0;
    that.m_data = nullptr;
    return *this;
}

void const* Buffer::data() const noexcept
{
    return m_data;
}

size_t Buffer::capacity() const noexcept
{
    return m_capacity;
}

size_t Buffer::size() const noexcept
{
    return m_size;
}

bool Buffer::empty() const noexcept
{
    return 0 == m_size;
}

void Buffer::reset() noexcept
{
    if (nullptr != m_data) {
        free(m_data);
    }

    m_size = 0;
    m_capacity = 0;
}

void Buffer::clear() noexcept
{
    m_size = 0;
}

void Buffer::shrink() noexcept
{
    if (0 == m_size) {
        reset();
        return;
    }

    realloc(m_size, true);
}

void* Buffer::reserve(size_t capacity)
{
    realloc(capacity, false);
    return m_data;
}

void* Buffer::resize(size_t size)
{
    if (size > 0) {
        realloc(size, false);
    }

    m_size = size;
    return m_data;
}

void Buffer::assign(void const* data, size_t size)
{
    clear();
    append(data, size);
}

void Buffer::append(void const* data, size_t size)
{
    insert(m_size, data, size);
}

void Buffer::insert(size_t offset, void const* data, size_t size)
{
    assert(offset <= m_size);

    if (0 == size) {
        return;
    }

    realloc(m_size + size, false);

    uint8_t* ptr = reinterpret_cast<uint8_t*>(m_data);

    memmove(ptr + offset + size, ptr + offset, m_size - offset);
    if (nullptr != data) {
        memcpy(ptr + offset, data, size);
    }
    m_size += size;
}

void Buffer::erase(size_t offset, size_t size)
{
    assert(offset + size <= m_size);

    uint8_t* ptr = reinterpret_cast<uint8_t*>(m_data);

    memmove(ptr + offset, ptr + offset + size, m_size - (offset + size));
    m_size -= size;
}

