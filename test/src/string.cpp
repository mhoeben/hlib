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
#include "catch2/catch_test_macros.hpp"
#include "hlib/string.hpp"
#include "hlib/format.hpp"

using namespace hlib;

TEST_CASE("String Try To", "[string]")
{
    REQUIRE( true == try_to<bool>("true").value());
    REQUIRE(false == try_to<bool>("false").value());
    REQUIRE(false == try_to<bool>("1").has_value());
    REQUIRE(false == try_to<bool>("0").has_value());
    REQUIRE(false == try_to<bool>("foo").has_value());

    REQUIRE(   0 == try_to<int8_t>("0").value());
    REQUIRE( 127 == try_to<int8_t>("127").value());
    REQUIRE(-128 == try_to<int8_t>("-128").value());
    REQUIRE( 127 == try_to<int8_t>("0x7f", 16).value());
    REQUIRE(-128 == try_to<int8_t>("-0x80", 16).value());
    REQUIRE(false == try_to<int8_t>("128").has_value());
    REQUIRE(false == try_to<int8_t>("-129").has_value());
    REQUIRE(false == try_to<int8_t>("foo").has_value());
    REQUIRE(false == try_to<int8_t>(" 0").has_value());
    REQUIRE(false == try_to<int8_t>("0 ").has_value());

    REQUIRE(     0 == try_to<int16_t>("0").value());
    REQUIRE( 32767 == try_to<int16_t>("32767").value());
    REQUIRE(-32768 == try_to<int16_t>("-32768").value());
    REQUIRE( 32767 == try_to<int16_t>("0x7fff", 16).value());
    REQUIRE(-32768 == try_to<int16_t>("-0x8000", 16).value());
    REQUIRE( false == try_to<int16_t>("32768").has_value());
    REQUIRE( false == try_to<int16_t>("-32769").has_value());
    REQUIRE( false == try_to<int16_t>("foo").has_value());
    REQUIRE( false == try_to<int16_t>(" 0").has_value());
    REQUIRE( false == try_to<int16_t>("0 ").has_value());

    REQUIRE(          0 == try_to<int32_t>("0").value());
    REQUIRE( 2147483647 == try_to<int32_t>("2147483647").value());
    REQUIRE(-2147483648 == try_to<int32_t>("-2147483648").value());
    REQUIRE( 2147483647 == try_to<int32_t>("0x7fffffff", 16).value());
    REQUIRE(-2147483648 == try_to<int32_t>("-0x80000000", 16).value());
    REQUIRE( false == try_to<int32_t>("2147483648").has_value());
    REQUIRE( false == try_to<int32_t>("-2147483649").has_value());
    REQUIRE( false == try_to<int32_t>("foo").has_value());
    REQUIRE( false == try_to<int32_t>(" 0").has_value());
    REQUIRE( false == try_to<int32_t>("0 ").has_value());

    REQUIRE(                                   0 == try_to<int64_t>("0").value());
    REQUIRE( std::numeric_limits<int64_t>::max() == try_to<int64_t>("9223372036854775807").value());
    REQUIRE( std::numeric_limits<int64_t>::min() == try_to<int64_t>("-9223372036854775808").value());
    REQUIRE( std::numeric_limits<int64_t>::max() == try_to<int64_t>("0x7fffffffffffffff", 16).value());
    REQUIRE( std::numeric_limits<int64_t>::min() == try_to<int64_t>("-0x8000000000000000", 16).value());
    REQUIRE( false == try_to<int64_t>("9223372036854775808").has_value());
    REQUIRE( false == try_to<int64_t>("-9223372036854775809").has_value());
    REQUIRE( false == try_to<int64_t>("foo").has_value());
    REQUIRE( false == try_to<int64_t>(" 0").has_value());
    REQUIRE( false == try_to<int64_t>("0 ").has_value());

    REQUIRE(   0 == try_to<uint8_t>("0").value());
    REQUIRE( 255 == try_to<uint8_t>("255").value());
    REQUIRE( 255 == try_to<uint8_t>("0xff", 16).value());
    REQUIRE(false == try_to<uint8_t>("256").has_value());
    REQUIRE(false == try_to<uint8_t>("foo").has_value());
    REQUIRE(false == try_to<uint8_t>(" 0").has_value());
    REQUIRE(false == try_to<uint8_t>("0 ").has_value());

    REQUIRE(    0 == try_to<uint16_t>("0").value());
    REQUIRE(65535 == try_to<uint16_t>("65535").value());
    REQUIRE(65535 == try_to<uint16_t>("0xffff", 16).value());
    REQUIRE(false == try_to<uint16_t>("65536").has_value());
    REQUIRE(false == try_to<uint16_t>("foo").has_value());
    REQUIRE(false == try_to<uint16_t>(" 0").has_value());
    REQUIRE(false == try_to<uint16_t>("0 ").has_value());

    REQUIRE(         0 == try_to<uint32_t>("0").value());
    REQUIRE(4294967295 == try_to<uint32_t>("4294967295").value());
    REQUIRE(4294967295 == try_to<uint32_t>("0xffffffff", 16).value());
    REQUIRE(false == try_to<uint32_t>("4294967296").has_value());
    REQUIRE(false == try_to<uint32_t>("foo").has_value());
    REQUIRE(false == try_to<uint32_t>(" 0").has_value());
    REQUIRE(false == try_to<uint32_t>("0 ").has_value());

    REQUIRE(                                   0 == try_to<uint64_t>("0").value());
    REQUIRE(std::numeric_limits<uint64_t>::max() == try_to<uint64_t>("18446744073709551615").value());
    REQUIRE(std::numeric_limits<uint64_t>::max() == try_to<uint64_t>("0xffffffffffffffff", 16).value());
    REQUIRE(false == try_to<uint64_t>("18446744073709551616)").has_value());
    REQUIRE(false == try_to<uint64_t>("foo").has_value());
    REQUIRE(false == try_to<uint64_t>(" 0").has_value());
    REQUIRE(false == try_to<uint64_t>("0 ").has_value());

    REQUIRE(0.0f == try_to<float>("0.0").value());
    REQUIRE(1.0f == try_to<float>("1.0").value());
    REQUIRE(-1.0f == try_to<float>("-1").value());
    REQUIRE(false == try_to<float>("foo").has_value());
    REQUIRE(false == try_to<float>(" 0.0").has_value());
    REQUIRE(false == try_to<float>("0.0 ").has_value());

    REQUIRE(0.0 == try_to<double>("0.0").value());
    REQUIRE(1.0 == try_to<double>("1.0").value());
    REQUIRE(-1.0 == try_to<double>("-1").value());
    REQUIRE(false == try_to<double>("foo").has_value());
    REQUIRE(false == try_to<double>(" 0.0").has_value());
    REQUIRE(false == try_to<double>("0.0 ").has_value());
}

