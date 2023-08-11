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
#include <netinet/in.h>
#include <new>
#include <sys/un.h>
#include <string>

namespace hlib
{

class SockAddr final
{
public:
    SockAddr() noexcept;

    explicit SockAddr(SockAddr const& that) noexcept;
    explicit SockAddr(SockAddr&& that) noexcept;

    explicit SockAddr(sockaddr_in const& that) noexcept;
    explicit SockAddr(sockaddr_in6 const& that) noexcept;
    explicit SockAddr(sockaddr_un const& that) noexcept;
    explicit SockAddr(sockaddr_storage const& that) noexcept;

     SockAddr(std::string const& that);

    SockAddr& operator = (SockAddr&& that) noexcept;

    SockAddr& operator = (SockAddr const& that) noexcept;
    SockAddr& operator = (sockaddr_in const& that) noexcept;
    SockAddr& operator = (sockaddr_in6 const& that) noexcept;
    SockAddr& operator = (sockaddr_un const& that) noexcept;
    SockAddr& operator = (sockaddr_storage const& that) noexcept;
    SockAddr& operator = (std::string const& that) noexcept;

    sa_family_t family() const noexcept;
    std::size_t length() const noexcept;
    std::uint16_t port() const noexcept;
    std::string address() const;
    bool empty() const noexcept;

    explicit operator sockaddr const* () const noexcept;
    explicit operator sockaddr* () noexcept;

    explicit operator sockaddr_in const*() const noexcept;
    explicit operator sockaddr_in*() noexcept;

    explicit operator sockaddr_in6 const*() const noexcept;
    explicit operator sockaddr_in6*() noexcept;

    explicit operator sockaddr_un const*() const noexcept;
    explicit operator sockaddr_un*() noexcept;

    bool setPort(uint16_t port, std::nothrow_t) noexcept;
    void setPort(uint16_t port);

    bool parse(std::string const& string, std::nothrow_t) noexcept;
    void parse(std::string const& string);

private:
    union
    {
        sa_family_t     m_family;
        sockaddr_in     m_inet;
        sockaddr_in6    m_inet6;
        sockaddr_un     m_unix;
        sockaddr_storage m_storage;
    };
};

std::string to_string(SockAddr const& sa);

template<typename T>
T const* as(SockAddr const& sa) noexcept
{
    return static_cast<T const*>(sa);
}

template<typename T>
T* as(SockAddr& sa) noexcept
{
    return static_cast<T*>(sa);
}

} // namespace hlib

