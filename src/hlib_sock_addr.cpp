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
#include "hlib/sock_addr.hpp"
#include "hlib/error.hpp"
#include "hlib/format.hpp"
#include "hlib/string.hpp"
#include <arpa/inet.h>

using namespace hlib;

//
// Public
//
SockAddr::SockAddr() noexcept
{
    m_family = AF_UNSPEC;
}

SockAddr::SockAddr(SockAddr const& that) noexcept
{
    m_storage = that.m_storage;
}

SockAddr::SockAddr(SockAddr&& that) noexcept
{
    memcpy(&m_storage, &that.m_storage, sizeof(sockaddr_storage));
    that.m_family = AF_UNSPEC;
}

SockAddr::SockAddr(sockaddr_in const& that) noexcept
{
    assert(AF_INET == that.sin_family);
    m_inet = that;
} 

SockAddr::SockAddr(sockaddr_in6 const& that) noexcept
{
    assert(AF_INET6 == that.sin6_family);
    m_inet6 = that;
}

SockAddr::SockAddr(sockaddr_un const& that) noexcept
{
    assert(AF_UNIX == that.sun_family);
    m_unix = that;
} 

SockAddr::SockAddr(sockaddr_storage const& that) noexcept
{
    m_storage = that;
}

SockAddr::SockAddr(std::string const& that)
{
    parse(that);
}

SockAddr& SockAddr::operator = (SockAddr&& that) noexcept
{
    memcpy(&m_storage, &that.m_storage, sizeof(sockaddr_storage));
    that.m_family = AF_UNSPEC;
    return *this;
}

SockAddr& SockAddr::operator = (SockAddr const& that) noexcept
{
    m_storage = that.m_storage;
    return *this;
}

SockAddr& SockAddr::operator = (sockaddr_in const& that) noexcept
{
    assert(AF_INET == that.sin_family);
    m_inet = that;
    return *this;
}

SockAddr& SockAddr::operator = (sockaddr_in6 const& that) noexcept
{
    assert(AF_INET6 == that.sin6_family);
    m_inet6 = that;
    return *this;
}

SockAddr& SockAddr::operator = (sockaddr_un const& that) noexcept
{
    assert(AF_UNIX == that.sun_family);
    m_unix = that;
    return *this;
}

SockAddr& SockAddr::operator = (sockaddr_storage const& that) noexcept
{
    m_storage = that;
    return *this;
}

SockAddr& SockAddr::operator = (std::string const& that) noexcept
{
    parse(that);
    return *this;
}

sa_family_t SockAddr::family() const noexcept
{
    return m_family;
}

std::size_t SockAddr::length() const noexcept
{
    switch (m_family) {
    case AF_INET:   return sizeof(sockaddr_in);
    case AF_INET6:  return sizeof(sockaddr_in6);
    case AF_UNIX:   return sizeof(sockaddr_un);
    default:
        return sizeof(sockaddr_storage);
    }
}

uint16_t SockAddr::port() const noexcept
{
    switch (m_family) {
    case AF_INET: return ntohs(m_inet.sin_port);
    case AF_INET6: return ntohs(m_inet6.sin6_port);
    default:
        return 0;
    }
}

std::string SockAddr::address() const
{
    static_assert(INET6_ADDRSTRLEN >= INET_ADDRSTRLEN, "INET6_ADDRSTRLEN < INET_ADDRSTRLEN");
    char address[INET6_ADDRSTRLEN];

    switch (m_family) {
    case AF_INET:
        if (nullptr == inet_ntop(AF_INET, &m_inet.sin_addr, address, INET_ADDRSTRLEN)) {
            throwf<std::runtime_error>("inet_ntop() failed ({})", get_error_string());
        }
        return address;

    case AF_INET6:
        if (nullptr == inet_ntop(AF_INET6, &m_inet6.sin6_addr, address, INET6_ADDRSTRLEN)) {
            throwf<std::runtime_error>("inet_ntop() failed ({})", get_error_string());
        }
        return address;

    case AF_UNIX:
        // TODO verify that it is always zero terminated!
        return m_unix.sun_path;

    default:
        return std::string();
    }
}

bool SockAddr::empty() const noexcept
{
    return AF_UNSPEC == m_family;
}

SockAddr::operator sockaddr const* () const noexcept
{
    return reinterpret_cast<sockaddr const*>(&m_storage);
}

SockAddr::operator sockaddr const* () noexcept
{
    return reinterpret_cast<sockaddr const*>(&m_storage);
}

SockAddr::operator sockaddr* () noexcept
{
    return reinterpret_cast<sockaddr*>(&m_storage);
}

