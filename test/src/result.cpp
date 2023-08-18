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
    REQUIRE(false == result.fail());
    REQUIRE(std::monostate() == as<decltype(result)::Type>(result));
}

TEST_CASE("Result String", "[result]")
{
    Result<std::string> result;
    REQUIRE(true == result);
    REQUIRE(false == !result);
    REQUIRE(true == result.success());
    REQUIRE(false == result.fail());
    REQUIRE("" == as<std::string>(result));
    REQUIRE_THROWS(as<std::error_code>(result));

    std::string const string = "success";

    result = Result(string);
    REQUIRE(true == result.success());
    REQUIRE(false == result.fail());
    REQUIRE(string == as<std::string>(result));
    REQUIRE_THROWS(as<std::error_code>(result));

    result = "more success";
    REQUIRE("more success" == as<std::string>(result));

    std::error_code const error_code = std::make_error_code(std::errc::invalid_argument);

    result = Error(error_code);
    REQUIRE(false == result.success());
    REQUIRE(true == result.fail());
    REQUIRE_THROWS(as<std::string>(result));
    REQUIRE(error_code == as<std::error_code>(result));

    std::string const error_string = "error string";

    result = Error(error_string);
    REQUIRE(false == result.success());
    REQUIRE(true == result.fail());
    REQUIRE_THROWS(as<std::string>(result));
    REQUIRE_THROWS(as<std::error_code>(result));

    Error error = to<Error>(result);
    REQUIRE(false == error.success());
    REQUIRE(true == error.fail());
    REQUIRE(error_string == as<std::string>(error));
}

TEST_CASE("Result Integer", "[result]")
{
    Result<int> result;
    REQUIRE(true == result.success());
    REQUIRE(false == result.fail());
    REQUIRE(0 == as<int>(result));
    REQUIRE_THROWS(as<std::error_code>(result));
    REQUIRE_THROWS(as<std::string>(result));

    result = 1311;
    REQUIRE(true == result.success());
    REQUIRE(false == result.fail());
    REQUIRE(1311 == as<int>(result));
    REQUIRE_THROWS(as<std::error_code>(result));
    REQUIRE_THROWS(as<std::string>(result));
}

