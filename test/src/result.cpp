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
#include "hlib/result.hpp"

using namespace hlib;

TEST_CASE("Result Monostate", "[result]")
{
    Result<> result;
    REQUIRE(true == result.success());
    REQUIRE(false == result.failure());
    REQUIRE(std::monostate() == result.value());
}

TEST_CASE("Result String", "[result]")
{
    Result<std::string> result;
    REQUIRE(false == !result);
    REQUIRE(true == result.success());
    REQUIRE(false == result.failure());
    REQUIRE("" == result.value());
    REQUIRE_THROWS(result.error());

    std::string const string = "success";

    result = Result(string);
    REQUIRE(true == result.success());
    REQUIRE(false == result.failure());
    REQUIRE(string == result.value());
    REQUIRE_THROWS(result.error());

    result = "more success";
    REQUIRE("more success" == result.value());

    std::error_code const error_code = std::make_error_code(std::errc::invalid_argument);

    result = std::system_error(error_code);
    REQUIRE(false == result.success());
    REQUIRE(true == result.failure());
    REQUIRE_THROWS(result.value());
    REQUIRE(error_code == result.error().code());
}

TEST_CASE("Result Integer", "[result]")
{
    Result<int> result;
    REQUIRE(true == result.success());
    REQUIRE(false == result.failure());
    REQUIRE(0 == result.value());
    REQUIRE_THROWS(result.error());

    result = 1311;
    REQUIRE(true == result.success());
    REQUIRE(false == result.failure());
    REQUIRE(1311 == result.value());
    REQUIRE_THROWS(result.error());
}

TEST_CASE("Result Check", "[result]")
{
    Result<int> int_result(13);
    REQUIRE(13 == check<int>(int_result, [](Error&& error) { throw error; }));

    Result<int> error_result = make_system_error(EINVAL);
    REQUIRE_THROWS(check<int>(error_result, [](Error&& error) { throw error; }));

    Error error;
    REQUIRE(11 == check<int>(error_result, set_error<int>(error, 11)));
}

