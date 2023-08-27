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

#include "hlib/buffer.hpp"
#include "hlib/c/endian.h"
#include <limits>
#include <new>

namespace hlib
{

namespace be
{

//
// Serialize
//
template<typename T>
inline void* transform(void* data, T const& value) noexcept;

template<>
inline void* transform(void* data, int8_t const& value) noexcept
    { return hlib_be_set_int8(data, value); }

template<>
inline void* transform(void* data, int16_t const& value) noexcept
    { return hlib_be_set_int16(data, value); }

template<>
inline void* transform(void* data, int32_t const& value) noexcept
    { return hlib_be_set_int32(data, value); }

template<>
inline void* transform(void* data, int64_t const& value) noexcept
    { return hlib_be_set_int64(data, value); }

template<>
inline void* transform(void* data, uint8_t const& value) noexcept
    { return hlib_be_set_uint8(data, value); }

template<>
inline void* transform(void* data, uint16_t const& value) noexcept
    { return hlib_be_set_uint16(data, value); }

template<>
inline void* transform(void* data, uint32_t const& value) noexcept
    { return hlib_be_set_uint32(data, value); }

template<>
inline void* transform(void* data, uint64_t const& value) noexcept
    { return hlib_be_set_uint64(data, value); }

template<>
inline void* transform(void* data, float const& value) noexcept
    { return hlib_be_set_float(data, value); }

template<>
inline void* transform(void* data, double const& value) noexcept
    { return hlib_be_set_double(data, value); }

template<typename T, typename U>
inline void* transform(void* data, T const& value) noexcept
{
    return transform(data, static_cast<U>(value));
}

class Serializer final
{
public:
    Serializer(void* data, [[maybe_unused]] std::size_t capacity = std::numeric_limits<std::size_t>::max());

    void* data() const noexcept;

    template<typename T, typename U = T>
    inline Serializer& transform(T const& value) noexcept
    {
        assert(sizeof(U) <= m_capacity);

        m_data = be::transform(m_data, static_cast<U>(value));
#ifndef NDEBUG
        m_capacity -= sizeof(U);
#endif
        return *this;
    }

private:
    void* m_data;
#ifndef NDEBUG
    std::size_t m_capacity;
#endif
};

class BufferSerializer final
{
    HLIB_NOT_COPYABLE(BufferSerializer);
    HLIB_NOT_MOVABLE(BufferSerializer);

public:
    BufferSerializer(Buffer& buffer);

    template<typename T, typename U = T>
    BufferSerializer& transform(T const& value) noexcept
    {
        be::transform<U>(m_buffer.extend(sizeof(U)), static_cast<U>(value));
        m_buffer.resize(m_buffer.size() + sizeof(U));
        return *this;
    }

private:
    Buffer& m_buffer;
};

//
// Deserialize
//
template<typename T>
inline void const* transform(void const* data, T& value) noexcept;

template<>
inline void const* transform(void const* data, int8_t& value) noexcept
    { return hlib_be_get_int8(data, &value); }

template<>
inline void const* transform(void const* data, int16_t& value) noexcept
    { return hlib_be_get_int16(data, &value); }

template<>
inline void const* transform(void const* data, int32_t& value) noexcept
    { return hlib_be_get_int32(data, &value); }

template<>
inline void const* transform(void const* data, int64_t& value) noexcept
    { return hlib_be_get_int64(data, &value); }

template<>
inline void const* transform(void const* data, uint8_t& value) noexcept
    { return hlib_be_get_uint8(data, &value); }

template<>
inline void const* transform(void const* data, uint16_t& value) noexcept
    { return hlib_be_get_uint16(data, &value); }

template<>
inline void const* transform(void const* data, uint32_t& value) noexcept
    { return hlib_be_get_uint32(data, &value); }

template<>
inline void const* transform(void const* data, uint64_t& value) noexcept
    { return hlib_be_get_uint64(data, &value); }

template<>
inline void const* transform(void const* data, float& value) noexcept
    { return hlib_be_get_float(data, &value); }

template<>
inline void const* transform(void const* data, double& value) noexcept
    { return hlib_be_get_double(data, &value); }

template<typename T, typename U>
inline void const* transform(void const* data, T& value) noexcept
{
    U underlying;
    data = transform(data, underlying);
    value = static_cast<T>(underlying);
    return data;
}

template<typename T>
inline T to(void const* data) noexcept
{
    T value;
    transform(data, value);
    return value;
}

class Deserializer final
{
public:
    Deserializer(void* data, [[maybe_unused]] std::size_t size = std::numeric_limits<std::size_t>::max());

