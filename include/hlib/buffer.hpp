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

#include "hlib/base.hpp"
#include "hlib/c/buffer.h"
#include <string>

namespace hlib
{

class Buffer final
{
    HLIB_NOT_COPYABLE(Buffer);

public:
    Buffer() = default;
    Buffer(Buffer&& that) noexcept;
    ~Buffer();

    Buffer& operator =(Buffer&& that) noexcept;

    hlib_buffer_t const* buffer() const noexcept;
    hlib_buffer_t* buffer() noexcept;

    void const* data() const noexcept;
    std::size_t capacity() const noexcept;
    std::size_t size() const noexcept;
    bool empty() const noexcept;

    void reset() noexcept;
    void clear() noexcept;
    void shrink() noexcept;

    void* reserve(std::size_t capacity);
    void* resize(std::size_t size);

    void assign(void const* data, std::size_t size);
    void append(void const* data, std::size_t size);
    void insert(std::size_t offset, void const* data, std::size_t size);
    void erase(std::size_t offset, std::size_t size);

private:
    hlib_buffer_t m_buffer{};
};

template <typename T>
class BufferAllocator
{
public:
    using value_type = T;

    // Constructor
    BufferAllocator(Buffer& buffer) noexcept
        : m_buffer(buffer)
        , m_size(buffer.size())
    {
    }

    T* allocate(std::size_t size)
    {
        if (size > static_cast<std::size_t>(-1) / sizeof(T)) {
            throw std::bad_alloc();
        }

        std::uint8_t* ptr = static_cast<std::uint8_t*>(m_buffer.resize(m_size + size * sizeof(T)));
        return reinterpret_cast<T*>(ptr + m_size);
    }

    void deallocate(T* ptr, std::size_t size) noexcept
    {
        assert(static_cast<std::uint8_t const*>(m_buffer.data()) + m_size == reinterpret_cast<std::uint8_t*>(ptr));
        assert(m_buffer.size() == m_size + size);
        m_buffer.resize(m_size);
    }

private:
    Buffer& m_buffer;
    std::size_t m_size;
};

template <typename T, typename U>
bool operator ==(BufferAllocator<T> const& a, BufferAllocator<U> const& b)
{
    return a->data() == b->data();
}

template <typename T, typename U>
bool operator !=(BufferAllocator<T> const& a, BufferAllocator<U> const& b)
{
    return a->data() != b->data();
}

std::string to_string(Buffer const& buffer);

} // namespace hlib

