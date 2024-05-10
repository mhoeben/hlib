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
#include "hlib/buffer.hpp"
#include <limits>

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
inline void* transform(void* data, std::int8_t const& value) noexcept
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)value;
    return ptr + sizeof(int8_t);
}

template<>
inline void* transform(void* data, std::int16_t const& value) noexcept
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >>  8);
    ptr[1] = (uint8_t)(value >>  0);
    return ptr + sizeof(int16_t);
}

template<>
inline void* transform(void* data, std::int32_t const& value) noexcept
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >> 24);
    ptr[1] = (uint8_t)(value >> 16);
    ptr[2] = (uint8_t)(value >>  8);
    ptr[3] = (uint8_t)(value >>  0);
    return ptr + sizeof(int32_t);
}

template<>
inline void* transform(void* data, std::int64_t const& value) noexcept
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >> 56);
    ptr[1] = (uint8_t)(value >> 48);
    ptr[2] = (uint8_t)(value >> 40);
    ptr[3] = (uint8_t)(value >> 32);
    ptr[4] = (uint8_t)(value >> 24);
    ptr[5] = (uint8_t)(value >> 16);
    ptr[6] = (uint8_t)(value >>  8);
    ptr[7] = (uint8_t)(value >>  0);
    return ptr + sizeof(int64_t);
}

template<>
inline void* transform(void* data, std::uint8_t const& value) noexcept
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = value;
    return ptr + sizeof(uint8_t);
}

template<>
inline void* transform(void* data, std::uint16_t const& value) noexcept
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >>  8);
    ptr[1] = (uint8_t)(value >>  0);
    return ptr + sizeof(uint16_t);
}

template<>
inline void* transform(void* data, std::uint32_t const& value) noexcept
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >> 24);
    ptr[1] = (uint8_t)(value >> 16);
    ptr[2] = (uint8_t)(value >>  8);
    ptr[3] = (uint8_t)(value >>  0);
    return ptr + sizeof(uint32_t);
}

template<>
inline void* transform(void* data, std::uint64_t const& value) noexcept
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >> 56);
    ptr[1] = (uint8_t)(value >> 48);
    ptr[2] = (uint8_t)(value >> 40);
    ptr[3] = (uint8_t)(value >> 32);
    ptr[4] = (uint8_t)(value >> 24);
    ptr[5] = (uint8_t)(value >> 16);
    ptr[6] = (uint8_t)(value >>  8);
    ptr[7] = (uint8_t)(value >>  0);
    return ptr + sizeof(uint64_t);
}

template<>
inline void* transform(void* data, float const& value) noexcept
{
    union
    {
        float f;
        uint32_t n;
    } convert = { value };
    return transform(data, convert.n);
}

template<>
inline void* transform(void* data, double const& value) noexcept
{
    union
    {
        double f;
        uint64_t n;
    } convert = { value };
    return transform(data, convert.n);
}

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

    Buffer& buffer() noexcept;

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
inline void const* transform(void const* data, std::int8_t& value) noexcept
{
    uint8_t const* ptr = (uint8_t const*)data;
    value = (int8_t)ptr[0];
    return ptr + sizeof(int8_t);
}

template<>
inline void const* transform(void const* data, std::int16_t& value) noexcept
{
    uint8_t const* ptr = (uint8_t const*)data;
    value = ((int16_t)ptr[0] << 8)
          | ((int16_t)ptr[1] << 0);
    return ptr + sizeof(int16_t);
}

template<>
inline void const* transform(void const* data, std::int32_t& value) noexcept
{
    uint8_t const* ptr = (uint8_t const*)data;
    value = ((int32_t)ptr[0] << 24)
          | ((int32_t)ptr[1] << 16)
          | ((int32_t)ptr[2] <<  8)
          | ((int32_t)ptr[3] <<  0);
    return ptr + sizeof(int32_t);
}

template<>
inline void const* transform(void const* data, std::int64_t& value) noexcept
{
    uint8_t const* ptr = (uint8_t const*)data;
    value = ((int64_t)ptr[0] << 56)
          | ((int64_t)ptr[1] << 48)
          | ((int64_t)ptr[2] << 40)
          | ((int64_t)ptr[3] << 32)
          | ((int64_t)ptr[4] << 24)
          | ((int64_t)ptr[5] << 16)
          | ((int64_t)ptr[6] <<  8)
          | ((int64_t)ptr[7] <<  0);
    return ptr + sizeof(int64_t);
}

template<>
inline void const* transform(void const* data, std::uint8_t& value) noexcept
{
    uint8_t const* ptr = (uint8_t const*)data;
    value = ptr[0];
    return ptr + sizeof(uint8_t);
}

