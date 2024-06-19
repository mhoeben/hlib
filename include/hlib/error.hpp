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
#include <errno.h>
#include <stdexcept>
#include <string>
#include <system_error>
#include <variant>

namespace hlib
{

class Error final
{
public:
    typedef std::variant<
        std::monostate,
        std::exception,
            std::logic_error,
                std::invalid_argument,
                std::domain_error,
                std::length_error,
                std::out_of_range,
            std::runtime_error,
                std::range_error,
                std::overflow_error,
                std::underflow_error,
                std::system_error,
            std::bad_alloc
    > Value;

public:
    Error() = default;

    template<typename T, typename = std::enable_if_t<std::is_base_of<std::exception, T>::value>>
    Error(T const& value)
        : m_value(value)
    {
    }

    template<typename T, typename = std::enable_if_t<std::is_base_of<std::exception, T>::value>>
    Error(T&& value)
        : m_value(std::move(value))
    {
    }

    template<typename T, typename = std::enable_if_t<std::is_base_of<std::exception, T>::value>>
    Error& operator =(T const& value)
    {
        m_value = value;
        return *this;
    }

    template<typename T, typename = std::enable_if_t<std::is_base_of<std::exception, T>::value>>
    Error& operator =(T&& value)
    {
        m_value = std::move(value);
        return *this;
    }

    bool empty() const noexcept;

    std::error_code code() const;
    std::string what() const;

    [[noreturn]] void toss() const;

private:
    Value m_value;
};

inline bool operator ==(bool is_set, Error const& error)
{
    return error.empty() != is_set;
}

inline bool operator !=(bool is_set, Error const& error)
{
    return error.empty() == is_set;
}

inline bool operator ==(Error const& error, bool is_set)
{
    return error.empty() != is_set;
}

inline bool operator !=(Error const& error, bool is_set)
{
    return error.empty() == is_set;
}

int get_socket_error(int fd) noexcept;

std::string get_error_string(int error_no);
std::string get_error_string();

std::error_code make_error_code(int posix_errno);

std::system_error make_system_error(int posix_errno);
std::system_error make_system_error(int posix_errno, std::string const& what);
std::system_error make_system_error(int posix_errno, std::error_category& category, std::string const& what);

} // namespace hlib

