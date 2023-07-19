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
#include "hlib/c/buffer.h"
#include <cstring>

using namespace hlib;

//
// Public
//
Buffer::Buffer(std::size_t reservation)
{
    reserve(reservation);
}

Buffer::Buffer(void const* data, size_t size)
{
    assign(data, size);
}

Buffer::Buffer(std::string const& string)
{
    assign(string.data(), string.size());
}

Buffer::Buffer(Buffer&& that) noexcept
{
    hlib_buffer_move(&m_buffer, &that.m_buffer);
}

Buffer::~Buffer()
{
    reset();
}

Buffer& Buffer::operator =(Buffer&& that) noexcept
{
    hlib_buffer_move(&m_buffer, &that.m_buffer);
    return *this;
}

hlib_buffer_t const* Buffer::buffer() const noexcept
{
    return &m_buffer;
}

hlib_buffer_t* Buffer::buffer() noexcept
{
    return &m_buffer;
}

void const* Buffer::data() const noexcept
{
    return m_buffer.data;
}

std::size_t Buffer::capacity() const noexcept
{
    return m_buffer.capacity;
}

std::size_t Buffer::size() const noexcept
{
    return m_buffer.size;
}

bool Buffer::empty() const noexcept
{
    return 0 == m_buffer.size;
}

void const* Buffer::operator[](std::size_t index) noexcept
{
    assert(index < m_buffer.size);
    return static_cast<uint8_t const*>(m_buffer.data) + index;
}

void const* Buffer::at(std::size_t index)
{
    if (index >= m_buffer.size) {
        throw std::out_of_range("Buffer::at index argument exceeds buffer size");
    }

    return (*this)[index];
}

void Buffer::reset() noexcept
{
    hlib_buffer_free(&m_buffer);
}

void Buffer::clear() noexcept
{
    m_buffer.size = 0;
}

void Buffer::shrink() noexcept
{
    hlib_buffer_shrink(&m_buffer);
}

void* Buffer::reserve(std::size_t capacity, std::nothrow_t) noexcept
{
    return hlib_buffer_reserve(&m_buffer, capacity);
}

void* Buffer::reserve(std::size_t capacity)
{
    void* data = hlib_buffer_reserve(&m_buffer, capacity);
    if (nullptr == data) {
        throw std::bad_alloc();
    }
    return data;
}

void* Buffer::resize(std::size_t size, std::nothrow_t) noexcept
{
    return hlib_buffer_resize(&m_buffer, size);
}

void* Buffer::resize(std::size_t size)
{
    void* data = hlib_buffer_resize(&m_buffer, size);
    if (nullptr == data) {
        throw std::bad_alloc();
    }
    return data;
}

bool Buffer::assign(void const* data, std::size_t size, std::nothrow_t) noexcept
{
    return -1 != hlib_buffer_assign(&m_buffer, data, size);
}

void Buffer::assign(void const* data, std::size_t size)
{
    if (-1 == hlib_buffer_assign(&m_buffer, data, size)) {
        throw std::bad_alloc();
    }
}

bool Buffer::append(void const* data, std::size_t size, std::nothrow_t) noexcept
{
    return -1 != hlib_buffer_append(&m_buffer, data, size);
}

void Buffer::append(void const* data, std::size_t size)
{
    if (-1 == hlib_buffer_append(&m_buffer, data, size)) {
        throw std::bad_alloc();
    }
}

bool Buffer::insert(std::size_t offset, void const* data, std::size_t size, std::nothrow_t) noexcept
{
    return -1 != hlib_buffer_insert(&m_buffer, offset, data, size);
}

void Buffer::insert(std::size_t offset, void const* data, std::size_t size)
{
    if (-1 == hlib_buffer_insert(&m_buffer, offset, data, size)) {
        throw std::bad_alloc();
    }
}

void Buffer::erase(std::size_t offset, std::size_t size) noexcept
{
    hlib_buffer_erase(&m_buffer, offset, size);
}

//
// Utility
//
std::string hlib::to_string(Buffer const& buffer)
{
    return std::string(static_cast<char const*>(buffer.data()), buffer.size());
}

