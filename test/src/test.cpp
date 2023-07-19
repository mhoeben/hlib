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
#include "test.hpp"
#include "hlib/container.hpp"
#include "hlib/subprocess.hpp"

#include <vector>

using namespace hlib;

//
// Scratch tests.
//
TEST_CASE("Test")
{
    std::vector<std::string> lhs{ "a", "b", "c" };
    std::vector<std::string> rhs{ "b", "c", "d" };
    std::vector<std::string> r = container::make_union(lhs, rhs);

    REQUIRE(4 == r.size());
}

//
// Public
//
bool test::is_curl_installed()
{
    return 0 == Subprocess("curl", { "--version" }).returnCode();
}

bool test::is_uwsc_installed()
{
    return 0 == Subprocess("uwsc", { "--version" }).returnCode();
}

