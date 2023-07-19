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
#include "test.hpp"
#include "hlib/uri.hpp"

using namespace hlib;

TEST_CASE("URI", "[uri]")
{
    URI uri("scheme://user:password@host:1234/path/to/nowhere?query#fragment");
    REQUIRE("scheme" == uri.scheme);
    REQUIRE("user:password" == uri.user_info);
    REQUIRE("host" == uri.host);
    REQUIRE(1234 == uri.port);
    REQUIRE("/path/to/nowhere" == uri.path);
    REQUIRE("query" == uri.query);
    REQUIRE("fragment" == uri.fragment);
    REQUIRE("scheme://user:password@host:1234/path/to/nowhere?query#fragment" == to_string(uri));
}

TEST_CASE("URI Empty Path", "[uri]")
{
    URI uri("scheme://host");
    REQUIRE("scheme" == uri.scheme);
    REQUIRE(true == uri.user_info.empty());
    REQUIRE("host" == uri.host);
    REQUIRE(0 == uri.port);
    REQUIRE("/" == uri.path);
    REQUIRE(true == uri.query.empty());
    REQUIRE(true == uri.fragment.empty());
    REQUIRE("scheme://host/" == to_string(uri));
}

TEST_CASE("URI File", "[uri]")
{
    URI uri("file:///C:/Users/Username/Documents/example.txt");
    REQUIRE("file" == uri.scheme);
    REQUIRE(true == uri.user_info.empty());
    REQUIRE(true == uri.host.empty());
    REQUIRE(0 == uri.port);
    REQUIRE("/C:/Users/Username/Documents/example.txt" == uri.path);
    REQUIRE(true == uri.query.empty());
    REQUIRE(true == uri.fragment.empty());
    // FIXME
    // REQUIRE("file:///C:/Users/Username/Documents/example.txt" == to_string(uri));
}

TEST_CASE("URI No Port", "[uri]")
{
    REQUIRE( 80 == URI("http://example.com").port);
    REQUIRE(443 == URI("https://example.com").port);
    REQUIRE( 80 == URI("ws://example.com").port);
    REQUIRE(443 == URI("wss://example.com").port);
}

TEST_CASE("URI Invalid", "[uri]")
{
    REQUIRE_THROWS(uri_parse("reference"));
}

TEST_CASE("URI Wikipedia Examples", "[uri]")
{
    URI uri;

    uri = uri_parse("https://john.doe@www.example.com:123/forum/questions/?tag=networking&order=newest#top");
    REQUIRE("https" == uri.scheme);
    REQUIRE("john.doe" == uri.user_info);
    REQUIRE("www.example.com" == uri.host);
    REQUIRE(123 == uri.port);
    REQUIRE("/forum/questions/" == uri.path);
    REQUIRE("tag=networking&order=newest" == uri.query); 
    REQUIRE("top" == uri.fragment);
    REQUIRE("https://john.doe@www.example.com:123/forum/questions/?tag=networking&order=newest#top" == to_string(uri));

    uri = uri_parse("ldap://[2001:db8::7]/c=GB?objectClass?one");
    REQUIRE("ldap" == uri.scheme);
    REQUIRE(true == uri.user_info.empty());
    REQUIRE("[2001:db8::7]" == uri.host);
    REQUIRE(0 == uri.port);
    REQUIRE("/c=GB" == uri.path);
    REQUIRE("objectClass?one" == uri.query);
    REQUIRE(true == uri.fragment.empty());
    REQUIRE("ldap://[2001:db8::7]/c=GB?objectClass?one" == to_string(uri));

    uri = uri_parse("mailto:John.Doe@example.com");
    REQUIRE("mailto" == uri.scheme);
    REQUIRE(true == uri.user_info.empty());
    REQUIRE(true == uri.host.empty());
    REQUIRE(0 == uri.port);
    REQUIRE("John.Doe@example.com" == uri.path);
    REQUIRE(true == uri.query.empty());
    REQUIRE(true == uri.fragment.empty());
    REQUIRE("mailto:John.Doe@example.com" == to_string(uri));

    uri = uri_parse("news:comp.infosystems.www.servers.unix");
    REQUIRE("news" == uri.scheme);
    REQUIRE(true == uri.user_info.empty());
    REQUIRE(true == uri.host.empty());
    REQUIRE(0 == uri.port);
    REQUIRE("comp.infosystems.www.servers.unix" == uri.path);
    REQUIRE(true == uri.query.empty());
    REQUIRE(true == uri.fragment.empty());
    REQUIRE("news:comp.infosystems.www.servers.unix" == to_string(uri));

    uri = uri_parse("tel:+1-816-555-1212");
    REQUIRE("tel" == uri.scheme);
    REQUIRE(true == uri.user_info.empty());
    REQUIRE(true == uri.host.empty());
    REQUIRE(0 == uri.port);
    REQUIRE("+1-816-555-1212" == uri.path);
    REQUIRE(true == uri.query.empty());
    REQUIRE(true == uri.fragment.empty());
    REQUIRE("tel:+1-816-555-1212" == to_string(uri));

    uri = uri_parse("telnet://192.0.2.16:80/");
    REQUIRE("telnet" == uri.scheme);
    REQUIRE(true == uri.user_info.empty());
    REQUIRE("192.0.2.16" == uri.host);
    REQUIRE(80 == uri.port);
    REQUIRE("/" == uri.path);
    REQUIRE(true == uri.query.empty());
    REQUIRE(true == uri.fragment.empty());
    REQUIRE("telnet://192.0.2.16:80/" == to_string(uri));

    uri = uri_parse("urn:oasis:names:specification:docbook:dtd:xml:4.1.2");
    REQUIRE("urn" == uri.scheme);
    REQUIRE(true == uri.user_info.empty());
    REQUIRE(true == uri.host.empty());
    REQUIRE(0 == uri.port);
    REQUIRE("oasis:names:specification:docbook:dtd:xml:4.1.2" == uri.path);
    REQUIRE(true == uri.query.empty());
    REQUIRE(true == uri.fragment.empty());
    REQUIRE("urn:oasis:names:specification:docbook:dtd:xml:4.1.2" == to_string(uri));
}