template<>
inline void const* transform(void const* data, std::uint16_t& value) noexcept
{
    uint8_t const* ptr = (uint8_t const*)data;
    value = ((uint16_t)ptr[0] << 8)
          | ((uint16_t)ptr[1] << 0);
    return ptr + sizeof(uint16_t);
}

template<>
inline void const* transform(void const* data, std::uint32_t& value) noexcept
{
    uint8_t const* ptr = (uint8_t const*)data;
    value = ((uint32_t)ptr[0] << 24)
          | ((uint32_t)ptr[1] << 16)
          | ((uint32_t)ptr[2] <<  8)
          | ((uint32_t)ptr[3] <<  0);
    return ptr + sizeof(uint32_t);
}

template<>
inline void const* transform(void const* data, std::uint64_t& value) noexcept
{
    uint8_t const* ptr = (uint8_t const*)data;
    value = ((uint64_t)ptr[0] << 56)
          | ((uint64_t)ptr[1] << 48)
          | ((uint64_t)ptr[2] << 40)
          | ((uint64_t)ptr[3] << 32)
          | ((uint64_t)ptr[4] << 24)
          | ((uint64_t)ptr[5] << 16)
          | ((uint64_t)ptr[6] <<  8)
          | ((uint64_t)ptr[7] <<  0);
    return ptr + sizeof(uint64_t);
}

template<>
inline void const* transform(void const* data, float& value) noexcept
{
    union
    {
        uint32_t n;
        float f;
    } convert;
    data = transform<std::uint32_t>(data, convert.n);
    value = convert.f;
    return data;
}

template<>
inline void const* transform(void const* data, double& value) noexcept
{
    union
    {
        uint64_t n;
        double f;
    } convert;
    data = transform<std::uint64_t>(data, convert.n);
    value = convert.f;
    return data;
}

template<typename T, typename U>
inline void const* transform(void const* data, T& value) noexcept
{
    U underlying;
    data = transform<U>(data, underlying);
    value = static_cast<T>(underlying);
    return data;
}

template<typename T>
inline T to(void const* data) noexcept
{
    T value;
    transform<T>(data, value);
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

    Buffer const& buffer() const noexcept;

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
inline void* transform(void* data, std::int8_t const& value) noexcept
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)value;
    return ptr + sizeof(int8_t);
}

template<>
inline void* transform(void* data, std::int16_t const& value) noexcept
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >>  0);
    ptr[1] = (uint8_t)(value >>  8);
    return ptr + sizeof(int16_t);
}

template<>
inline void* transform(void* data, std::int32_t const& value) noexcept
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >>  0);
    ptr[1] = (uint8_t)(value >>  8);
    ptr[2] = (uint8_t)(value >> 16);
    ptr[3] = (uint8_t)(value >> 24);
    return ptr + sizeof(int32_t);
}

template<>
inline void* transform(void* data, std::int64_t const& value) noexcept
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >>  0);
    ptr[1] = (uint8_t)(value >>  8);
    ptr[2] = (uint8_t)(value >> 16);
    ptr[3] = (uint8_t)(value >> 24);
    ptr[4] = (uint8_t)(value >> 32);
    ptr[5] = (uint8_t)(value >> 40);
    ptr[6] = (uint8_t)(value >> 48);
    ptr[7] = (uint8_t)(value >> 56);
    return ptr + sizeof(int64_t);
}

template<>
inline void* transform(void* data, std::uint8_t const& value) noexcept
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = value;
    return ptr + sizeof(uint8_t);
}

template<>
inline void* transform(void* data, std::uint16_t const& value) noexcept
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >>  0);
    ptr[1] = (uint8_t)(value >>  8);
    return ptr + sizeof(uint16_t);
}

template<>
inline void* transform(void* data, std::uint32_t const& value) noexcept
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >>  0);
    ptr[1] = (uint8_t)(value >>  8);
    ptr[2] = (uint8_t)(value >> 16);
    ptr[3] = (uint8_t)(value >> 24);
    return ptr + sizeof(uint32_t);
}

template<>
inline void* transform(void* data, std::uint64_t const& value) noexcept
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >>  0);
    ptr[1] = (uint8_t)(value >>  8);
    ptr[2] = (uint8_t)(value >> 16);
    ptr[3] = (uint8_t)(value >> 24);
    ptr[4] = (uint8_t)(value >> 32);
    ptr[5] = (uint8_t)(value >> 40);
    ptr[6] = (uint8_t)(value >> 48);
    ptr[7] = (uint8_t)(value >> 56);
    return ptr + sizeof(uint64_t);
}

