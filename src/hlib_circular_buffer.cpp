//
// MIT License
//
// Copyright (c) 2024 Maarten Hoeben
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
#include "hlib/circular_buffer.hpp"
#include "hlib/c/circular_buffer.h"
#include "hlib/buffer.hpp"
#include "hlib/error.hpp"
#include <system_error>

using namespace hlib;

//
// Public
//
CircularBuffer::CircularBuffer(std::size_t capacity, std::nothrow_t) noexcept
    : m_buffer(hlib_circular_buffer_create(capacity))
{
}

CircularBuffer::CircularBuffer(std::size_t capacity)
    : CircularBuffer(capacity, std::nothrow)
{
    if (nullptr == m_buffer) {
        auto error_code = std::make_error_code(static_cast<std::errc>(errno));
        throw std::system_error(error_code, "hlib_circular_buffer_creat() failed");
    }
}

CircularBuffer::CircularBuffer(CircularBuffer&& that) noexcept
    : m_buffer(that.m_buffer)
{
    that.m_buffer = nullptr;
}

CircularBuffer::~CircularBuffer()
{
    hlib_circular_buffer_destroy(m_buffer);
}

CircularBuffer& CircularBuffer::operator =(CircularBuffer& that) noexcept
{
    hlib_circular_buffer_destroy(m_buffer);

    m_buffer = that.m_buffer;
    that.m_buffer = nullptr;
    return *this;
}

hlib_circular_buffer_t const* CircularBuffer::buffer() const noexcept
{
    return m_buffer;
}

hlib_circular_buffer_t* CircularBuffer::buffer() noexcept
{
    return m_buffer;
}

std::size_t CircularBuffer::capacity() const noexcept
{
    return nullptr != m_buffer ? m_buffer->capacity : 0;
}

std::size_t CircularBuffer::size() const noexcept
{
    return nullptr != m_buffer ? m_buffer->size : 0;
}

bool CircularBuffer::empty() const noexcept
{
    return 0 == size();
}

Result<std::size_t> CircularBuffer::produce(void const* data, std::size_t size, bool wait, std::nothrow_t) noexcept
{
    ssize_t produced = hlib_circular_buffer_produce(m_buffer, data, size, wait);
    if (-1 == produced) {
        return make_system_error(errno);
    }

    return produced;
}

std::size_t CircularBuffer::produce(void const* data, std::size_t size, bool wait)
{
    Result<std::size_t> result = produce(data, size, wait, std::nothrow);
    if (true == result.failure()) {
        throw result.error();
    }

    return result.value();
}

Result<std::size_t> CircularBuffer::produce(Buffer const& buffer, bool wait, std::nothrow_t) noexcept
{
    return produce(buffer.data(), buffer.size(), wait, std::nothrow);
}

std::size_t  CircularBuffer::produce(Buffer const& buffer, bool wait)
{
    return produce(buffer.data(), buffer.size(), wait);
}

Result<std::size_t> CircularBuffer::consume(void* data, std::size_t size, bool wait, std::nothrow_t) noexcept
{
    ssize_t consumed = hlib_circular_buffer_consume(m_buffer, data, size, wait);
    if (-1 == consumed) {
        return make_system_error(errno);
    }

    return consumed;
}

std::size_t CircularBuffer::consume(void* data, std::size_t size, bool wait)
{
    Result<std::size_t> result = consume(data, size, wait, std::nothrow);
    if (true == result.failure()) {
        throw result.error();
    }

    return result.value();
}

Result<std::size_t> CircularBuffer::consume(Buffer& buffer, std::size_t size, bool wait, std::nothrow_t) noexcept
{
    void* data = buffer.extend(size, std::nothrow);
    if (nullptr == data) {
        return make_system_error(errno);
    }

    Result<std::size_t> result = consume(data, size, wait, std::nothrow);
    if (true == result.success()) {
        buffer.resize(buffer.size() + size);
    }

    return result;
}

std::size_t CircularBuffer::consume(Buffer& buffer, std::size_t size, bool wait)
{
    size = consume(buffer.extend(size), size, wait);

    buffer.resize(buffer.size() + size);
    return size;
}