    template<typename T>
    inline Deserializer& transform(T& value) noexcept
    {
        assert(sizeof(T) <= m_size);

        m_data = be::transform<T>(m_data, value);
#ifndef NDEBUG
        m_size -= sizeof(T);
#endif
        return *this;
    }

    template<typename T, typename U>
    inline Deserializer& transform(T& value) noexcept
    {
        U underlying;
        transform(underlying);
        value = static_cast<T>(underlying);
        return *this;
    }

private:
    void* m_data;
#ifndef NDEBUG
    size_t m_size;
#endif
};

class BufferDeserializer final
{
public:
    BufferDeserializer(Buffer const& buffer, std::size_t offset = 0);

    template<typename T>
    inline BufferDeserializer& transform(T& value)
    {
        if (m_offset + sizeof(T) > m_buffer.size()) {
            throw std::out_of_range("transform()");
        }

        be::transform<T>(m_buffer.get(m_offset), value);
        m_offset += sizeof(T);
        return *this;
    }

    template<typename T, typename U>
    inline BufferDeserializer& transform(T& value)
    {
        U underlying;
        transform(underlying);
        value = static_cast<T>(underlying);
        return *this;
    }

private:
    Buffer const& m_buffer;
    std::size_t m_offset{ 0 };
};

} // namespace be

namespace le
{

//
// Serialize
//
template<typename T>
inline void* transform(void* data, T const& value) noexcept;

template<>
inline void* transform(void* data, int8_t const& value) noexcept
    { return hlib_le_set_int8(data, value); }

template<>
inline void* transform(void* data, int16_t const& value) noexcept
    { return hlib_le_set_int16(data, value); }

template<>
inline void* transform(void* data, int32_t const& value) noexcept
    { return hlib_le_set_int32(data, value); }

template<>
inline void* transform(void* data, int64_t const& value) noexcept
    { return hlib_le_set_int64(data, value); }

template<>
inline void* transform(void* data, uint8_t const& value) noexcept
    { return hlib_le_set_uint8(data, value); }

template<>
inline void* transform(void* data, uint16_t const& value) noexcept
    { return hlib_le_set_uint16(data, value); }

template<>
inline void* transform(void* data, uint32_t const& value) noexcept
    { return hlib_le_set_uint32(data, value); }

template<>
inline void* transform(void* data, uint64_t const& value) noexcept
    { return hlib_le_set_uint64(data, value); }

template<>
inline void* transform(void* data, float const& value) noexcept
    { return hlib_le_set_float(data, value); }

template<>
inline void* transform(void* data, double const& value) noexcept
    { return hlib_le_set_double(data, value); }

template<typename T, typename U>
inline void* transform(void* data, T const& value) noexcept
{
    return transform(data, static_cast<U>(value));
}

class Serializer final
{
public:
    Serializer(void* data, [[maybe_unused]] std::size_t capacity = std::numeric_limits<std::size_t>::max());

    void* data() const noexcept;

