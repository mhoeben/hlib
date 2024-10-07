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
#include "hlib/string.hpp"
#include <map>

using namespace hlib;

TEST_CASE("String To NoThrow", "[string]")
{
    REQUIRE( true == string_to<bool>("true", std::nothrow).value());
    REQUIRE(false == string_to<bool>("false", std::nothrow).value());
    REQUIRE(false == string_to<bool>("1", std::nothrow).has_value());
    REQUIRE(false == string_to<bool>("0", std::nothrow).has_value());
    REQUIRE(false == string_to<bool>("foo", std::nothrow).has_value());

    REQUIRE(   0 == string_to<int8_t>("0", 10, std::nothrow).value());
    REQUIRE( 127 == string_to<int8_t>("127", 10, std::nothrow).value());
    REQUIRE(-128 == string_to<int8_t>("-128", 10, std::nothrow).value());
    REQUIRE( 127 == string_to<int8_t>("0x7f", 16, std::nothrow).value());
    REQUIRE(-128 == string_to<int8_t>("-0x80", 16, std::nothrow).value());
    REQUIRE(false == string_to<int8_t>("128", 10, std::nothrow).has_value());
    REQUIRE(false == string_to<int8_t>("-129", 10, std::nothrow).has_value());
    REQUIRE(false == string_to<int8_t>("foo", 10, std::nothrow).has_value());
    REQUIRE(false == string_to<int8_t>(" 0", 10, std::nothrow).has_value());
    REQUIRE(false == string_to<int8_t>("0 ", 10, std::nothrow).has_value());

    REQUIRE(     0 == string_to<int16_t>("0", 10, std::nothrow).value());
    REQUIRE( 32767 == string_to<int16_t>("32767", 10, std::nothrow).value());
    REQUIRE(-32768 == string_to<int16_t>("-32768", 10, std::nothrow).value());
    REQUIRE( 32767 == string_to<int16_t>("0x7fff", 16, std::nothrow).value());
    REQUIRE(-32768 == string_to<int16_t>("-0x8000", 16, std::nothrow).value());
    REQUIRE( false == string_to<int16_t>("32768", 10, std::nothrow).has_value());
    REQUIRE( false == string_to<int16_t>("-32769", 10, std::nothrow).has_value());
    REQUIRE( false == string_to<int16_t>("foo", 10, std::nothrow).has_value());
    REQUIRE( false == string_to<int16_t>(" 0", 10, std::nothrow).has_value());
    REQUIRE( false == string_to<int16_t>("0 ", 10, std::nothrow).has_value());

    REQUIRE(          0 == string_to<int32_t>("0", 10, std::nothrow).value());
    REQUIRE( 2147483647 == string_to<int32_t>("2147483647", 10, std::nothrow).value());
    REQUIRE(-2147483648 == string_to<int32_t>("-2147483648", 10, std::nothrow).value());
    REQUIRE( 2147483647 == string_to<int32_t>("0x7fffffff", 16, std::nothrow).value());
    REQUIRE(-2147483648 == string_to<int32_t>("-0x80000000", 16, std::nothrow).value());
    REQUIRE( false == string_to<int32_t>("2147483648", 10, std::nothrow).has_value());
    REQUIRE( false == string_to<int32_t>("-2147483649", 10, std::nothrow).has_value());
    REQUIRE( false == string_to<int32_t>("foo", 10, std::nothrow).has_value());
    REQUIRE( false == string_to<int32_t>(" 0", 10, std::nothrow).has_value());
    REQUIRE( false == string_to<int32_t>("0 ", 10, std::nothrow).has_value());

    REQUIRE(                                   0 == string_to<int64_t>("0", 10, std::nothrow).value());
    REQUIRE( std::numeric_limits<int64_t>::max() == string_to<int64_t>("9223372036854775807", 10, std::nothrow).value());
    REQUIRE( std::numeric_limits<int64_t>::min() == string_to<int64_t>("-9223372036854775808", 10, std::nothrow).value());
    REQUIRE( std::numeric_limits<int64_t>::max() == string_to<int64_t>("0x7fffffffffffffff", 16, std::nothrow).value());
    REQUIRE( std::numeric_limits<int64_t>::min() == string_to<int64_t>("-0x8000000000000000", 16, std::nothrow).value());
    REQUIRE( false == string_to<int64_t>("9223372036854775808", 10, std::nothrow).has_value());
    REQUIRE( false == string_to<int64_t>("-9223372036854775809", 10, std::nothrow).has_value());
    REQUIRE( false == string_to<int64_t>("foo", 10, std::nothrow).has_value());
    REQUIRE( false == string_to<int64_t>(" 0", 10, std::nothrow).has_value());
    REQUIRE( false == string_to<int64_t>("0 ", 10, std::nothrow).has_value());

    REQUIRE(    0 == string_to<uint8_t>("0", 10, std::nothrow).value());
    REQUIRE(  255 == string_to<uint8_t>("255", 10, std::nothrow).value());
    REQUIRE(  255 == string_to<uint8_t>("0xff", 16, std::nothrow).value());
    REQUIRE(false == string_to<uint8_t>("256", 10, std::nothrow).has_value());
    REQUIRE(false == string_to<uint8_t>("foo", 10, std::nothrow).has_value());
    REQUIRE(false == string_to<uint8_t>(" 0", 10, std::nothrow).has_value());
    REQUIRE(false == string_to<uint8_t>("0 ", 10, std::nothrow).has_value());

    REQUIRE(    0 == string_to<uint16_t>("0", 10, std::nothrow).value());
    REQUIRE(65535 == string_to<uint16_t>("65535", 10, std::nothrow).value());
    REQUIRE(65535 == string_to<uint16_t>("0xffff", 16, std::nothrow).value());
    REQUIRE(false == string_to<uint16_t>("65536", 10, std::nothrow).has_value());
    REQUIRE(false == string_to<uint16_t>("foo", 10, std::nothrow).has_value());
    REQUIRE(false == string_to<uint16_t>(" 0", 10, std::nothrow).has_value());
    REQUIRE(false == string_to<uint16_t>("0 ", 10, std::nothrow).has_value());

    REQUIRE(         0 == string_to<uint32_t>("0", 10, std::nothrow).value());
    REQUIRE(4294967295 == string_to<uint32_t>("4294967295", 10, std::nothrow).value());
    REQUIRE(4294967295 == string_to<uint32_t>("0xffffffff", 16, std::nothrow).value());
    REQUIRE(     false == string_to<uint32_t>("4294967296", 10, std::nothrow).has_value());
    REQUIRE(     false == string_to<uint32_t>("foo", 10, std::nothrow).has_value());
    REQUIRE(     false == string_to<uint32_t>(" 0", 10, std::nothrow).has_value());
    REQUIRE(     false == string_to<uint32_t>("0 ", 10, std::nothrow).has_value());

    REQUIRE(                                   0 == string_to<uint64_t>("0", 10, std::nothrow).value());
    REQUIRE(std::numeric_limits<uint64_t>::max() == string_to<uint64_t>("18446744073709551615", 10, std::nothrow).value());
    REQUIRE(std::numeric_limits<uint64_t>::max() == string_to<uint64_t>("0xffffffffffffffff", 16, std::nothrow).value());
    REQUIRE(                               false == string_to<uint64_t>("18446744073709551616)", 10, std::nothrow).has_value());
    REQUIRE(                               false == string_to<uint64_t>("foo", 10, std::nothrow).has_value());
    REQUIRE(                               false == string_to<uint64_t>(" 0", 10, std::nothrow).has_value());
    REQUIRE(                               false == string_to<uint64_t>("0 ", 10, std::nothrow).has_value());

    REQUIRE( 0.0f == string_to<float>("0.0", std::nothrow).value());
    REQUIRE( 1.0f == string_to<float>("1.0", std::nothrow).value());
    REQUIRE(-1.0f == string_to<float>("-1", std::nothrow).value());
    REQUIRE(false == string_to<float>("foo", std::nothrow).has_value());
    REQUIRE(false == string_to<float>(" 0.0", std::nothrow).has_value());
    REQUIRE(false == string_to<float>("0.0 ", std::nothrow).has_value());

    REQUIRE(  0.0 == string_to<double>("0.0", std::nothrow).value());
    REQUIRE(  1.0 == string_to<double>("1.0", std::nothrow).value());
    REQUIRE( -1.0 == string_to<double>("-1", std::nothrow).value());
    REQUIRE(false == string_to<double>("foo", std::nothrow).has_value());
    REQUIRE(false == string_to<double>(" 0.0", std::nothrow).has_value());
    REQUIRE(false == string_to<double>("0.0 ", std::nothrow).has_value());
}

