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
#include "hlib/uri.hpp"
#include "hlib/base.hpp"
#include "hlib/format.hpp"
#include "tao/pegtl.hpp"
#include "tao/pegtl/contrib/uri.hpp"
#include <stdlib.h>
#include <string.h>
#include <unordered_map>

using namespace hlib;

//
// Implementation
//
namespace
{

namespace pegtl = tao::pegtl;

template<std::string URI::*Field>
struct bind
{
    template<typename INPUT>
    static void apply(INPUT const& input, URI& uri)
    {
       uri.*Field = input.string();
    }
};


struct bind_user_info
{
    template<typename INPUT>
    static void apply(INPUT const& input, URI& uri)
    {
        uri.user_info = input.string();
    }
};

struct bind_port
{
    template<typename INPUT>
    static void apply(INPUT const& input, URI& uri)
    {
        if (false == input.empty()) {
            uint32_t port = std::stoul(input.string());
            if (port > 65536) {
                throw std::out_of_range("port out of range");
            }

            uri.port = static_cast<uint16_t>(port);
        }
        else {
            uri.port = uri_get_default_port_for_scheme(uri.scheme);
        }
    }
};

template< typename Rule > struct action {};

template<> struct action<pegtl::uri::scheme>        : bind<&URI::scheme> {};
template<> struct action<pegtl::uri::opt_userinfo>  : bind_user_info {};
template<> struct action<pegtl::uri::host>          : bind<&URI::host> {};
template<> struct action<pegtl::uri::port>          : bind_port {};
template<> struct action<pegtl::uri::path_noscheme> : bind<&URI::path> {};
template<> struct action<pegtl::uri::path_rootless> : bind<&URI::path> {};
template<> struct action<pegtl::uri::path_absolute> : bind<&URI::path> {};
template<> struct action<pegtl::uri::path_abempty>  : bind<&URI::path> {};
template<> struct action<pegtl::uri::query>         : bind<&URI::query> {};
template<> struct action<pegtl::uri::fragment>      : bind<&URI::fragment> {};

}

//
// Public
//
URI::URI(std::string const& that)
{
    uri_parse(that);
}

URI hlib::uri_parse(std::string const& string)
{
    URI uri;

    namespace pegtl = tao::pegtl;
    using grammar = pegtl::must<pegtl::uri::URI_reference>;

    pegtl::memory_input<> input(string, "uri");
    pegtl::parse<grammar, action>(input, uri);

    if (uri.port == 0) {
        uri.port = uri_get_default_port_for_scheme(uri.scheme);
    }

    return uri;
}

std::string hlib::to_string(URI const& uri)
{
    size_t const length = uri.scheme.length() + 3
                        + uri.user_info.length() + 1
                        + uri.host.length() + 6
                        + uri.path.length() + 1
                        + uri.query.length() + 1
                        + uri.fragment.length() + 1;

    fmt::memory_buffer buffer;
    buffer.reserve(length);

    if (false == uri.scheme.empty()) {
        fmt::format_to(fmt::appender(buffer), "{}:", uri.scheme);
    }

    if (false == uri.host.empty()) {
        if (false == uri.scheme.empty()) {
            fmt::append_to(buffer, "//");
        }
        if (false == uri.user_info.empty()) {
            fmt::format_to(fmt::appender(buffer), "{}@", uri.user_info);
        }

        fmt::append_to(buffer, uri.host);

        if (0 != uri.port && uri_get_default_port_for_scheme(uri.scheme) != uri.port) {
            fmt::format_to(fmt::appender(buffer), ":{}", uri.port);
        }
    }

    fmt::append_to(buffer, uri.path);

    if (false == uri.query.empty()) {
        fmt::format_to(fmt::appender(buffer), "?{}", uri.query);
    }

    if (false == uri.fragment.empty()) {
        fmt::format_to(fmt::appender(buffer), "#{}", uri.fragment);
    }

    return fmt::to_string(buffer);
}

std::string hlib::uri_get_host_port(URI const& uri)
{
    fmt::memory_buffer buffer;
    buffer.reserve(uri.host.length() + 6);

    fmt::append_to(buffer, uri.host);
    if (uri.port != 0 && uri.port != uri_get_default_port_for_scheme(uri.scheme)) {
        fmt::format_to(fmt::appender(buffer), ":{}", uri.port);
    }

    return fmt::to_string(buffer);
}

std::string hlib::uri_get_path_query_fragment(URI const& uri)
{
    fmt::memory_buffer buffer;
    buffer.reserve(uri.path.length() + uri.query.length() + uri.fragment.length() + 2);

    fmt::append_to(buffer, uri.path);
    if (uri.query.empty() == false) {
        fmt::format_to(fmt::appender(buffer), "?{}", uri.query);
    }
    if (uri.fragment.empty() == false) {
        fmt::format_to(fmt::appender(buffer), "#{}", uri.fragment);
    }

    return fmt::to_string(buffer);
}

uint16_t hlib::uri_get_default_port_for_scheme(std::string const& scheme)
{
    static std::unordered_map<std::string, uint16_t> const table =
    {
        { "http",   80 },
        { "https",  443 },
        { "ws",     80 },
        { "wss",    443 }
    };


    auto it = table.find(scheme);
    return table.end() != it ? it->second : 0;
}

std::string hlib::uri_encoding_escape(std::string const& string)
{
    static char const *hdigit{ "0123456789ABCDEF" };

    auto escape = [](char const c)
    {
        return !(isalnum(c) || '-' == c || '.' == c || '_' == c || '~' == c);
    };

    fmt::memory_buffer buffer;
    buffer.reserve(string.length());

    for(size_t i = 0; i < string.length(); ++i) {
        char const c = string[i];
        if (escape(c)) {
            fmt::format_to(fmt::appender(buffer), "%{}{}", 
                hdigit[(c >> 4) & 0x0f],
                hdigit[(c >> 0) & 0x0f]
            );
        }
        else {
            fmt::append_to(buffer, c);
        }
    }

    return fmt::to_string(buffer);
}

std::string hlib::uri_encoding_unescape(std::string const& string)
{
    fmt::memory_buffer buffer;
    char hex[4]{};

    for(size_t i = 0; i < string.length(); ++i) {
        char c = string[i];
        if ('%' == c) {
            if (i + 3 > string.length()) {
                break;
            }

            hex[0] = string[++i];
            hex[1] = string[++i];

            if (!isxdigit(hex[0]) || !isxdigit(hex[1])) {
                break;
            }

            c = static_cast<char>(::strtoul(hex, nullptr, 16));
        }

        fmt::append_to(buffer, c);
    }

    return fmt::to_string(buffer);
}