SockAddr::operator sockaddr_in const*() const noexcept
{
    assert(AF_INET == m_family);
    return &m_inet;
}

SockAddr::operator sockaddr_in const*() noexcept
{
    assert(AF_INET == m_family);
    return &m_inet;
}

SockAddr::operator sockaddr_in*() noexcept
{
    assert(AF_INET == m_family);
    return &m_inet;
}

SockAddr::operator sockaddr_in6 const*() const noexcept
{
    assert(AF_INET6 == m_family);
    return &m_inet6;
}

SockAddr::operator sockaddr_in6 const*() noexcept
{
    assert(AF_INET6 == m_family);
    return &m_inet6;
}

SockAddr::operator sockaddr_in6*() noexcept
{
    assert(AF_INET6 == m_family);
    return &m_inet6;
}

SockAddr::operator sockaddr_un const*() const noexcept
{
    assert(AF_UNIX == m_family);
    return &m_unix;
}

SockAddr::operator sockaddr_un const*() noexcept
{
    assert(AF_UNIX == m_family);
    return &m_unix;
}

SockAddr::operator sockaddr_un*() noexcept
{
    assert(AF_UNIX == m_family);
    return &m_unix;
}

bool SockAddr::setPort(uint16_t port, std::nothrow_t) noexcept
{
    switch (m_family) {
    case AF_INET:
        m_inet.sin_port = htons(port);
        break;

    case AF_INET6:
        m_inet6.sin6_port = htons(port);
        break;

    default:
        return false;
    }

    return true;
}

void SockAddr::setPort(uint16_t port)
{
    if (false == setPort(port, std::nothrow)) {
        throw std::runtime_error("Not a network address");
    }
}

bool SockAddr::parse(std::string const& string, std::nothrow_t) noexcept
{
    std::optional<std::uint16_t> port;

    auto to_unix = [&]()
    {
        if (string.length() + 1 > sizeof(sockaddr_un::sun_path)) {
            return false;
        }

        m_unix.sun_family = AF_UNIX;

        // Use memcpy to also support paths for the Linux proprietary
        // abstract socket namespace.
        memcpy(m_unix.sun_path, string.data(), string.length());
        m_unix.sun_path[string.length()] = 0;
        return true;
    };

    m_family = AF_UNSPEC;

    // An IPv4 address with optional port has 3 '.' characters.
    if (3 == count(string, '.')) {
        in_addr addr;

        // Find optional port.
        std::string::size_type pos = string.rfind(':');

        // Parse IPv4 address.
        if (1 != inet_pton(AF_INET, string.substr(0, pos).c_str(), &addr)) {
            return to_unix();
        }
        m_inet.sin_family = AF_INET;
        m_inet.sin_addr = addr;
        m_inet.sin_port = 0;

        // Port available?
        if (std::string::npos != pos) {
            port = stoui16(string.substr(pos + 1), 10, std::nothrow);
            if (std::nullopt == port) {
                return to_unix();
            }
            m_inet.sin_port = htons(port.value());
        }
        return true;
    }
    // Else try to parse it as a IPv6 address with optional port.
    else {
        in6_addr addr;

        std::string::size_type start = starts_with(string, '[');
        std::string::size_type end = string.rfind("]:");

        // String either has "[<ipv6>]:port" format or just "<ipv6>".
        if ((1 == start && std::string::npos == end)
         || (0 == start && std::string::npos != end)) {
            return to_unix();
        }

        // Parse IPv6 address.
        if (1 != inet_pton(AF_INET6, string.substr(start, end).c_str(), &addr)) {
            return to_unix();
        }
        m_inet6.sin6_family = AF_INET6;
        m_inet6.sin6_addr = addr;
        m_inet6.sin6_port = 0;

        // Port available?
        if (std::string::npos != end) {
            port = stoui16(string.substr(end + 2), 10, std::nothrow);
            if (std::nullopt == port) {
                return to_unix();
            }
            m_inet6.sin6_port = htons(port.value());
        }
        return true;
    }
}

void SockAddr::parse(std::string const& string)
{
    if (false == parse(string, std::nothrow)) {
        throw std::runtime_error("String too long");
    }
}

std::string hlib::to_string(SockAddr const& sa)
{
    std::string address = sa.address();
    int port = sa.port();

    switch (sa.family()) {
    case AF_INET:
        if (port > 0) {
            return fmt::format("{}:{}", address, port);
        }
        break;

    case AF_INET6:
        if (port > 0) {
            return fmt::format("[{}]:{}", address, port);
        }
        break;

    default:
        break;
    }

    return address;
}

