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
#include <sys/un.h>
#include <string>

namespace hlib
{

class SockAddr final
{
public:
    SockAddr();

    explicit SockAddr(SockAddr const& that);
    explicit SockAddr(SockAddr&& that);

    explicit SockAddr(sockaddr_in const& that);
    explicit SockAddr(sockaddr_in6 const& that);
    explicit SockAddr(sockaddr_un const& that);
    explicit SockAddr(sockaddr_storage const& that);

    SockAddr(std::string const& that);

    SockAddr& operator = (SockAddr&& that);

    SockAddr& operator = (SockAddr const& that);
    SockAddr& operator = (sockaddr_in const& that);
    SockAddr& operator = (sockaddr_in6 const& that);
    SockAddr& operator = (sockaddr_un const& that);
    SockAddr& operator = (sockaddr_storage const& that);
    SockAddr& operator = (std::string const& that);

    sa_family_t family() const;
    std::size_t length() const;
    int port() const;
    std::string address() const;

    explicit operator sockaddr const* () const;
    explicit operator sockaddr* ();

    explicit operator sockaddr_in const*() const;
    explicit operator sockaddr_in*();

    explicit operator sockaddr_in6 const*() const;
    explicit operator sockaddr_in6*();

    explicit operator sockaddr_un const*() const;
    explicit operator sockaddr_un*();

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
T const* as(SockAddr const& sa)
{
    return static_cast<T const*>(sa);
}

template<typename T>
T* as(SockAddr& sa)
{
    return static_cast<T*>(sa);
}

} // namespace hlib