template<>
inline void* transform(void* data, float const& value) noexcept
{
    union
    {
        float f;
        uint32_t n;
    } convert = { value };
    return transform<uint32_t>(data, convert.n);
}

template<>
inline void* transform(void* data, double const& value) noexcept
{
    union
    {
        double f;
        uint64_t n;
    } convert = { value };
    return transform<uint64_t>(data, convert.n);
}

template<typename T, typename U>
inline void* transform(void* data, T const& value) noexcept
{
    return transform<U>(data, static_cast<U>(value));
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

    Buffer& buffer() noexcept;

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
inline void const* transform(void const* data, std::int8_t& value) noexcept
{
    uint8_t const* ptr = (uint8_t const*)data;
    value = (int8_t)ptr[0];
    return ptr + sizeof(int8_t);
}

template<>
inline void const* transform(void const* data, std::int16_t& value) noexcept
{
    uint8_t const* ptr = (uint8_t const*)data;
    value = ((int16_t)ptr[0] << 0)
          | ((int16_t)ptr[1] << 8);
    return ptr + sizeof(int16_t);
}

template<>
inline void const* transform(void const* data, std::int32_t& value) noexcept
{
    uint8_t const* ptr = (uint8_t const*)data;
    value = ((int32_t)ptr[0] <<  0)
          | ((int32_t)ptr[1] <<  8)
          | ((int32_t)ptr[2] << 16)
          | ((int32_t)ptr[3] << 24);
    return ptr + sizeof(int32_t);
}

template<>
inline void const* transform(void const* data, std::int64_t& value) noexcept
{
    uint8_t const* ptr = (uint8_t const*)data;
    value = ((int64_t)ptr[0] <<  0)
          | ((int64_t)ptr[1] <<  8)
          | ((int64_t)ptr[2] << 16)
          | ((int64_t)ptr[3] << 24)
          | ((int64_t)ptr[4] << 32)
          | ((int64_t)ptr[5] << 40)
          | ((int64_t)ptr[6] << 48)
          | ((int64_t)ptr[7] << 56);
    return ptr + sizeof(int64_t);
}

template<>
inline void const* transform(void const* data, std::uint8_t& value) noexcept
{
    uint8_t const* ptr = (uint8_t const*)data;
    value = ptr[0];
    return ptr + sizeof(uint8_t);
}

template<>
inline void const* transform(void const* data, std::uint16_t& value) noexcept
{
    uint8_t const* ptr = (uint8_t const*)data;
    value = ((uint16_t)ptr[0] << 0)
          | ((uint16_t)ptr[1] << 8);
    return ptr + sizeof(uint16_t);
}

template<>
inline void const* transform(void const* data, std::uint32_t& value) noexcept
{
    uint8_t const* ptr = (uint8_t const*)data;
    value = ((uint32_t)ptr[0] <<  0)
          | ((uint32_t)ptr[1] <<  8)
          | ((uint32_t)ptr[2] << 16)
          | ((uint32_t)ptr[3] << 24);
    return ptr + sizeof(uint32_t);
}

template<>
inline void const* transform(void const* data, std::uint64_t& value) noexcept
{
    uint8_t const* ptr = (uint8_t const*)data;
    value = ((uint64_t)ptr[0] <<  0)
          | ((uint64_t)ptr[1] <<  8)
          | ((uint64_t)ptr[2] << 16)
          | ((uint64_t)ptr[3] << 24)
          | ((uint64_t)ptr[4] << 32)
          | ((uint64_t)ptr[5] << 40)
          | ((uint64_t)ptr[6] << 48)
          | ((uint64_t)ptr[7] << 56);
    return ptr + sizeof(uint64_t);
}

template<>
inline void const* transform(void const* data, float& value) noexcept
{
    union
    {
        uint32_t n;
        float f;
    } convert;
    data = transform<uint32_t>(data, convert.n);
    value = convert.f;
    return data;
}

template<>
inline void const* transform(void const* data, double& value) noexcept
{
    union
    {
        uint64_t n;
        double f;
    } convert;
    data = transform<std::uint64_t>(data, convert.n);
    value = convert.f;
    return data;
}

template<typename T, typename U>
inline void const* transform(void const* data, T& value) noexcept
{
    U underlying;
    data = transform<U>(data, underlying);
    value = static_cast<T>(underlying);
    return data;
}

template<typename T>
inline T to(void const* data) noexcept
{
    T value;
    transform<T>(data, value);
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

    Buffer const& buffer() const noexcept;

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

