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

class JSON final
{
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

public:
    JSON();
    JSON(JSON&& that) noexcept;

    JSON& operator =(JSON&& that) noexcept;

    bool empty() const noexcept;
    Type type() const noexcept;
    std::string name() const;
    std::string value() const;

    template<typename T>
    T as() const;

    JSON operator[](int index) const;
    JSON operator[](std::string const& name) const;

    JSON at(int index) const;
    JSON at(std::string const& name) const;

    void clear();
    void parse(std::string string);

private:
    struct Data;
    std::shared_ptr<Data> m_data;
    int m_name{ -1 };
    int m_value{ -1 };

    JSON(std::shared_ptr<Data> data, int name, int value);

    template<typename T>
    T convert(T(*function)(std::string const&, size_t* pos)) const;

    int skip(int token) const;
};

template<>
bool JSON::as() const;

template<>
int32_t JSON::as() const;

template<>
int64_t JSON::as() const;

template<>
float JSON::as() const;

template<>
double JSON::as() const;

template<>
std::string JSON::as() const;

} // namespace hlib