    template<typename T, typename U = T>
    inline Serializer& transform(T const& value) noexcept
    {
        assert(sizeof(U) <= m_capacity);

        m_data = le::transform(m_data, static_cast<U>(value));
#ifndef NDEBUG
        m_capacity -= sizeof(U);
#endif
        return *this;
    }

private:
    void* m_data;
#ifndef NDEBUG
    std::size_t m_capacity;
#endif
};

class BufferSerializer final
{
    HLIB_NOT_COPYABLE(BufferSerializer);
    HLIB_NOT_MOVABLE(BufferSerializer);

public:
    BufferSerializer(Buffer& buffer);

    template<typename T, typename U = T>
    BufferSerializer& transform(T const& value) noexcept
    {
        le::transform<U>(m_buffer.extend(sizeof(U)), static_cast<U>(value));
        m_buffer.resize(m_buffer.size() + sizeof(U));
        return *this;
    }

private:
    Buffer& m_buffer;
};

//
// Deserialize
//
template<typename T>
inline void const* transform(void const* data, T& value) noexcept;

template<>
inline void const* transform(void const* data, int8_t& value) noexcept
    { return hlib_le_get_int8(data, &value); }

template<>
inline void const* transform(void const* data, int16_t& value) noexcept
    { return hlib_le_get_int16(data, &value); }

template<>
inline void const* transform(void const* data, int32_t& value) noexcept
    { return hlib_le_get_int32(data, &value); }

template<>
inline void const* transform(void const* data, int64_t& value) noexcept
    { return hlib_le_get_int64(data, &value); }

template<>
inline void const* transform(void const* data, uint8_t& value) noexcept
    { return hlib_le_get_uint8(data, &value); }

template<>
inline void const* transform(void const* data, uint16_t& value) noexcept
    { return hlib_le_get_uint16(data, &value); }

template<>
inline void const* transform(void const* data, uint32_t& value) noexcept
    { return hlib_le_get_uint32(data, &value); }

template<>
inline void const* transform(void const* data, uint64_t& value) noexcept
    { return hlib_le_get_uint64(data, &value); }

template<>
inline void const* transform(void const* data, float& value) noexcept
    { return hlib_le_get_float(data, &value); }

template<>
inline void const* transform(void const* data, double& value) noexcept
    { return hlib_le_get_double(data, &value); }

template<typename T, typename U>
inline void const* transform(void const* data, T& value) noexcept
{
    U underlying;
    data = transform(data, underlying);
    value = static_cast<T>(underlying);
    return data;
}

template<typename T>
inline T to(void const* data) noexcept
{
    T value;
    transform(data, value);
    return value;
}

class Deserializer final
{
public:
    Deserializer(void* data, [[maybe_unused]] std::size_t size = std::numeric_limits<std::size_t>::max());

    template<typename T>
    inline Deserializer& transform(T& value) noexcept
    {
        assert(sizeof(T) <= m_size);

        m_data = le::transform<T>(m_data, value);
#ifndef NDEBUG
        m_size -= sizeof(T);
#endif
        return *this;
    }

    template<typename T, typename U>
    inline Deserializer& transform(T& value) noexcept
    {
        U underlying;
        transform(underlying);
        value = static_cast<T>(underlying);
        return *this;
    }

private:
    void* m_data;
#ifndef NDEBUG
    size_t m_size;
#endif
};

class BufferDeserializer final
{
public:
    BufferDeserializer(Buffer const& buffer, std::size_t offset = 0);

    template<typename T>
    inline BufferDeserializer& transform(T& value)
    {
        if (m_offset + sizeof(T) > m_buffer.size()) {
            throw std::out_of_range("transform()");
        }

        le::transform<T>(m_buffer.get(m_offset), value);
        m_offset += sizeof(T);
        return *this;
    }

    template<typename T, typename U>
    inline BufferDeserializer& transform(T& value)
    {
        U underlying;
        transform(underlying);
        value = static_cast<T>(underlying);
        return *this;
    }

private:
    Buffer const& m_buffer;
    std::size_t m_offset{ 0 };
};

} // namespace le
} // namespace hlib

