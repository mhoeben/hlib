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
#include "hlib/error.hpp"
#include "hlib/config.hpp"
#include "hlib/format.hpp"
#include <sys/socket.h>

using namespace hlib;

//
// Public (Error)
//
Error::Error(std::error_code const& code)
    : m_value(code)
{
}

Error::Error(std::error_code&& code) noexcept
    : m_value(std::move(code))
{
}

Error::Error(std::errc code)
    : Error(std::make_error_code(code))
{
}

Error::Error(std::string const& string)
    : m_value(string)
{
}

Error::Error(std::string&& string) noexcept
    : m_value(std::move(string))
{
}

Error& Error::operator =(std::error_code const& code)
{
    m_value = code;
    return *this;
}

Error& Error::operator =(std::error_code&& code) noexcept
{
    m_value = std::move(code);
    return *this;
}

Error& Error::operator =(std::errc code)
{
    m_value = std::make_error_code(code);
    return *this;
}

Error& Error::operator =(std::string const& string)
{
    m_value = string;
    return *this;
}

Error& Error::operator =(std::string&& string) noexcept
{
    m_value = std::move(string);
    return *this;
}

bool Error::operator == (Error const& that) const noexcept
{
    return that.m_value == m_value;
}

bool Error::operator != (Error const& that) const noexcept
{
    return that.m_value != m_value;
}

bool Error::operator !() const noexcept
{
    return success();
}

Error::operator bool() const noexcept
{
    return failure();
}

Error::Index Error::index() const noexcept
{
    return static_cast<Index>(m_value.index());
}

bool Error::success() const noexcept
{
    return None == index();
}

bool Error::failure() const noexcept
{
    return None != index();
}

Error::Value const& Error::value() const noexcept
{
    return m_value;
}

Error::Value& Error::value() noexcept
{
    return m_value;
}

std::error_code const& Error::code() const
{
    return std::get<Code>(m_value);
}

std::error_code& Error::code()
{
    return std::get<Code>(m_value);
}

std::string const& Error::string() const
{
    return std::get<String>(m_value);
}

std::string& Error::string()
{
    return std::get<String>(m_value);
}

void Error::clear() noexcept
{
    m_value = Value();
}

char const* Error::what() const noexcept
{
    if (String == index()) {
        return std::get<String>(m_value).c_str();
    }

    return "";
}

//
// Public
//
int hlib::get_socket_error(int fd) noexcept
{
    int error;
    socklen_t length = sizeof(error);

    if (-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &length)) {
        return errno;
    }

    return error;
}

std::string hlib::get_error_string(int error_no)
{
    char buffer[Config::maxErrorString()];

    char* string = strerror_r(error_no, buffer, sizeof(buffer));
    return nullptr != string ? std::string(string) : fmt::format("errno {}", error_no);
}

std::string hlib::get_error_string()
{
    return get_error_string(errno);
}

