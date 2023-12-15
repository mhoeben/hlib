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
#include "hlib/test.hpp"
#include "hlib/container.hpp"
#include "hlib/format.hpp"
#include "hlib/string.hpp"

using namespace hlib;

//
// Implementation
//
namespace
{

std::vector<std::string> parse_tags(std::string const& tags)
{
    return container::for_each(split(tags, ','), [](std::string& tag) { return strip(tag); });
}

} // namespace

//
// Public (test::Case)
//
test::Case::Case(char const* a_file, int a_line, std::string const& a_tags, Function const& a_function)
    : file(a_file)
    , line(a_line)
    , tags(parse_tags(a_tags))
    , function(a_function)
{
    assert(nullptr != function);
    assert(false == tags.empty());

    test::Suite::get().add(*this);
}

//
// Implementation (test::Suite)
//
void test::Suite::run(std::string tag, Case* test_case)
{
    assert(nullptr != test_case);

    try {
        ++statistics.cases;

        m_current_tag = std::move(tag);
        m_current_case = test_case;
        m_current_case->function();
        m_current_case->assessed = true;
        m_current_case = nullptr;
        return;
    }
    catch (AssertionFailed const& e) {
        fmt::print(stderr, "{}:{}: assertion failed: {}\n", e.expression.file, e.expression.line, e.what());
        fmt::print(stderr, "{}:{}: in test case with tags [{}]\n", test_case->file, test_case->line, join(test_case->tags, ", "));
    }
    catch (std::exception const& e) {
        fmt::print(stderr, "{}:{}: uncaught exception '{}' in test case with tags [{}]\n", test_case->file, test_case->line, e.what(), join(test_case->tags, ", "));
    }
    catch (...) {
        fmt::print(stderr, "{}:{}: uncaught exception in test case with tags [{}]\n", test_case->file, test_case->line, join(test_case->tags, ", "));
    }

    ++statistics.assertions_failed;
    ++statistics.cases_failed;
}

//
// Public (test::Suite)
//
test::Suite& test::Suite::get()
{
    static test::Suite singleton;
    return singleton;
}

test::Case const* test::Suite::currentCase() const
{
    return m_current_case;
}

void test::Suite::add(test::Case& test_case)
{
    m_test_cases.push_back(&test_case);
}

void test::Suite::run(std::vector<std::string> const& tags)
{
    statistics = Statistics();

    m_current_tag.clear();
    m_current_case = nullptr;

    container::for_each(m_test_cases, [](Case* test_case) {
        test_case->assessed = false;
    });

    if (true == tags.empty()) {
        for (test::Case* test_case : m_test_cases) {
            run("*", test_case);
        }
        return;
    }

    for (test::Case* test_case : m_test_cases) {
        for (std::string const& tag : tags) {
            if (false == test_case->assessed
             && true == container::contains(test_case->tags, tag)) {
                run(tag, test_case);
            }
        }
    }
}

void test::Suite::run(std::string const& tags)
{
    run(parse_tags(tags));
}

//
// Public (AssertionFailed)
//
test::AssertionFailed::AssertionFailed(Expression&& a_expression) noexcept
    : expression(std::move(a_expression))
    , m_what(to_string(expression))
{
}

char const* test::AssertionFailed::what() const noexcept
{
    return m_what.c_str();
}

//
// Public (test)
//
std::string test::to_string(Assertion assertion)
{
    static const std::unordered_map<Assertion, std::string> table =
    {
        { Assertion::Require,           "HLIB_REQUIRE" },
        { Assertion::RequireNothrow,    "HLIB_REQUIRE_NOTHROW" },
        { Assertion::RequireThrows,     "HLIB_REQUIRE_THROWS" }
    };

    return container::find_or_default(table, assertion, "");
}

std::string test::to_string(Operation operation)
{
    static const std::unordered_map<Operation, std::string> table =
    {
        { Operation::Undefined,     "" },
        { Operation::Equal,         "==" },
        { Operation::NotEqual,      "!=" },
        { Operation::Less,          "<"  },
        { Operation::LessEqual,     "<=" },
        { Operation::Greater,       ">"  },
        { Operation::GreaterEqual,  ">=" }
    };

    return container::find_or_default(table, operation, "");
}

std::string test::to_string(Expression const& expression)
{
    switch (expression.assertion) {
    case Assertion::Require:
        return fmt::format("{}({}) => ({} {} {})",
            to_string(expression.assertion),
            expression.string,
            expression.lhs,
            to_string(expression.operation),
            expression.rhs
        );
    default:
        return fmt::format("{}({})",
            to_string(expression.assertion),
            expression.string
        );
    }
}

int test::main(int argc, char* argv[])
{
    std::vector<std::string> tags;

    for (int i = 1; i < argc; ++i) {
        tags.push_back(argv[i]);
    }

    Suite::get().run(tags);
    return 0;
}
