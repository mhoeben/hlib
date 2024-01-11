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

