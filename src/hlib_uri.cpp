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
#include "hlib_format.hpp"
#include "hlib/string.hpp"
#include <regex>
#include <unordered_map>

using namespace hlib;

//
// Public
//
URI::URI(std::string const& that)
{
    *this = uri_parse(that);
}

URI hlib::uri_parse(std::string const& string)
{
    // See https://datatracker.ietf.org/doc/html/rfc3986#appendix-B.
    static std::regex const uri_regex(
        R"(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)",
        std::regex::extended
    );

    URI uri;
    std::string::size_type pos;

    std::smatch matches;
    if (false == std::regex_match(string, matches, uri_regex)) {
        throw std::invalid_argument("string argument not a valid URI");
    }
    assert(10 == matches.size());

    // Get scheme match.
    uri.scheme = matches[2].str();

    // Get authority match.
    std::string authority = matches[4];

    // User-info (username[:password]@) present?
    pos = authority.find('@');
    if (std::string::npos != pos) {
        uri.user_info = authority.substr(0, pos);
        authority = authority.substr(pos + 1);
    }

    // Port present?
    pos = authority.rfind(':');
    if (std::string::npos != pos) {
        // Parse characters after ':', if it is parsable as uint16_t, it is a port.
        std::optional<std::uint16_t> port = stoui16(authority.substr(pos + 1), 10, std::nothrow);
        if (std::nullopt != port) {
            uri.port = port.value();
            authority = authority.substr(0, pos);
        }
    }

    // Remaining authority string is host.
    uri.host = std::move(authority);

    // Default port?
    if (0 == uri.port) {
        uri.port = uri_get_default_port_for_scheme(uri.scheme);
    }

    // Get path match.
    uri.path = matches[5].str();
    if (true == uri.path.empty()) {
        uri.path = "/";
    }

    // Get query and fragment matches.
    uri.query = matches[7].str();
    uri.fragment = matches[9];
    return uri;
}

std::string hlib::to_string(URI const& uri)
{
    std::size_t const length = uri.scheme.length() + 3
                        + uri.user_info.length() + 1
                        + uri.host.length() + 6
                        + uri.path.length() + 1
                        + uri.query.length() + 1
                        + uri.fragment.length() + 1;

    std::string string;
    string.reserve(length);

    if (false == uri.scheme.empty()) {
        format_to(string, "%s:", uri.scheme.c_str());
    }

    if (false == uri.host.empty()) {
        string += "//";

        if (false == uri.user_info.empty()) {
            format_to(string, "%s@", uri.user_info.c_str());
        }

        string += uri.host;
        if (0 != uri.port && uri_get_default_port_for_scheme(uri.scheme) != uri.port) {
            format_to(string, ":%u", uri.port);
        }
    }

    string += false == uri.path.empty() ? uri.path.c_str() : "/";

    if (false == uri.query.empty()) {
        format_to(string, "?%s", uri.query.c_str());
    }

    if (false == uri.fragment.empty()) {
        format_to(string, "#%s", uri.fragment.c_str());
    }

    return string;
}

std::string hlib::uri_get_host_port(URI const& uri)
{
    std::string string;
    string.reserve(uri.host.length() + 6);

    string = uri.host;
    if (uri.port != 0 && uri.port != uri_get_default_port_for_scheme(uri.scheme)) {
        format_to(string, ":%u", uri.port);
    }

    return string;
}

std::string hlib::uri_get_path_query_fragment(URI const& uri)
{
    std::string string;
    string.reserve(uri.path.length() + uri.query.length() + uri.fragment.length() + 2);

    string = uri.path;
    if (uri.query.empty() == false) {
        format_to(string, "?%s", uri.query.c_str());
    }
    if (uri.fragment.empty() == false) {
        format_to(string, "#%s", uri.fragment.c_str());
    }

    return string;
}

std::uint16_t hlib::uri_get_default_port_for_scheme(std::string const& scheme)
{
    static std::unordered_map<std::string, std::uint16_t> const table =
    {
        { "ftp",    22 },
        { "gopher", 70 },
        { "http",   80 },
        { "https",  443 },
        { "imap",   143 },
        { "ldap",   389 },
        { "nfs",    2049 },
        { "nntp",   119 },
        { "pop",    110 },
        { "smtp",   25 },
        { "telnet", 23 },
        { "ws",     80 },
        { "wss",    443 }
    };


    auto it = table.find(scheme);
    return table.end() != it ? it->second : 0;
}

std::string hlib::uri_encoding_escape(std::string const& string)
{
    static char const *hdigit{ "0123456789ABCDEF" };

    auto needs_escaping = [](char const c)
    {
        return !(isalnum(c) || '-' == c || '.' == c || '_' == c || '~' == c);
    };

    std::string escaped;
    escaped.reserve(string.length());

    for(std::size_t i = 0; i < string.length(); ++i) {
        char const c = string[i];
        if (true == needs_escaping(c)) {
            format_to(escaped, "%%%c%c", 
                hdigit[(c >> 4) & 0x0f],
                hdigit[(c >> 0) & 0x0f]
            );
        }
        else {
            escaped += c;
        }
    }

    return escaped;
}

std::string hlib::uri_encoding_unescape(std::string const& string)
{
    std::string unescaped;
    unescaped.reserve(string.length());

    char hex[4]{};

    for(std::size_t i = 0; i < string.length(); ++i) {
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

        unescaped += hex;
    }

    return unescaped;
}

std::string hlib::target_get_path(std::string const& target)
{
    std::string::size_type end = target.find_first_of("?#");
    if (std::string::npos == end) {
        return target;
    }

    return target.substr(0, end);
}

std::string hlib::target_get_query(std::string const& target)
{
    std::string::size_type start = target.find('?');
    if (std::string::npos == start) {
        return std::string();
    }

    ++start;

    std::string::size_type end = target.find('#', start);
    if (std::string::npos == end) {
        end = target.length();
    }

    return target.substr(start, end - start);
}

std::string hlib::target_get_fragment(std::string const& target)
{
    std::string::size_type start = target.find('#');
    if (std::string::npos == start) {
        return std::string();
    }

    return target.substr(start + 1);
}

