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
#include "hlib/error.hpp"

using namespace hlib;

TEST_CASE("Error", "[error]")
{
    Error error;
    REQUIRE(Error::None == error.index());
    REQUIRE(true == !error);
    REQUIRE(false == error);
    REQUIRE(true == error.success());
    REQUIRE(false == error.failed());
    REQUIRE_THROWS(as<std::error_code&>(error));
    REQUIRE_THROWS(as<std::string&>(error));

    std::error_code const error_code = std::make_error_code(std::errc::invalid_argument);

    error = Error(error_code);
    REQUIRE(Error::Code == error.index());
    REQUIRE(false == error.success());
    REQUIRE(true == error.failed());
    REQUIRE(error_code == as<std::error_code>(error));
    REQUIRE_THROWS(as<std::string const&>(error));

    std::string const error_string = "error string";

    error = Error(error_string);
    REQUIRE(Error::String == error.index());
    REQUIRE(false == error.success());
    REQUIRE(true == error.failed());
    REQUIRE_THROWS(as<std::error_code const&>(error));
    REQUIRE(error_string == as<std::string>(error));

    Error copy = error;
    REQUIRE(copy == error);

    Error moved = std::move(copy);
    REQUIRE(moved == error);
    REQUIRE(moved != copy);
}