TEST_CASE("String To", "[string]")
{
    REQUIRE( true == string_to<bool>("true"));
    REQUIRE(false == string_to<bool>("false"));
    REQUIRE_THROWS(string_to<bool>("1"));
    REQUIRE_THROWS(string_to<bool>("0"));
    REQUIRE_THROWS(string_to<bool>("foo"));

    REQUIRE(   0 == string_to<int8_t>("0"));
    REQUIRE( 127 == string_to<int8_t>("127"));
    REQUIRE(-128 == string_to<int8_t>("-128"));
    REQUIRE( 127 == string_to<int8_t>("0x7f", 16));
    REQUIRE(-128 == string_to<int8_t>("-0x80", 16));
    REQUIRE_THROWS(string_to<int8_t>("128"));
    REQUIRE_THROWS(string_to<int8_t>("-129"));
    REQUIRE_THROWS(string_to<int8_t>("foo"));
    REQUIRE_THROWS(string_to<int8_t>(" 0"));
    REQUIRE_THROWS(string_to<int8_t>("0 "));

    REQUIRE(     0 == string_to<int16_t>("0"));
    REQUIRE( 32767 == string_to<int16_t>("32767"));
    REQUIRE(-32768 == string_to<int16_t>("-32768"));
    REQUIRE( 32767 == string_to<int16_t>("0x7fff", 16));
    REQUIRE(-32768 == string_to<int16_t>("-0x8000", 16));
    REQUIRE_THROWS(string_to<int16_t>("32768"));
    REQUIRE_THROWS(string_to<int16_t>("-32769"));
    REQUIRE_THROWS(string_to<int16_t>("foo"));
    REQUIRE_THROWS(string_to<int16_t>(" 0"));
    REQUIRE_THROWS(string_to<int16_t>("0 "));

    REQUIRE(          0 == string_to<int32_t>("0"));
    REQUIRE( 2147483647 == string_to<int32_t>("2147483647"));
    REQUIRE(-2147483648 == string_to<int32_t>("-2147483648"));
    REQUIRE( 2147483647 == string_to<int32_t>("0x7fffffff", 16));
    REQUIRE(-2147483648 == string_to<int32_t>("-0x80000000", 16));
    REQUIRE_THROWS(string_to<int32_t>("2147483648"));
    REQUIRE_THROWS(string_to<int32_t>("-2147483649"));
    REQUIRE_THROWS(string_to<int32_t>("foo"));
    REQUIRE_THROWS(string_to<int32_t>(" 0"));
    REQUIRE_THROWS(string_to<int32_t>("0 "));

    REQUIRE(                                   0 == string_to<int64_t>("0"));
    REQUIRE( std::numeric_limits<int64_t>::max() == string_to<int64_t>("9223372036854775807"));
    REQUIRE( std::numeric_limits<int64_t>::min() == string_to<int64_t>("-9223372036854775808"));
    REQUIRE( std::numeric_limits<int64_t>::max() == string_to<int64_t>("0x7fffffffffffffff", 16));
    REQUIRE( std::numeric_limits<int64_t>::min() == string_to<int64_t>("-0x8000000000000000", 16));
    REQUIRE_THROWS(string_to<int64_t>("9223372036854775808"));
    REQUIRE_THROWS(string_to<int64_t>("-9223372036854775809"));
    REQUIRE_THROWS(string_to<int64_t>("foo"));
    REQUIRE_THROWS(string_to<int64_t>(" 0"));
    REQUIRE_THROWS(string_to<int64_t>("0 "));

    REQUIRE(   0 == string_to<uint8_t>("0"));
    REQUIRE( 255 == string_to<uint8_t>("255"));
    REQUIRE( 255 == string_to<uint8_t>("0xff", 16));
    REQUIRE_THROWS(string_to<uint8_t>("256"));
    REQUIRE_THROWS(string_to<uint8_t>("foo"));
    REQUIRE_THROWS(string_to<uint8_t>(" 0"));
    REQUIRE_THROWS(string_to<uint8_t>("0 "));

    REQUIRE(    0 == string_to<uint16_t>("0"));
    REQUIRE(65535 == string_to<uint16_t>("65535"));
    REQUIRE(65535 == string_to<uint16_t>("0xffff", 16));
    REQUIRE_THROWS(string_to<uint16_t>("65536"));
    REQUIRE_THROWS(string_to<uint16_t>("foo"));
    REQUIRE_THROWS(string_to<uint16_t>(" 0"));
    REQUIRE_THROWS(string_to<uint16_t>("0 "));

    REQUIRE(         0 == string_to<uint32_t>("0"));
    REQUIRE(4294967295 == string_to<uint32_t>("4294967295"));
    REQUIRE(4294967295 == string_to<uint32_t>("0xffffffff", 16));
    REQUIRE_THROWS(string_to<uint32_t>("4294967296"));
    REQUIRE_THROWS(string_to<uint32_t>("foo"));
    REQUIRE_THROWS(string_to<uint32_t>(" 0"));
    REQUIRE_THROWS(string_to<uint32_t>("0 "));

    REQUIRE(                                   0 == string_to<uint64_t>("0"));
    REQUIRE(std::numeric_limits<uint64_t>::max() == string_to<uint64_t>("18446744073709551615"));
    REQUIRE(std::numeric_limits<uint64_t>::max() == string_to<uint64_t>("0xffffffffffffffff", 16));
    REQUIRE_THROWS(string_to<uint64_t>("18446744073709551616)"));
    REQUIRE_THROWS(string_to<uint64_t>("foo"));
    REQUIRE_THROWS(string_to<uint64_t>(" 0"));
    REQUIRE_THROWS(string_to<uint64_t>("0 "));

    REQUIRE(0.0f == string_to<float>("0.0"));
    REQUIRE(1.0f == string_to<float>("1.0"));
    REQUIRE(-1.0f == string_to<float>("-1"));
    REQUIRE_THROWS(string_to<float>("foo"));
    REQUIRE_THROWS(string_to<float>(" 0.0"));
    REQUIRE_THROWS(string_to<float>("0.0 "));

    REQUIRE(0.0 == string_to<double>("0.0"));
    REQUIRE(1.0 == string_to<double>("1.0"));
    REQUIRE(-1.0 == string_to<double>("-1"));
    REQUIRE_THROWS(string_to<double>("foo"));
    REQUIRE_THROWS(string_to<double>(" 0.0"));
    REQUIRE_THROWS(string_to<double>("0.0 "));
}

