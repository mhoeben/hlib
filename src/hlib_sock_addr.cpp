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
#include "tao/pegtl.hpp"
#include <arpa/inet.h>

using namespace hlib;

//
// Implementation
//
namespace grammar
{

struct Result
{
    sa_family_t family{ AF_UNSPEC };
    std::string address;
    int port{ 0 };
};

using namespace tao::pegtl;

struct address_ipv4         : plus<sor<digit, one<'.'>>> {};
struct address_ipv6         : plus<sor<xdigit, one<':'>, one<'.'>>> {};
struct port                 : rep_min_max<1, 5, digit> {};

struct address              : sor<address_ipv4, address_ipv6> {};
struct address_and_port_1   : seq<address_ipv4, one<':'>, port> {};
struct address_and_port_2   : seq<one<'['>, address, one<']'>, opt<seq<one<':'>, port>>> {};

struct address_and_port     : must<sor<address_and_port_1, address_and_port_2, address>, eof> {};

template<typename RULE>
struct action
{
};

template<>
struct action<address_ipv4>
{
    template<typename INPUT>
    static void apply(INPUT const& input, Result& result)
    {
        result.family = AF_INET;
        result.address = input.string();
    }
};

template<>
struct action<address_ipv6>
{
    template<typename INPUT>
    static void apply(INPUT const& input, Result& result)
    {
        result.family = AF_INET6;
        result.address = input.string();
    }
};

template<>
struct action<port>
{
    template<typename INPUT>
    static void apply(INPUT const& input, Result& result)
    {
        result.port = std::stoi(input.string());
    }
};

} // namespace

void SockAddr::parse(std::string const& string)
{
    using namespace tao;

    auto to_unix = [&]()
    {
        hcheck(string.length() + 1 <= sizeof(sockaddr_un::sun_path));

        m_unix.sun_family = AF_UNIX;

        // Use memcpy to also support paths for the Linux proprietary
        // abstract socket namespace.
        memcpy(m_unix.sun_path, string.data(), string.length());
        m_unix.sun_path[string.length()] = 0;
    };

    m_family = AF_UNSPEC;

    try {
        grammar::Result result;

        pegtl::memory_input<> input(string, "");
        pegtl::parse<grammar::address_and_port, grammar::action>(input, result);

        switch (result.family) {
        case AF_INET:
          {
            in_addr addr;

            if (1 != inet_pton(AF_INET, result.address.c_str(), &addr)) {
                return to_unix();
            }

            m_inet.sin_family = AF_INET;
            m_inet.sin_addr = addr;
            m_inet.sin_port = htons(result.port);
            break;
          }
        case AF_INET6:
          {
            in6_addr addr;

            if (1 != inet_pton(AF_INET6, result.address.c_str(), &addr)) {
                return to_unix();
            }

            m_inet6.sin6_family = AF_INET6;
            m_inet6.sin6_addr = addr;
            m_inet6.sin6_port = htons(result.port);
            break;
          }
        default:
            assert(false);
            break;
        }
    }
    catch (pegtl::parse_error const& e) {
        return to_unix();
    }
}

//
// Public
//
SockAddr::SockAddr()
{
    m_family = AF_UNSPEC;
}

SockAddr::SockAddr(SockAddr const& that)
{
    m_storage = that.m_storage;
}

SockAddr::SockAddr(sockaddr_in const& that)
{
    assert(AF_INET == that.sin_family);
    m_inet = that;
} 

SockAddr::SockAddr(sockaddr_in6 const& that)
{
    assert(AF_INET6 == that.sin6_family);
    m_inet6 = that;
}

SockAddr::SockAddr(sockaddr_un const& that)
{
    assert(AF_UNIX == that.sun_family);
    m_unix = that;
} 

SockAddr::SockAddr(sockaddr_storage const& that)
{
    m_storage = that;
}

SockAddr::SockAddr(std::string const& that)
{
    parse(that);
}

SockAddr& SockAddr::operator = (SockAddr const& that)
{
    m_storage = that.m_storage;
    return *this;
}

SockAddr& SockAddr::operator = (sockaddr_in const& that)
{
    assert(AF_INET == that.sin_family);
    m_inet = that;
    return *this;
}

SockAddr& SockAddr::operator = (sockaddr_in6 const& that)
{
    assert(AF_INET6 == that.sin6_family);
    m_inet6 = that;
    return *this;
}

SockAddr& SockAddr::operator = (sockaddr_un const& that)
{
    assert(AF_UNIX == that.sun_family);
    m_unix = that;
    return *this;
}

SockAddr& SockAddr::operator = (sockaddr_storage const& that)
{
    m_storage = that;
    return *this;
}

SockAddr& SockAddr::operator = (std::string const& that)
{
    parse(that);
    return *this;
}

sa_family_t SockAddr::family() const
{
    return m_family;
}

int SockAddr::port() const
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
            throwf<std::runtime_error>("SockAddr::toString: inet_ntop failed ({})", get_error_string(errno));
        }
        return address;

    case AF_INET6:
        if (nullptr == inet_ntop(AF_INET6, &m_inet6.sin6_addr, address, INET6_ADDRSTRLEN)) {
            throwf<std::runtime_error>("SockAddr::toString: inet_ntop failed ({})", get_error_string(errno));
        }
        return address;

    case AF_UNIX:
        // TODO verify that it is always zero terminated!
        return m_unix.sun_path;

    default:
        return std::string();
    }
}

SockAddr::operator sockaddr const* () const
{
    return reinterpret_cast<sockaddr const*>(&m_storage);
}

SockAddr::operator sockaddr* ()
{
    return reinterpret_cast<sockaddr*>(&m_storage);
}

SockAddr::operator sockaddr_in const*() const
{
    assert(AF_INET == m_family);
    return &m_inet;
}

SockAddr::operator sockaddr_in*()
{
    assert(AF_INET == m_family);
    return &m_inet;
}

SockAddr::operator sockaddr_in6 const*() const
{
    assert(AF_INET6 == m_family);
    return &m_inet6;
}

SockAddr::operator sockaddr_in6*()
{
    assert(AF_INET6 == m_family);
    return &m_inet6;
}

SockAddr::operator sockaddr_un const*() const
{
    assert(AF_UNIX == m_family);
    return &m_unix;
}

SockAddr::operator sockaddr_un*()
{
    assert(AF_UNIX == m_family);
    return &m_unix;
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

