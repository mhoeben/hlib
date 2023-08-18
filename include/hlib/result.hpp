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
        FailCode,
        FailString
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
            m_value = Value(std::in_place_index<FailCode>, std::get<Error::Code>(value));
            break;

        case Error::String:
            m_value = Value(std::in_place_index<FailString>, std::get<Error::String>(value));
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
            m_value = Value(std::in_place_index<FailCode>, std::move(std::get<Error::Code>(value)));
            break;

        case Error::String:
            m_value = Value(std::in_place_index<FailString>, std::move(std::get<Error::String>(value)));
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
            m_value = Value(std::in_place_index<FailCode>, std::get<Error::Code>(value));
            break;

        case Error::String:
            m_value = Value(std::in_place_index<FailString>, std::get<Error::String>(value));
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
            m_value = Value(std::in_place_index<FailCode>, std::move(std::get<Error::Code>(value)));
            break;

        case Error::String:
            m_value = Value(std::in_place_index<FailString>, std::move(std::get<Error::String>(value)));
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
        return fail();
    }

    operator T const&() const
    {
        return std::get<Success>(m_value);
    }

    operator T&()
    {
        return std::get<Success>(m_value);
    }

    operator std::error_code const&() const
    {
        return std::get<FailCode>(m_value);
    }

    operator std::error_code&()
    {
        return std::get<FailCode>(m_value);
    }

    template<typename U, typename = std::enable_if_t<std::is_same<std::string, U>::value
                                                  && !std::is_same<T, U>::value>>
    operator U const&() const
    {
        return std::get<FailString>(m_value);
    }

    template<typename U, typename = std::enable_if_t<std::is_same<std::string, U>::value
                                                  && !std::is_same<T, U>::value>>
    operator U&()
    {
        return std::get<FailString>(m_value);
    }

    Index index() const noexcept
    {
        return static_cast<Index>(m_value.index());
    }

    bool success() const noexcept
    {
        return Success == index();
    }

    bool fail() const noexcept
    {
        return Success != index();
    }

    T const& value() const noexcept
    {
        return std::get<Success>(m_value);
    }

    T& value() noexcept
    {
        return std::get<Success>(m_value);
    }

    Error getError() const
    {
        switch (index()) {
        case Success: return Error();
        case FailCode: return Error(std::get<FailCode>(m_value));
        case FailString: return Error(std::get<FailString>(m_value));
        default:
            throw std::logic_error("getError()");
        }
    }

private:
    std::variant<T, std::error_code, std::string> m_value;
};

template<typename T, typename U>
T as(Result<U> const& result)
{
    return static_cast<T const&>(result);
}

template<typename T, typename U>
T as(Result<U>& result)
{
    return static_cast<T&>(result);
}

template<typename T, typename U>
typename std::enable_if<std::is_same<Error, T>::value, T>::type
to(Result<U> const& result)
{
    return result.getError();
}

} // namespace hlib

