//
// MIT License
//
// Copyright (c) 2019 Maarten Hoeben
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
#include "hlib/source.hpp"
#include "hlib/sink.hpp"
#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace hlib
{

class Buffer final
{
    HLIB_NOT_COPYABLE(Buffer);

public:
    static constexpr std::size_t InfiniteCapacity = std::numeric_limits<std::size_t>::max();

public:
    Buffer() = default;
    explicit Buffer(std::size_t reservation, std::size_t maximum);
    explicit Buffer(std::size_t reservation);
    Buffer(void const* data, size_t size);
    Buffer(std::string_view const& string);
    Buffer(char const* string);
    Buffer(Buffer&& that) noexcept;
    ~Buffer();

    Buffer& operator =(Buffer&& that) noexcept;

    void const* data() const noexcept;
    void* data() noexcept;

    std::size_t capacity() const noexcept;
    std::size_t size() const noexcept;
    bool empty() const noexcept;

    void const* get(std::size_t index, std::nothrow_t) const noexcept;
    void const* get(std::size_t index) const;

    std::byte operator[](std::size_t index) const noexcept;
    std::byte at(std::size_t index, std::nothrow_t) const noexcept;
    std::byte at(std::size_t index) const;

    void reset() noexcept;
    void clear() noexcept;
    void shrink() noexcept;

    void* reserve(std::size_t capacity, std::nothrow_t) noexcept;
    void* reserve(std::size_t capacity);
    void* reserveZeroed(std::size_t capacity, std::nothrow_t) noexcept;
    void* reserveZeroed(std::size_t capacity);

    void* extend(std::size_t capacity, std::nothrow_t) noexcept;
    void* extend(std::size_t capacity);
    void* extendZeroed(std::size_t capacity, std::nothrow_t) noexcept;
    void* extendZeroed(std::size_t capacity);

    void* resize(std::size_t size, std::nothrow_t) noexcept;
    void* resize(std::size_t size);
    void* resizeZeroed(std::size_t size, std::nothrow_t) noexcept;
    void* resizeZeroed(std::size_t size);

    bool assign(void const* data, std::size_t size, std::nothrow_t) noexcept;
    void assign(void const* data, std::size_t size);
    bool assign(std::string_view const& string, std::nothrow_t) noexcept;
    void assign(std::string_view const& string);

    bool append(void const* data, std::size_t size, std::nothrow_t) noexcept;
    void append(void const* data, std::size_t size);
    bool append(std::string_view const& string, std::nothrow_t) noexcept;
    void append(std::string_view const& string);

    bool insert(std::size_t offset, void const* data, std::size_t size, std::nothrow_t) noexcept;
    void insert(std::size_t offset, void const* data, std::size_t size);
    bool insert(std::size_t offset, std::string_view const& string, std::nothrow_t) noexcept;
    void insert(std::size_t offset, std::string_view const& string);

    void erase(std::size_t offset, std::size_t size) noexcept;

    bool copy(Buffer& buffer, std::nothrow_t) const noexcept;
    void copy(Buffer& buffer) const;
    Buffer copy() const;

    bool copy(std::size_t offset, std::size_t size, Buffer& buffer, std::nothrow_t) const noexcept;
    void copy(std::size_t offset, std::size_t size, Buffer& buffer) const;
    Buffer copy(std::size_t offset, std::size_t size) const;

    bool extract(std::size_t offset, std::size_t size, Buffer& buffer, std::nothrow_t) noexcept;
    void extract(std::size_t offset, std::size_t size, Buffer& buffer);
    Buffer extract(std::size_t offset, std::size_t size);

    bool extract(std::size_t offset, std::string_view const& sentinel, bool include_sentinel, Buffer& buffer, std::nothrow_t) noexcept;
    void extract(std::size_t offset, std::string_view const& sentinel, bool include_sentinel, Buffer& buffer);
    Buffer extract(std::size_t offset, std::string_view const& sentinel, bool include_sentinel);

private:
    void* m_data{ nullptr };
    std::size_t m_maximum{ InfiniteCapacity };
    std::size_t m_capacity{ 0 };
    std::size_t m_size{ 0 };

    enum ReallocType
    {
        Shrink,
        Grow,
        GrowZeroed
    };

    bool realloc(std::size_t capacity, ReallocType type) noexcept;
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

    void deallocate(__attribute__((unused)) T* ptr, __attribute__((unused)) std::size_t size) noexcept
    {
        assert(static_cast<std::uint8_t const*>(m_buffer.data()) + m_size == reinterpret_cast<std::uint8_t*>(ptr));
        assert(m_buffer.size() == m_size + size);
        m_buffer.resize(m_size);
    }

    template<typename U>
    bool operator ==(BufferAllocator<U> const& that) const noexcept
    {
        return m_buffer.data() == that.m_buffer.data();
    }

    template<typename U>
    bool operator !=(BufferAllocator<U> const& that) const noexcept
    {
        return m_buffer.data() != that.m_buffer.data();
    }

private:
    Buffer& m_buffer;
    std::size_t m_size;
};

std::string to_string(Buffer const& buffer);

std::shared_ptr<SourceAdapter<Buffer>> make_shared_source_buffer();
std::shared_ptr<SourceAdapter<Buffer>> make_shared_source_buffer(std::string_view const& string);
std::shared_ptr<SinkAdapter<Buffer>> make_shared_sink_buffer(std::size_t maximum = Sink::MinimalCapacity);

} // namespace hlib