TEST_CASE("String C++20", "[string]")
{
    REQUIRE(true  == starts_with("foobar", "foo"));
    REQUIRE(false == starts_with("foobar", "bar"));
    REQUIRE(false == starts_with("foo", "foobar"));
    REQUIRE(true  == starts_with("foobar", "foobar"));
    REQUIRE(false == starts_with("", "foobar"));

    REQUIRE(true  == ends_with("foobar", "bar"));
    REQUIRE(false == ends_with("foobar", "foo"));
    REQUIRE(false == ends_with("bar", "foobar"));
    REQUIRE(true  == ends_with("foobar", "foobar"));
    REQUIRE(false == ends_with("", "foobar"));

    REQUIRE(true  == contains("foobar", "foo"));
    REQUIRE(true  == contains("foobar", "bar"));
    REQUIRE(true  == contains("foobar", "ob"));
    REQUIRE(true  == contains("foobar", "foobar"));
    REQUIRE(false == contains("foobar", "--"));
    REQUIRE(false == contains("foo", "foobar"));
    REQUIRE(false == contains("bar", "foobar"));
    REQUIRE(false == contains("", "foobar"));

    REQUIRE(0 == count("foobar", "baz"));
    REQUIRE(1 == count("foobar", "bar"));
    REQUIRE(2 == count("foobarfoo", "foo"));
    REQUIRE(1 == count("foobarfoo", "bar"));

    REQUIRE(0 == count("foobar", 'x'));
    REQUIRE(1 == count("foobar", 'a'));
    REQUIRE(2 == count("foobar", 'o'));
}

TEST_CASE("String Trim", "[string]")
{
    REQUIRE("foo" == trim(" \tfoo\r\n"));
    REQUIRE("foo bar" == trim(" \tfoo bar\r\n"));
    REQUIRE("foo-bar" == trim("*#*foo-bar#*#", "#*-"));
    REQUIRE(true == trim(" \t\f\v\r\n").empty());
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

TEST_CASE("String Join", "[string]")
{
    REQUIRE("foo bar" == join({ "foo", "bar" }, " "));
    REQUIRE("13, 11, 1971" == join<std::vector<int>>({ 13, 11, 1971 }, ", "));

    static std::map<int, std::string> const map =
    {
        { 1, "one" },
        { 2, "two" },
        { 3, "three" }
    };

    // REQUIRE("(1 == one), (2 == two), (3 == three)" == join(map, ", ", [](auto pair) { return fmt::format("({} == {})", pair.first, pair.second); }));
}

TEST_CASE("String Replace", "[string]")
{
    REQUIRE("foo baz" == replace("foo bar", "bar", "baz"));
    REQUIRE("baz bar" == replace("foo bar", "foo", "baz"));
    REQUIRE("foobar" == replace("foo bar", " ", ""));
}
