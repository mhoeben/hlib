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
#include "hlib/error.hpp"
#include <functional>
#include <variant>

namespace hlib
{

template<typename T = std::monostate>
class Result final
{
public:
    typedef T Type;
    typedef std::variant<T, Error> Value;

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

    Result(Error const &error)
        : m_value(std::in_place_index<1>, error)
    {
    }

    Result(Error&& error)
        : m_value(std::in_place_index<1>, std::move(error))
    {
    }

    Result(std::exception_ptr ptr)
        : m_value(std::in_place_index<1>, Error(std::move(ptr)))
    {
    }

    template<typename E, typename = std::enable_if_t<std::is_base_of<std::exception, E>::value>>
    Result(E const& exception)
        : m_value(std::in_place_index<1>, Error(exception))
    {
    }

    template<typename E, typename = std::enable_if_t<std::is_base_of<std::exception, E>::value>>
    Result(E&& exception)
        : m_value(std::in_place_index<1>, Error(std::move(exception)))
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

    Result& operator =(Error const &error)
    {
        m_value = Value(std::in_place_index<1>, error);
        return *this;
    }

    Result& operator =(Error&& error)
    {
        m_value = Value(std::in_place_index<1>, std::move(error));
        return *this;
    }

    Result& operator =(std::exception_ptr ptr)
    {
        m_value = Value(std::in_place_index<1>, Error(std::move(ptr)));
        return *this;
    }

    template<typename E, typename = std::enable_if_t<std::is_base_of<std::exception, E>::value>>
    Result& operator =(E const &exception)
    {
        m_value = Value(std::in_place_index<1>, Error(exception));
        return *this;
    }

    template<typename E, typename = std::enable_if_t<std::is_base_of<std::exception, E>::value>>
    Result& operator =(E&& exception)
    {
        m_value = Value(std::in_place_index<1>, Error(std::move(exception)));
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

    Error const& error() const
    {
        return std::get<1>(m_value);
    }

    Error& error()
    {
        return std::get<1>(m_value);
    }

    template<typename F, typename R = std::invoke_result_t<F, T&&>>
    Result<typename R::Type> then(F&& on_success)
    {
        if (true == failure()) {
            return error();
        }

        return on_success(std::move(value()));
    }

    template<typename F, typename R = std::invoke_result_t<F>>
    std::enable_if_t<std::is_same_v<T, std::monostate>, Result<typename R::Type>>
    then(F&& on_success)
    {
        if (true == failure()) {
            return error();
        }

        return on_success();
    }

    template<typename F>
    Result<T>& otherwise(F&& on_error)
    {
        if (true == success()) {
            return *this;
        }

        on_error(error());
        return *this;
    }

private:
    Value m_value;
};

template<typename T>
inline bool operator ==(bool success, Result<T> const& result)
{
    return result.success() == success;
}

template<typename T>
inline bool operator !=(bool success, Result<T> const& result)
{
    return result.success() != success;
}

template<typename T>
inline bool operator ==(Result<T> const& result, bool success)
{
    return result.success() == success;
}

template<typename T>
inline bool operator !=(Result<T> const& result, bool success)
{
    return result.success() != success;
}

template<typename T>
T check(Result<T> result, std::function<void(Error&& error)> on_error)
{
    if (true == result.success()) {
        return std::move(result.value());
    }

    on_error(std::move(result.error()));
    std::terminate();
}

template<typename T>
T check(Result<T> result, std::function<T(Error&& error)> on_error)
{
    if (true == result.success()) {
        return std::move(result.value());
    }

    return on_error(std::move(result.error()));
}

inline std::function<void(Error&&)> set_error(Error& error)
{
    return [&error](Error&& e) noexcept {
        error = std::move(e);
        return;
    };
}

template<typename T>
std::function<T(Error&&)> set_error(Error& error)
{
    return [&error](Error&& e) noexcept {
        error = std::move(e);
        return T();
    };
}

template<typename T>
std::function<T(Error&&)> set_error(Error& error, T default_value)
{
    return [&error, default_value = std::move(default_value)](Error&& e) noexcept {
        error = std::move(e);
        return std::move(default_value);
    };
}

[[noreturn]] inline void toss_error(Error const& error)
{
    error.toss();
}

template<typename T = void>
T success_or_throw(Result<T> result)
{
    return check(std::move(result), [](Error&& error) -> void { throw std::move(error); });
}

template <typename F, typename... Args>
auto attempt(F&& func, Args&&... args) -> Result<decltype(std::invoke(std::forward<F>(func), std::forward<Args>(args)...))> {
    try {
        return std::invoke(std::forward<F>(func), std::forward<Args>(args)...);
    } catch (...) {
        return std::current_exception();
    }
}

} // namespace hlib

