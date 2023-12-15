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

#include "hlib/base.hpp"
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

namespace hlib::test
{

class Case final
{
    HLIB_NOT_COPYABLE(Case);
    HLIB_NOT_MOVABLE(Case);

    typedef void (*Function)();

public:
    char const* const file;
    int const line;
    std::vector<std::string> const tags;
    Function const function;

    bool assessed{ false };

    Case(char const* a_file, int a_line, std::string const& a_tags, Function const& a_function);
};

class Suite final
{
    HLIB_NOT_COPYABLE(Suite);
    HLIB_NOT_MOVABLE(Suite);

public:
    struct Statistics
    {
        std::uint32_t cases{ 0 };
        std::uint32_t cases_failed{ 0 };
        std::uint32_t assertions{ 0 };
        std::uint32_t assertions_failed{ 0 };
    };

    static Suite& get();

public:
    Statistics statistics;

    Suite() = default;

    Case const* currentCase() const;

    void add(Case& tcase);
    void run(std::vector<std::string> const& tags = {});
    void run(std::string const& tags);

private:
    std::vector<Case*> m_test_cases;

    std::string m_current_tag;
    Case* m_current_case{ nullptr };

    void run(std::string tag, Case* test_case);
};

#define HLIB_TEST_CASE_INTERNAL(tags, line) \
    static void hlib_test_case_##line##_function(); \
    static hlib::test::Case hlib_test_case_##line(__FILE__, __LINE__, tags, &hlib_test_case_##line##_function); \
    static void hlib_test_case_##line##_function()

#define HLIB_TEST_CASE(tags) HLIB_TEST_CASE_INTERNAL(tags, __LINE__)

enum class Assertion
{
    Require,
    RequireNothrow,
    RequireThrows
};

enum class Operation
{
    Undefined,
    Equal,          // ==
    NotEqual,       // !=
    Less,           // <
    LessEqual,      // <=
    Greater,        // >
    GreaterEqual    // >=
};

template<typename T>
std::string stringify(T const& value)
{
    if constexpr (true == std::is_enum_v<T>) {
        return std::to_string(static_cast<typename std::underlying_type_t<T>>(value));
    }
    else if constexpr (true == std::is_convertible_v<T, std::string>) {
        return std::string("\"") + value + "\"";
    }
    else if constexpr (true == std::is_same_v<T, bool>) {
        return value ? "true" : "false";
    }
    else {
        return std::to_string(value);
    }
}

struct Expression
{
    Assertion const assertion;
    char const* string;
    char const* file;
    int const line;

    Case const* test_case;

    bool result{ false };
    Operation operation{};
    std::string lhs{};
    std::string rhs{};

    Expression(Assertion a_assertion, char const* a_string, char const* a_file, int a_line)
        : assertion{ a_assertion }
        , string(a_string)
        , file(a_file)
        , line{ a_line }
        , test_case(Suite::get().currentCase())
        {}
};

class Decomposition final
{
    HLIB_NOT_COPYABLE(Decomposition);
    HLIB_NOT_MOVABLE(Decomposition);

private:
    template<typename LHS>
    class Evaluator final
    {
    public:
        LHS lhs;
        Expression& expression;

    #define HLIB_TEST_EVALUATOR_OPERATOR(op, operation_value) \
        template<class RHS> \
        Expression operator op(RHS&& rhs) && \
        { \
            expression.result = (lhs op rhs); \
            expression.operation = operation_value; \
            evaluate(rhs); \
            return std::move(expression); \
        }

        HLIB_TEST_EVALUATOR_OPERATOR(==, Operation::Equal)
        HLIB_TEST_EVALUATOR_OPERATOR(!=, Operation::NotEqual)
        HLIB_TEST_EVALUATOR_OPERATOR(<,  Operation::Less)
        HLIB_TEST_EVALUATOR_OPERATOR(<=, Operation::LessEqual)
        HLIB_TEST_EVALUATOR_OPERATOR(>,  Operation::Greater)
        HLIB_TEST_EVALUATOR_OPERATOR(>=, Operation::GreaterEqual)

    private:
        template<typename RHS>
        void evaluate(RHS const& rhs)
        {
            if (true == expression.result) {
                return;
            }

            expression.lhs = stringify(lhs);
            expression.rhs = stringify(rhs);
        }
    };

public:
    Expression expression;

    Decomposition(Assertion assertion, char const* string, char const* file, int line)
        : expression(assertion, string, file, line)
    {
    }

    template<class LHS>
    Evaluator<LHS> operator ->*(LHS&& lhs) &&
    {
        return Evaluator<LHS>{ std::forward<LHS>(lhs), expression };
    }
};

class AssertionFailed final : std::exception
{
    HLIB_NOT_COPYABLE(AssertionFailed);
    HLIB_NOT_MOVABLE(AssertionFailed);

public:
    Expression expression;

    AssertionFailed(Expression&& a_expression) noexcept;
    char const* what() const noexcept override;

private:
    std::string m_what;
};

#define HLIB_TEST_WARNINGS_PUSH _Pragma("GCC diagnostic push")
#define HLIB_TEST_WARNINGS_POP  _Pragma("GCC diagnostic pop")

#define HLIB_TEST_WARNINGS_DISABLE_PARENTHESES _Pragma("GCC diagnostic ignored \"-Wparentheses\"")

#define HLIB_TEST_ASSERTION_BEGIN ++hlib::test::Suite::get().statistics.assertions;
#define HLIB_TEST_ASSERTION_END

#define HLIB_TEST_DECOMPOSE(assertion, expression) \
    ((hlib::test::Decomposition(assertion, #expression, __FILE__, __LINE__)) ->* expression)

#define HLIB_REQUIRE(expression) \
    do { \
        HLIB_TEST_ASSERTION_BEGIN \
        HLIB_TEST_WARNINGS_PUSH \
        HLIB_TEST_WARNINGS_DISABLE_PARENTHESES \
        hlib::test::Expression expr = HLIB_TEST_DECOMPOSE(hlib::test::Assertion::Require, expression); \
        if (false == expr.result) { throw hlib::test::AssertionFailed(std::move(expr)); } \
        HLIB_TEST_WARNINGS_POP \
        HLIB_TEST_ASSERTION_END \
    } \
    while ((void)0, (false) && static_cast<bool const&>(!!(expression)))

#define HLIB_REQUIRE_NOTHROW(expression) \
    do { \
        HLIB_TEST_ASSERTION_BEGIN \
        hlib::test::Expression expr(hlib::test::Assertion::RequireNothrow, #expression, __FILE__, __LINE__); \
        try { static_cast<void>(expression); expr.result = true; } \
        catch (...) { throw hlib::test::AssertionFailed(std::move(expr)); } \
        HLIB_TEST_ASSERTION_END \
    } \
    while (false)

#define HLIB_REQUIRE_THROWS(expression) \
    do { \
        HLIB_TEST_ASSERTION_BEGIN \
        hlib::test::Expression expr(hlib::test::Assertion::RequireNothrow, #expression, __FILE__, __LINE__); \
        try { static_cast<void>(expression); } \
        catch (...) { expr.result = true; } \
        if (false == expr.result) { throw hlib::test::AssertionFailed(std::move(expr)); } \
        HLIB_TEST_ASSERTION_END \
    } \
    while (false)

std::string to_string(Assertion assertion);
std::string to_string(Operation operation);
std::string to_string(Expression const& expression);

int main(int argc, char* argv[]);

} // namespace hlib::test

