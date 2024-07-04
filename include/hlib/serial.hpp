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
#include "hlib/sink.hpp"
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
    HLIB_NOT_COPYABLE(Serializer);
    HLIB_NOT_MOVABLE(Serializer);

public:
    Serializer(Sink& sink) noexcept
        : m_sink(sink)
    {
    }

    template<typename T, typename U = T>
    typename std::enable_if<std::is_arithmetic<U>::value, Serializer&>::type
        transform(T const& value) noexcept
    {
        assert(sizeof(U) <= m_sink.headroom());

        be::transform<T, U>(m_sink.produce(sizeof(U)), value);
        return *this;
    }

    template<typename T>
    typename std::enable_if<false == std::is_arithmetic<T>::value
                         && true == has_size_method<T>::value
                         && true == has_data_method<T>::value, Serializer&>::type
        transform(T const& value) noexcept
    {
        assert(value.size() <= m_sink.headroom());

        m_sink.produce(value.data(), value.size());
        return *this;
    }

private:
    Sink& m_sink;
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
    HLIB_NOT_COPYABLE(Deserializer);
    HLIB_NOT_MOVABLE(Deserializer);

public:
    Deserializer(Source& source) noexcept
        : m_source(source)
    {
    }

    template<typename T, typename U = T>
    typename std::enable_if<std::is_arithmetic<U>::value, Deserializer&>::type
        transform(T& value) noexcept
    {
        assert(sizeof(T) <= m_source.available());

        be::transform<T, U>(m_source.consume(sizeof(U)), value);
        return *this;
    }

    template<typename T>
    typename std::enable_if<false == std::is_arithmetic<T>::value
                         && true == has_data_method<T>::value
                         && true == has_resize_method<T>::value, Deserializer&>::type
        transform(T& value, std::size_t size) noexcept
    {
        assert(size <= m_source.available());

        std::size_t before_resize = value.size();
        value.resize(before_resize + size);

        m_source.consume(reinterpret_cast<uint8_t*>(value.data()) + before_resize, size);
        return *this;
    }

private:
    Source& m_source;
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
    HLIB_NOT_COPYABLE(Serializer);
    HLIB_NOT_MOVABLE(Serializer);

public:
    Serializer(Sink& sink) noexcept
        : m_sink(sink)
    {
    }

    template<typename T, typename U = T>
    typename std::enable_if<std::is_arithmetic<U>::value, Serializer&>::type
        transform(T const& value) noexcept
    {
        assert(sizeof(U) <= m_sink.headroom());

        le::transform<T, U>(m_sink.produce(sizeof(U)), value);
        return *this;
    }

    template<typename T>
    typename std::enable_if<false == std::is_arithmetic<T>::value
                         && true == has_size_method<T>::value
                         && true == has_data_method<T>::value, Serializer&>::type
        transform(T const& value) noexcept
    {
        assert(value.size() <= m_sink.headroom());

        m_sink.produce(value.data(), value.size());
        return *this;
    }

private:
    Sink& m_sink;
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
    HLIB_NOT_COPYABLE(Deserializer);
    HLIB_NOT_MOVABLE(Deserializer);

public:
    Deserializer(Source& source) noexcept
        : m_source(source)
    {
    }

    template<typename T, typename U = T>
    typename std::enable_if<std::is_arithmetic<U>::value, Deserializer&>::type
        transform(T& value) noexcept
    {
        assert(sizeof(T) <= m_source.available());

        le::transform<T, U>(m_source.consume(sizeof(U)), value);
        return *this;
    }

    template<typename T>
    typename std::enable_if<false == std::is_arithmetic<T>::value
                         && true == has_data_method<T>::value
                         && true == has_resize_method<T>::value, Deserializer&>::type
        transform(T& value, std::size_t size) noexcept
    {
        assert(size <= m_source.available());

        std::size_t before_resize = value.size();
        value.resize(before_resize + size);

        m_source.consume(reinterpret_cast<uint8_t*>(value.data()) + before_resize, size);
        return *this;
    }

private:
    Source& m_source;
};

} // namespace le
} // namespace hlib

