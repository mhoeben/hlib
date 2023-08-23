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

#include "hlib/error.hpp"

namespace hlib
{

template<typename T = std::monostate>
class Result final
{
public:
    enum Index
    {
        Success,
        ErrorCode,
        ErrorString
    };
    typedef T Type;
    typedef std::variant<T, std::error_code, std::string> Value;

public:
    Result() = default;

    Result(T const& value)
        : m_value(std::in_place_index<Success>, value)
    {
    }

    Result(T&& value)
        : m_value(std::in_place_index<Success>, std::move(value))
    {
    }

    Result(Error const &error)
    {
        Error::Value const& value = error.value();

        switch (value.index()) {
        case Error::None:
            break;

        case Error::Code:
            m_value = Value(std::in_place_index<ErrorCode>, std::get<Error::Code>(value));
            break;

        case Error::String:
            m_value = Value(std::in_place_index<ErrorString>, std::get<Error::String>(value));
            break;

        default:
            assert(false);
            break;
        }
    }

    Result(Error&& error)
    {
        Error::Value& value = error.value();

        switch (value.index()) {
        case Error::None:
            break;

        case Error::Code:
            m_value = Value(std::in_place_index<ErrorCode>, std::move(std::get<Error::Code>(value)));
            break;

        case Error::String:
            m_value = Value(std::in_place_index<ErrorString>, std::move(std::get<Error::String>(value)));
            break;

        default:
            assert(false);
            break;
        }
    }

    Result& operator =(T const& value) const
    {
        m_value = Value(std::in_place_index<Success>, value);
        return *this;
    }

    Result& operator =(T&& value)
    {
        m_value = Value(std::in_place_index<Success>, std::move(value));
        return *this;
    }

    Result& operator =(Error const &error)
    {
        Error::Value const& value = error.value();

        switch (value.index()) {
        case Error::None:
            break;

        case Error::Code:
            m_value = Value(std::in_place_index<ErrorCode>, std::get<Error::Code>(value));
            break;

        case Error::String:
            m_value = Value(std::in_place_index<ErrorString>, std::get<Error::String>(value));
            break;

        default:
            assert(false);
            break;
        }

        return *this;
    }

    Result& operator =(Error&& error)
    {
        Error::Value& value = error.value();

        switch (value.index()) {
        case Error::None:
            break;

        case Error::Code:
            m_value = Value(std::in_place_index<ErrorCode>, std::move(std::get<Error::Code>(value)));
            break;

        case Error::String:
            m_value = Value(std::in_place_index<ErrorString>, std::move(std::get<Error::String>(value)));
            break;

        default:
            assert(false);
            break;
        }

        return *this;
    }

    operator bool() const noexcept
    {
        return success();
    }

    bool operator !() const noexcept
    {
        return failure();
    }

    Index index() const noexcept
    {
        return static_cast<Index>(m_value.index());
    }

    bool success() const noexcept
    {
        return Success == index();
    }

    bool failure() const noexcept
    {
        return Success != index();
    }

    T const& value() const
    {
        return std::get<Success>(m_value);
    }

    T& value()
    {
        return std::get<Success>(m_value);
    }

    std::error_code const& errorCode() const
    {
        return std::get<ErrorCode>(m_value);
    }

    std::string const& errorString() const
    {
        return std::get<ErrorString>(m_value);
    }

    Error error() const
    {
        switch (index()) {
        case Success: return Error();
        case ErrorCode: return errorCode();
        case ErrorString: return errorString();
        default:
            throw std::logic_error("error()");
        }
    }

private:
    std::variant<T, std::error_code, std::string> m_value;
};

template<typename T, typename U>
typename std::enable_if<std::is_same<Error, T>::value, T>::type
to(Result<U> const& result)
{
    return result.error();
}

} // namespace hlib