TEST_CASE("String To", "[string]")
{
    REQUIRE( true == to<bool>("true"));
    REQUIRE(false == to<bool>("false"));
    REQUIRE_THROWS(to<bool>("1"));
    REQUIRE_THROWS(to<bool>("0"));
    REQUIRE_THROWS(to<bool>("foo"));

    REQUIRE(   0 == to<int8_t>("0"));
    REQUIRE( 127 == to<int8_t>("127"));
    REQUIRE(-128 == to<int8_t>("-128"));
    REQUIRE( 127 == to<int8_t>("0x7f", 16));
    REQUIRE(-128 == to<int8_t>("-0x80", 16));
    REQUIRE_THROWS(to<int8_t>("128"));
    REQUIRE_THROWS(to<int8_t>("-129"));
    REQUIRE_THROWS(to<int8_t>("foo"));
    REQUIRE_THROWS(to<int8_t>(" 0"));
    REQUIRE_THROWS(to<int8_t>("0 "));

    REQUIRE(     0 == to<int16_t>("0"));
    REQUIRE( 32767 == to<int16_t>("32767"));
    REQUIRE(-32768 == to<int16_t>("-32768"));
    REQUIRE( 32767 == to<int16_t>("0x7fff", 16));
    REQUIRE(-32768 == to<int16_t>("-0x8000", 16));
    REQUIRE_THROWS(to<int16_t>("32768"));
    REQUIRE_THROWS(to<int16_t>("-32769"));
    REQUIRE_THROWS(to<int16_t>("foo"));
    REQUIRE_THROWS(to<int16_t>(" 0"));
    REQUIRE_THROWS(to<int16_t>("0 "));

    REQUIRE(          0 == to<int32_t>("0"));
    REQUIRE( 2147483647 == to<int32_t>("2147483647"));
    REQUIRE(-2147483648 == to<int32_t>("-2147483648"));
    REQUIRE( 2147483647 == to<int32_t>("0x7fffffff", 16));
    REQUIRE(-2147483648 == to<int32_t>("-0x80000000", 16));
    REQUIRE_THROWS(to<int32_t>("2147483648"));
    REQUIRE_THROWS(to<int32_t>("-2147483649"));
    REQUIRE_THROWS(to<int32_t>("foo"));
    REQUIRE_THROWS(to<int32_t>(" 0"));
    REQUIRE_THROWS(to<int32_t>("0 "));

    REQUIRE(                                   0 == to<int64_t>("0"));
    REQUIRE( std::numeric_limits<int64_t>::max() == to<int64_t>("9223372036854775807"));
    REQUIRE( std::numeric_limits<int64_t>::min() == to<int64_t>("-9223372036854775808"));
    REQUIRE( std::numeric_limits<int64_t>::max() == to<int64_t>("0x7fffffffffffffff", 16));
    REQUIRE( std::numeric_limits<int64_t>::min() == to<int64_t>("-0x8000000000000000", 16));
    REQUIRE_THROWS(to<int64_t>("9223372036854775808"));
    REQUIRE_THROWS(to<int64_t>("-9223372036854775809"));
    REQUIRE_THROWS(to<int64_t>("foo"));
    REQUIRE_THROWS(to<int64_t>(" 0"));
    REQUIRE_THROWS(to<int64_t>("0 "));

    REQUIRE(   0 == to<uint8_t>("0"));
    REQUIRE( 255 == to<uint8_t>("255"));
    REQUIRE( 255 == to<uint8_t>("0xff", 16));
    REQUIRE_THROWS(to<uint8_t>("256"));
    REQUIRE_THROWS(to<uint8_t>("foo"));
    REQUIRE_THROWS(to<uint8_t>(" 0"));
    REQUIRE_THROWS(to<uint8_t>("0 "));

    REQUIRE(    0 == to<uint16_t>("0"));
    REQUIRE(65535 == to<uint16_t>("65535"));
    REQUIRE(65535 == to<uint16_t>("0xffff", 16));
    REQUIRE_THROWS(to<uint16_t>("65536"));
    REQUIRE_THROWS(to<uint16_t>("foo"));
    REQUIRE_THROWS(to<uint16_t>(" 0"));
    REQUIRE_THROWS(to<uint16_t>("0 "));

    REQUIRE(         0 == to<uint32_t>("0"));
    REQUIRE(4294967295 == to<uint32_t>("4294967295"));
    REQUIRE(4294967295 == to<uint32_t>("0xffffffff", 16));
    REQUIRE_THROWS(to<uint32_t>("4294967296"));
    REQUIRE_THROWS(to<uint32_t>("foo"));
    REQUIRE_THROWS(to<uint32_t>(" 0"));
    REQUIRE_THROWS(to<uint32_t>("0 "));

    REQUIRE(                                   0 == to<uint64_t>("0"));
    REQUIRE(std::numeric_limits<uint64_t>::max() == to<uint64_t>("18446744073709551615"));
    REQUIRE(std::numeric_limits<uint64_t>::max() == to<uint64_t>("0xffffffffffffffff", 16));
    REQUIRE_THROWS(to<uint64_t>("18446744073709551616)"));
    REQUIRE_THROWS(to<uint64_t>("foo"));
    REQUIRE_THROWS(to<uint64_t>(" 0"));
    REQUIRE_THROWS(to<uint64_t>("0 "));

    REQUIRE(0.0f == to<float>("0.0"));
    REQUIRE(1.0f == to<float>("1.0"));
    REQUIRE(-1.0f == to<float>("-1"));
    REQUIRE_THROWS(to<float>("foo"));
    REQUIRE_THROWS(to<float>(" 0.0"));
    REQUIRE_THROWS(to<float>("0.0 "));

    REQUIRE(0.0 == to<double>("0.0"));
    REQUIRE(1.0 == to<double>("1.0"));
    REQUIRE(-1.0 == to<double>("-1"));
    REQUIRE_THROWS(to<double>("foo"));
    REQUIRE_THROWS(to<double>(" 0.0"));
    REQUIRE_THROWS(to<double>("0.0 "));
}

