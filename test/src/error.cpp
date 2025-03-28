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
#include "hlib/string.hpp"

using namespace hlib;

TEST_CASE("Error Monostate", "[error]")
{
    Error error;
    REQUIRE(true == error.empty());
    REQUIRE_THROWS_AS(error.toss(), std::logic_error);
}

TEST_CASE("Error", "[error]")
{
    Error logic_error(std::logic_error("logic error"));
    REQUIRE(false == logic_error.empty());
    REQUIRE("logic error" == logic_error.what());
    REQUIRE_THROWS_AS(logic_error.toss(), std::logic_error);

    Error runtime_error(std::runtime_error("runtime error"));
    REQUIRE(false == runtime_error.empty());
    REQUIRE("runtime error" == runtime_error.what());
    REQUIRE_THROWS_AS(runtime_error.toss(), std::runtime_error);

    Error system_error(make_system_error(EAGAIN, "system error"));
    REQUIRE(false == system_error.empty());
    REQUIRE(true == contains(system_error.what(), "system error"));
    REQUIRE_THROWS_AS(system_error.toss(), std::system_error);

    Error bad_alloc(std::bad_alloc{});
    REQUIRE(false == bad_alloc.empty());
    REQUIRE("std::bad_alloc" == bad_alloc.what());
    REQUIRE_THROWS_AS(bad_alloc.toss(), std::bad_alloc);
}

TEST_CASE("Error Boolean", "[error]")
{
    Error error;
    REQUIRE(false == error);
    REQUIRE(true != error);
    REQUIRE(error == false);
    REQUIRE(error != true);

    Error bad_alloc(std::bad_alloc{});
    REQUIRE(true == bad_alloc);
    REQUIRE(false != bad_alloc);
    REQUIRE(bad_alloc == true);
    REQUIRE(bad_alloc != false);
}

