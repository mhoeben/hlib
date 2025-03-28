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

TEST_CASE("Result Boolean", "[result]")
{
    Result<> result;
    REQUIRE(true == result);
    REQUIRE(false != result);
    REQUIRE(result == true);
    REQUIRE(result != false);

    Result<> error_result = Error(std::bad_alloc());
    REQUIRE(false == error_result);
    REQUIRE(true != error_result);
    REQUIRE(error_result == false);
    REQUIRE(error_result != true);
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

TEST_CASE("Result Then", "[result]")
{
    Error error;

    auto result = Result(13.f).then([](float value) {
        return Result<int>(value);
    }).then([](int value) {
        return Result(std::to_string(value));
    }).otherwise([&](Error const& e) {
        error = e;
    });
    REQUIRE(true == error.empty());
    REQUIRE(true == result.success());
    REQUIRE("13" == result.value());

    result = Result(11.f).then([](float /* value */) {
        return Result<int>(std::bad_alloc());
    }).then([](int value) {
        FAIL();
        return Result(std::to_string(value));
    }).otherwise([&](Error const& e) {
        error = e;
    });

    REQUIRE(false == error.empty());

    error = Error();
    result = Result<>().then([] {
        return Result<std::string>("1971");
    }).otherwise([&](Error const& e) {
        error = e;
    });

    REQUIRE(true == error.empty());
    REQUIRE(true == result.success());
    REQUIRE("1971" == result.value());
}

TEST_CASE("Attempt", "[result]")
{
    auto none = []() {};
    auto result0 = attempt(none);

    auto div = [](int a, int b) -> Result<int> {
        if (0 == b) {
            throw std::domain_error("Division by zero");
        }
        return a / b;
    };

    auto result1 = attempt(div, 6, 3);
    REQUIRE(true == result1.success());
    REQUIRE(2 == result1.value());

    result1 = attempt(div, 2, 0);
    REQUIRE(false == result1.success());
    REQUIRE("Division by zero" == result1.error().what());

    struct Test {
        int setValue(int x) { value = x; return value; }
        int value = 0;
    };

    Test test;
    auto result2 = attempt(&Test::setValue, std::ref(test), 42);
    REQUIRE(true == result2.success());
    REQUIRE(42 == result2.value());
}

