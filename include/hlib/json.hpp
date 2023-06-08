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

namespace hlib
{

struct JSONData;

class JSONIterator;

class JSON final
{
    friend class JSONIterator;

public:
    enum Type
    {
        Undefined,
        Null,
        Bool,
        Number,
        String,
        Array,
        Object
    };

    using Iterator = JSONIterator;

public:
    JSON() = default;
    JSON(std::string string);
    JSON(JSON const& that);
    JSON(JSON&& that) noexcept;

    JSON& operator =(JSON const& that);
    JSON& operator =(JSON&& that) noexcept;

    bool empty() const noexcept;
    Type type() const noexcept;
    std::string name() const;
    std::string value() const;
    std::size_t size() const;

    int line() const;
    int column() const;
    int start() const;
    int stop() const;

    template<typename T>
    T as() const;

    bool operator ==(JSON const& that) const;
    bool operator !=(JSON const& that) const;

    JSON operator[](int index) const;
    JSON operator[](std::string const& name) const;

    JSON at(int index) const;
    JSON at(std::string const& name) const;

    Iterator begin() const;
    Iterator end() const;

    Iterator find(std::string const& name) const;
    bool contains(std::string const& name) const;

    void clear();
    void parse(std::string string);

private:
    std::shared_ptr<JSONData> m_data;
    int m_name{ -1 };
    int m_value{ -1 };

    JSON(std::shared_ptr<JSONData> data, int name, int value);

    void getPosition(int& line, int& column, int position) const;

    template<typename T>
    T convert(T(*function)(std::string const&, std::size_t* pos)) const;
};

template<>
bool JSON::as() const;

template<>
std::int32_t JSON::as() const;

template<>
std::int64_t JSON::as() const;

template<>
float JSON::as() const;

template<>
double JSON::as() const;

template<>
std::string JSON::as() const;

class JSONIterator final
{
    friend class JSON;

public:
    JSON const* operator ->() const;
    JSON const& operator *() const;

    bool operator ==(JSONIterator const& that) const;
    bool operator !=(JSONIterator const& that) const;

    JSONIterator& operator++();

private:
    int m_index{ 0 };
    int m_value{ -1 };
    JSON m_current;

    JSONIterator() = default;
    JSONIterator(std::shared_ptr<JSONData> data, int value);
    int skip(int token) const;
};

} // namespace hlib

