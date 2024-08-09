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
#include "hlib/buffer.hpp"
#include <cstring>

using namespace hlib;

//
// Buffer Implementation
//
bool Buffer::realloc(std::size_t capacity, bool shrink) noexcept
{
    if (false == shrink && capacity <= m_capacity) {
        return true;
    }

    if (capacity > m_maximum) {
        return false;
    }

    void* data;

    if (capacity > 0) {
        data = ::realloc(m_data, capacity);
        if (nullptr == data) {
            return false;
        }
    }
    else {
        if (nullptr != m_data) {
            free(m_data);
        }
        data = nullptr;
    }

    m_data = data;
    m_capacity = capacity;
    m_size = std::min(m_size, m_capacity);
    return true;
}

//
// Buffer Public
//
Buffer::Buffer(std::size_t reservation, std::size_t maximum)
    : m_maximum(maximum)
{
    reserve(std::min(reservation, maximum));
}

Buffer::Buffer(std::size_t reservation)
{
    reserve(reservation);
}

Buffer::Buffer(void const* data, std::size_t size)
{
    assign(data, size);
}

Buffer::Buffer(std::string_view const& string)
{
    assign(string.data(), string.size());
}

Buffer::Buffer(char const* string)
{
    assign(string);
}

Buffer::Buffer(Buffer&& that) noexcept
    : m_data{ that.m_data }
    , m_maximum{ that.m_maximum }
    , m_capacity{ that.m_capacity }
    , m_size{ that.m_size }
{
    that.m_data = nullptr;
    that.m_capacity = 0;
    that.m_size = 0;
}

Buffer::~Buffer()
{
    reset();
}

Buffer& Buffer::operator =(Buffer&& that) noexcept
{
    reset();

    m_maximum = that.m_maximum;
    std::swap(m_data, that.m_data);
    std::swap(m_capacity, that.m_capacity);
    std::swap(m_size, that.m_size);
    return *this;
}

void const* Buffer::data() const noexcept
{
    return m_data;
}

void* Buffer::data() noexcept
{
    return m_data;
}

std::size_t Buffer::capacity() const noexcept
{
    return m_capacity;
}

std::size_t Buffer::size() const noexcept
{
    return m_size;
}

bool Buffer::empty() const noexcept
{
    return 0 == m_size;
}

void const* Buffer::get(std::size_t index, std::nothrow_t) const noexcept
{
    assert(index < m_capacity);
    return static_cast<std::uint8_t*>(m_data) + index;
}

void const* Buffer::get(std::size_t index) const
{
    if (index >= m_capacity) {
        throw std::out_of_range("Buffer::get() failed");
    }

    return static_cast<std::uint8_t*>(m_data) + index;
}

std::byte Buffer::operator[](std::size_t index) const noexcept
{
    return *static_cast<std::byte const*>(get(index, std::nothrow));
}

std::byte Buffer::at(std::size_t index, std::nothrow_t) const noexcept
{
    return *static_cast<std::byte const*>(get(index, std::nothrow));
}

std::byte Buffer::at(std::size_t index) const
{
    return *static_cast<std::byte const*>(get(index));
}

void Buffer::reset() noexcept
{
    if (nullptr != m_data) {
        free(m_data);
    }
    m_data = nullptr;
    m_capacity = 0;
    m_size = 0;
}

void Buffer::clear() noexcept
{
    m_size = 0;
}

void Buffer::shrink() noexcept
{
    assert(m_size <= m_capacity);

    if (nullptr == m_data || 0 == m_size) {
        reset();
        return;
    }

    (void)realloc(m_size, true);
}

void* Buffer::reserve(std::size_t capacity, std::nothrow_t) noexcept
{
    assert(m_size <= m_capacity);

    if (false == realloc(capacity, false)) {
        return nullptr;
    }

    return m_data;
}

void* Buffer::reserve(std::size_t capacity)
{
    void* data = reserve(capacity, std::nothrow);
    if (nullptr == data && capacity > 0) {
        throw std::bad_alloc();
    }
    return data;
}

void* Buffer::extend(std::size_t capacity, std::nothrow_t) noexcept
{
    assert(m_size <= m_capacity);

    if (capacity > 0 && false == realloc(m_size + capacity, false)) {
        return nullptr;
    }

    return static_cast<std::uint8_t*>(m_data) + m_size;
}

void* Buffer::extend(std::size_t capacity)
{
    void* data = extend(capacity, std::nothrow);
    if (nullptr == data && capacity > 0) {
        throw std::bad_alloc();
    }
    return data;
}

void* Buffer::resize(std::size_t size, std::nothrow_t) noexcept
{
    assert(m_size <= m_capacity);

    if (size > 0 && false == realloc(size, false)) {
        return nullptr;
    }

    m_size = size;
    return m_data;
}

void* Buffer::resize(std::size_t size)
{
    void* data = resize(size, std::nothrow);
    if (nullptr == data && size > 0) {
        throw std::bad_alloc();
    }
    return data;
}

bool Buffer::assign(void const* data, std::size_t size, std::nothrow_t) noexcept
{
    m_size = 0;
    return append(data, size, std::nothrow);
}

void Buffer::assign(void const* data, std::size_t size)
{
    if (false == assign(data, size, std::nothrow)) {
        throw std::bad_alloc();
    }
}

bool Buffer::assign(std::string_view const& string, std::nothrow_t) noexcept
{
    return assign(string.data(), string.size(), std::nothrow);
}

void Buffer::assign(std::string_view const& string)
{
    assign(string.data(), string.size());
}

bool Buffer::append(void const* data, std::size_t size, std::nothrow_t) noexcept
{
    return insert(m_size, data, size, std::nothrow);
}

