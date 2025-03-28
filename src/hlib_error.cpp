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
#include <cstring>
#include <sys/socket.h>

using namespace hlib;

namespace
{

constexpr std::size_t max_error_string = 256;

} // namespace

//
// Public (Error)
//
bool Error::empty() const noexcept
{
    return !m_exception;
}

std::error_code Error::code() const
{
    try {
        if (empty()) {
            return {};
        }

        std::rethrow_exception(m_exception);
    }
    catch (std::system_error const& e) {
        return e.code();
    }
    catch (...) {
        return {};
    }

}

std::string Error::what() const
{
    try {
        if (empty()) {
            return {};
        }

        std::rethrow_exception(m_exception);
    }
    catch (std::exception const& e) {
        return e.what();
    }
    catch (...) {
        return {};
    }
}

[[noreturn]] void Error::toss() const
{
    if (m_exception) std::rethrow_exception(m_exception);
    throw std::logic_error("No exception stored");
}

//
// Public
//
std::string hlib::get_error_string(int error_no)
{
    char buffer[max_error_string];

    char* string = strerror_r(error_no, buffer, sizeof(buffer));
    return nullptr != string ? std::string(string) : "errno " + std::to_string(error_no);
}

std::string hlib::get_error_string()
{
    return get_error_string(errno);
}

std::error_code hlib::make_error_code(int posix_errno)
{
    return std::make_error_code(static_cast<std::errc>(posix_errno));
}

std::system_error hlib::make_system_error(int posix_errno)
{
    return std::system_error(make_error_code(posix_errno));
}

std::system_error hlib::make_system_error(int posix_errno, std::string const& what)
{
    return std::system_error(make_error_code(posix_errno), what);
}

std::system_error hlib::make_system_error(int posix_errno, std::error_category& category, std::string const& what)
{
    return std::system_error(posix_errno, category, what);
}

