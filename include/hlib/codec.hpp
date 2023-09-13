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
#include <memory>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace hlib
{

class Buffer;

namespace codec
{

class Encoder;
class Decoder;

struct Type
{
    typedef int Id;

    virtual explicit operator Id() const noexcept = 0;
    virtual explicit operator std::size_t() const noexcept = 0;

    virtual void operator()(Encoder& encoder) const = 0;
    virtual void operator()(Decoder& decoder) = 0;
};

struct Map
{
    std::size_t size;

    Map(std::size_t a_size = 0) noexcept
        : size{ a_size }
    {
    }
};

struct Array
{ 
    std::size_t size;

    Array(std::size_t a_size = 0) noexcept
        : size{ a_size }
    {
    }
};

struct Binary
{
    void const* data;
    std::size_t size;

    Binary(void const* a_data = nullptr, size_t a_size = 0)
        : data(a_data)
        , size(a_size)
    {
    }

    Binary(std::string const& string)
        : data(string.data())
        , size(string.size())
    {
    }

    bool operator ==(Binary const& that) const noexcept
    {
        return size == that.size
            && 0 == memcmp(data, that.data, that.size);
    }

    bool operator !=(Binary const& that) const noexcept
    {
        return size != that.size
            || 0 == memcmp(data, that.data, that.size);
    }
};

class Encoder
{
public:
    static std::unique_ptr<Encoder> create(std::string const& kind, Buffer& buffer);

public:
    virtual ~Encoder() = default;

    virtual bool isBinary() const = 0;

    virtual void open(char const* name, Type const& value) = 0;
    virtual void open(char const* name, Array const& value) = 0;
    virtual void open(char const* name, Map const& value) = 0;
    virtual void encode(char const* name, bool const& value) = 0;
    virtual void encode(char const* name, std::int32_t const& value) = 0;
    virtual void encode(char const* name, std::int64_t const& value) = 0;
    virtual void encode(char const* name, float const& value) = 0;
    virtual void encode(char const* name, double const& value) = 0;
    virtual void encode(char const* name, std::string const& value) = 0;
    virtual void encode(char const* name, Binary const& value) = 0;
    virtual void close() = 0;

    template<typename T>
    void encode(char const* name, std::vector<T> const& values)
    {
        open(name, Array{ values.size() });
        for (T const& value : values) {
            encode(nullptr, value);
        }
        close();
    }

    virtual void wrap(Type const& value);
};

class Decoder
{
public:
    static std::unique_ptr<Decoder> create(std::string const& kind, void const* data, std::size_t size);

public:
    virtual ~Decoder() = default;

    virtual void reset(void const* data, std::size_t size) = 0;

    virtual void open(char const* name, Type& value) = 0;
    virtual void open(char const* name, Array& value) = 0;
    virtual void open(char const* name, Map& value) = 0;
    virtual void decode(char const* name, bool& value) = 0;
    virtual void decode(char const* name, std::int32_t& value) = 0;
    virtual void decode(char const* name, std::int64_t& value) = 0;
    virtual void decode(char const* name, float& value) = 0;
    virtual void decode(char const* name, double& value) = 0;
    virtual void decode(char const* name, std::string& value) = 0;
    virtual void decode(char const* name, Binary& value) = 0;
    virtual void close() = 0;

    template<typename T>
    void decode(char const* name, std::vector<T>& values)
    {
        Array array;

        open(name, array);

        values.reserve(array.size);

        for (size_t i = 0; i < array.size; ++i) {
            T value;
            decode(nullptr, value);
            values.emplace_back(std::move(value));
        }
        close();
    }

    virtual bool more() const = 0;
    virtual Type::Id peek() const = 0;
    virtual void unwrap(Type& value);
};

} // namespace codec
} // namespace hlib

