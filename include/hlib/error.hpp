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

#include <errno.h>
#include <string>
#include <system_error>
#include <variant>

namespace hlib
{

class Error final
{
public:
    enum Index
    {
        None,
        Code,
        String
    };

    typedef std::variant<std::monostate, std::error_code, std::string> Value;

public:
    Error() = default;
    Error(std::error_code code);
    Error(std::string string);

    Error& operator =(std::error_code const& code);
    Error& operator =(std::error_code&& code);
    Error& operator =(std::string const& string);
    Error& operator =(std::string&& string);

    bool operator == (Error const& that) const noexcept;
    bool operator != (Error const& that) const noexcept;

    bool operator !() const noexcept;
    operator bool() const noexcept;

    explicit operator std::error_code const&() const;
    explicit operator std::error_code&();
    explicit operator std::string const&() const;
    explicit operator std::string&();

    Index index() const noexcept;
    bool success() const noexcept;
    bool failure() const noexcept;

    Value const& value() const noexcept;
    Value& value() noexcept;

    std::error_code const& code() const;
    std::error_code& code();

    std::string const& string() const;
    std::string& string();

    void clear() noexcept;

private:
    Value m_value;
};

template<typename T>
T const& as(Error const& error)
{
    return static_cast<T const&>(error);
}

template<typename T>
T& as(Error& error)
{
    return static_cast<T&>(error);
}

int get_socket_error(int fd) noexcept;

std::string get_error_string(int error_no);
std::string get_error_string();

} // namespace hlib

