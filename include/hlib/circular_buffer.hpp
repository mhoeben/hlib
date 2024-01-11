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
#pragma once

#include "hlib/base.hpp"
#include "hlib/c/circular_buffer.h"
#include "hlib/result.hpp"
#include <new>

namespace hlib
{

class Buffer;

class CircularBuffer final
{
    HLIB_NOT_COPYABLE(CircularBuffer);

public:
    CircularBuffer(std::size_t capacity, std::nothrow_t) noexcept;
    CircularBuffer(std::size_t capacity);
    CircularBuffer(CircularBuffer&& that) noexcept;
    ~CircularBuffer();

    CircularBuffer& operator =(CircularBuffer& that) noexcept;

    hlib_circular_buffer_t const* buffer() const noexcept;
    hlib_circular_buffer_t* buffer() noexcept;

    std::size_t capacity() const noexcept;
    std::size_t size() const noexcept;
    bool empty() const noexcept;

    Result<std::size_t> produce(void const* data, std::size_t size, bool wait, std::nothrow_t) noexcept;
           std::size_t  produce(void const* data, std::size_t size, bool wait);
    Result<std::size_t> produce(Buffer const& buffer, bool wait, std::nothrow_t) noexcept;
           std::size_t  produce(Buffer const& buffer, bool wait);

    Result<std::size_t> consume(void* data, std::size_t size, bool wait, std::nothrow_t) noexcept;
           std::size_t  consume(void* data, std::size_t size, bool wait);
    Result<std::size_t> consume(Buffer& buffer, std::size_t size, bool wait, std::nothrow_t) noexcept;
           std::size_t  consume(Buffer& buffer, std::size_t size, bool wait);

private:
    hlib_circular_buffer_t* m_buffer{ nullptr };
};

} // namespace hlib