void Buffer::append(void const* data, std::size_t size)
{
    if (false == append(data, size, std::nothrow)) {
        throw std::bad_alloc();
    }
}

bool Buffer::append(std::string_view const& string, std::nothrow_t) noexcept
{
    return append(string.data(), string.size(), std::nothrow);
}

void Buffer::append(std::string_view const& string)
{
    append(string.data(), string.size());
}

bool Buffer::insert(std::size_t offset, void const* data, std::size_t size, std::nothrow_t) noexcept
{
    assert(offset <= m_size);
    assert(m_size <= m_capacity);

    if (0 == size) {
        return true;
    }

    if (false == realloc(m_size + size, false)) {
        return false;
    }

    std::uint8_t* ptr = static_cast<std::uint8_t*>(m_data);

    // Move tail. Note that when inserting at the tail, this is a NOP.
    memmove(ptr + offset + size, ptr + offset, m_size - offset);

    // Insert can be used to insert space in the middle without copying data.
    if (nullptr != data) {
        memcpy(ptr + offset, data, size);
    }
    m_size += size;
    return true;
}

void Buffer::insert(std::size_t offset, void const* data, std::size_t size)
{
    if (false == insert(offset, data, size, std::nothrow)) {
        throw std::bad_alloc();
    }
}

bool Buffer::insert(std::size_t offset, std::string_view const& string, std::nothrow_t) noexcept
{
    return insert(offset, string.data(), string.size(), std::nothrow);
}

void Buffer::insert(std::size_t offset, std::string_view const& string)
{
    insert(offset, string.data(), string.size());
}

void Buffer::erase(std::size_t offset, std::size_t size) noexcept
{
    assert(offset + size <= m_size);
    assert(m_size <= m_capacity);

    std::uint8_t* ptr = static_cast<std::uint8_t*>(m_data);

    memmove(ptr + offset, ptr + offset + size, m_size - (offset + size));
    m_size -= size;
}

bool Buffer::copy(Buffer& buffer, std::nothrow_t) const noexcept
{
    return buffer.assign(m_data, m_size, std::nothrow);
}

void Buffer::copy(Buffer& buffer) const
{
    if (false == copy(buffer, std::nothrow)) {
        throw std::bad_alloc();
    }
}

Buffer Buffer::copy() const
{
    return Buffer(m_data, m_size);
}

bool Buffer::copy(std::size_t offset, std::size_t size, Buffer& buffer, std::nothrow_t) const noexcept
{
    assert(offset + size <= m_size);

    std::uint8_t const* ptr = static_cast<std::uint8_t*>(m_data);
    return buffer.assign(ptr + offset, size, std::nothrow);
}

void Buffer::copy(std::size_t offset, std::size_t size, Buffer& buffer) const
{
    if (false == copy(offset, size, buffer, std::nothrow)) {
        throw std::bad_alloc();
    }
}

Buffer Buffer::copy(std::size_t offset, std::size_t size) const
{
    Buffer buffer;
    copy(offset, size, buffer);
    return buffer;
}

bool Buffer::extract(std::size_t offset, std::size_t size, Buffer& buffer, std::nothrow_t) noexcept
{
    assert(offset + size <= m_size);

    std::uint8_t const* ptr = static_cast<std::uint8_t*>(m_data);

    if (false == buffer.assign(ptr + offset, size, std::nothrow)) {
        return false;
    }
    erase(offset, size);
    return true;
}

void Buffer::extract(std::size_t offset, std::size_t size, Buffer& buffer)
{
    if (false == extract(offset, size, buffer, std::nothrow)) {
        throw std::bad_alloc();
    }
}

Buffer Buffer::extract(std::size_t offset, std::size_t size)
{
    Buffer buffer;
    extract(offset, size, buffer);
    return buffer;
}

bool Buffer::extract(std::size_t offset, std::string_view const& sentinel, bool include_sentinel, Buffer& buffer, std::nothrow_t) noexcept
{
    assert(offset <= m_size);

    std::string_view view(static_cast<char const*>(m_data), m_size);
    std::size_t position = view.find(sentinel, offset);

    if (position != std::string_view::npos) {
        assert(position >= offset);
        std::size_t size = position - offset;

        if (true == include_sentinel) {
            size += sentinel.size();
        }

        if (false == buffer.assign(view.data() + offset, size, std::nothrow)) {
            return false;
        }
        erase(offset, size);
        return true;
   }
    else {
        buffer.clear();
        return true;
    }
}

void Buffer::extract(std::size_t offset, std::string_view const& sentinel, bool include_sentinel, Buffer& buffer)
{
    if (false == extract(offset, sentinel, include_sentinel, buffer, std::nothrow)) {
        throw std::bad_alloc();
    }
}

Buffer Buffer::extract(std::size_t offset, std::string_view const& sentinel, bool include_sentinel)
{
    Buffer buffer;
    extract(offset, sentinel, include_sentinel, buffer);
    return buffer;
}

//
// Utility
//
std::string hlib::to_string(Buffer const& buffer)
{
    return std::string(static_cast<char const*>(buffer.data()), buffer.size());
}

std::shared_ptr<SourceAdapter<Buffer>> hlib::make_shared_source_buffer()
{
    return make_shared_source<Buffer>(Buffer());
}

std::shared_ptr<SourceAdapter<Buffer>> hlib::make_shared_source_buffer(std::string_view const& string)
{
    return make_shared_source<Buffer>(Buffer(string));
}

std::shared_ptr<SinkAdapter<Buffer>> hlib::make_shared_sink_buffer(std::size_t maximum)
{
    return make_shared_sink<Buffer>(maximum);
}