TEST_CASE("String Strip", "[string]")
{
    REQUIRE("foo" == strip(" \tfoo\r\n"));
    REQUIRE("foo bar" == strip(" \tfoo bar\r\n"));
    REQUIRE("foo-bar" == strip("*#*foo-bar#*#", "#*-"));
    REQUIRE(true == strip(" \t\f\v\r\n").empty());
}

TEST_CASE("String Split", "[string]")
{
    std::vector<std::string> tokens;

    REQUIRE_NOTHROW(tokens = split("", ','));
    REQUIRE(true == tokens.empty());

    REQUIRE_NOTHROW(tokens = split("foo bar", ','));
    REQUIRE(false == tokens.empty());
    REQUIRE(std::vector<std::string>{ "foo bar" } == tokens);

    REQUIRE_NOTHROW(tokens = split("foo,bar,baz", ','));
    REQUIRE(std::vector<std::string>{ "foo", "bar", "baz" } == tokens);

    REQUIRE_NOTHROW(tokens = split("foo,bar,baz,", ','));
    REQUIRE(std::vector<std::string>{ "foo", "bar", "baz", "" } == tokens);

    REQUIRE_NOTHROW(tokens = split("foo,bar,baz,", ',', true));
    REQUIRE(std::vector<std::string>{ "foo", "bar", "baz" } == tokens);

    REQUIRE_NOTHROW(tokens = split("foo,bar,,baz", ','));
    REQUIRE(std::vector<std::string>{ "foo", "bar", "", "baz" } == tokens);

    REQUIRE_NOTHROW(tokens = split("foo,bar,,baz", ',', true));
    REQUIRE(std::vector<std::string>{ "foo", "bar", "baz" } == tokens);

    REQUIRE_NOTHROW(tokens = split("foo,,bar,baz", ','));
    REQUIRE(std::vector<std::string>{ "foo", "", "bar", "baz" } == tokens);

    REQUIRE_NOTHROW(tokens = split("foo,,bar,baz", ',', true));
    REQUIRE(std::vector<std::string>{ "foo", "bar", "baz" } == tokens);

    REQUIRE_NOTHROW(tokens = split(",foo,bar,baz", ','));
    REQUIRE(std::vector<std::string>{ "", "foo", "bar", "baz" } == tokens);

    REQUIRE_NOTHROW(tokens = split(",foo,bar,baz", ',', true));
    REQUIRE(std::vector<std::string>{ "foo", "bar", "baz" } == tokens);

    REQUIRE_NOTHROW(tokens = split(",", ','));
    REQUIRE(false == tokens.empty());
    REQUIRE(std::vector<std::string>{ "", "" } == tokens);

    REQUIRE_NOTHROW(tokens = split(",", ',', true));
    REQUIRE(true == tokens.empty());

    REQUIRE_NOTHROW(tokens = split("foo bar\tbaz\nxxx\ryyy", isspace));
    REQUIRE(std::vector<std::string>{ "foo", "bar", "baz", "xxx", "yyy" } == tokens);
}

