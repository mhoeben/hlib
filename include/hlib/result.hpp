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

#include <system_error>
#include <variant>

namespace hlib
{

template<typename T = std::monostate>
class Result final
{
public:
    typedef T Type;
    typedef std::variant<T, std::system_error> Value;

public:
    Result() = default;

    Result(T const& value)
        : m_value(std::in_place_index<0>, value)
    {
    }

    Result(T&& value)
        : m_value(std::in_place_index<0>, std::move(value))
    {
    }

    Result(std::system_error const &error)
        : m_value(std::in_place_index<1>, error)
    {
    }

    Result(std::system_error&& error)
        : m_value(std::in_place_index<1>, std::move(error))
    {
    }

    Result& operator =(T const& value) const
    {
        m_value = Value(std::in_place_index<0>, value);
        return *this;
    }

    Result& operator =(T&& value)
    {
        m_value = Value(std::in_place_index<0>, std::move(value));
        return *this;
    }

    Result& operator =(std::system_error const &error)
    {
        m_value = Value(std::in_place_index<1>, error);
        return *this;
    }

    Result& operator =(std::system_error&& error)
    {
        m_value = Value(std::in_place_index<1>, std::move(error));
        return *this;
    }

    explicit operator bool() const noexcept
    {
        return success();
    }

    bool operator !() const noexcept
    {
        return failure();
    }

    T& operator *()
    {
        return std::get<0>(m_value);
    }

    T& operator ->()
    {
        return std::get<0>(m_value);
    }

    bool success() const noexcept
    {
        return 0 == m_value.index();
    }

    bool failure() const noexcept
    {
        return 1 == m_value.index();
    }

    T const& value() const
    {
        return std::get<0>(m_value);
    }

    T& value()
    {
        return std::get<0>(m_value);
    }

    std::system_error const& error() const
    {
        return std::get<1>(m_value);
    }

    std::system_error& error()
    {
        return std::get<1>(m_value);
    }

private:
    Value m_value;
};

template<typename T = void>
T throw_or_value(Result<T> result)
{
    if (true == result.failure()) {
        throw result.error();
    }

    return std::move(result.value());
}

} // namespace hlib

