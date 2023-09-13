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

#include <cstdint>
#include <string>

namespace hlib
{

struct URI
{
    enum class Syntax
    {
        URI,
        Reference,
        Absolute
    };

    std::string scheme;
    std::string user_info;
    std::string host;
    std::uint16_t port{ 0 };
    std::string path;
    std::string query;
    std::string fragment;

    URI() = default;
    URI(std::string const &that, Syntax syntax = Syntax::URI);
};

URI uri_parse(std::string const& string, URI::Syntax syntax = URI::Syntax::URI);

std::string to_string(URI const& uri);

std::string uri_get_authority(URI const& uri);
std::string uri_get_host_port(URI const& uri);
std::string uri_get_path_query_fragment(URI const& uri);

std::uint16_t uri_get_default_port_for_scheme(std::string const& scheme);

std::string uri_encoding_escape(std::string const& string);
std::string uri_encoding_unescape(std::string const& string);

std::string target_get_path(std::string const& target);
std::string target_get_query(std::string const& target);
std::string target_get_fragment(std::string const& target);

} // namespace tools

