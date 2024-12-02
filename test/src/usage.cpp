//
// MIT License
//
// Copyright (c) 2024 Maarten Hoeben
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
#include "hlib/usage.hpp"

using namespace hlib;

TEST_CASE("Usage", "[usage]")
{
    Usage usage{
        "Usage test description",
        {
            { 's', "string",    "String",   "<string>" },
            {  0 , "integer",   "Integer",  "<int>",    0 },
            { 'f', "",          "Float",    "<float>",  0.0 },
            { 'b', "",          "Boolean",  "<bool>",   false },

            { 'S', "",          "String Default",   "<string>", "Hello World!" },
            { 'I', "",          "Integer Default",  "<int>",    13 },
            { 'F', "",          "Float Default",    "<float>",  11.0 },
            { 'B', "",          "Boolean Default",  "<bool>",   true },
        },
        {
            { "ARG0",       "Argument 0" },
            { "ARG1",       "Argument 1", true },
            { "ARG2",       "Argument 2" },
            { "ARGUMENT3",  "Argument 3", true }
        },
        true
    };

    printf("%s", usage.toString("test").c_str());

    std::vector<char const*> args = { "foo", "-s", "bar", "--integer", "1971", "-f=13.11", "-b", "true", "foo",  };
    REQUIRE(true == usage.parse(static_cast<int>(args.size()), args.data()).success());

    REQUIRE(true == usage.has('s'));
    REQUIRE(true == usage.has("string"));
    REQUIRE("bar" == *usage.get<std::string>('s'));
    REQUIRE("bar" == *usage.get<std::string>("string"));

    REQUIRE(false == usage.has('i'));
    REQUIRE(true == usage.has("integer"));
    REQUIRE(std::nullopt == usage.get<int>('i'));
    REQUIRE(1971 == *usage.get<int>("integer"));

    REQUIRE(true == usage.has('f'));
    REQUIRE(false == usage.has("float"));
    REQUIRE(13.11 == *usage.get<double>('f'));
    REQUIRE(std::nullopt == usage.get<double>("float"));

    REQUIRE(true == usage.has('b'));
    REQUIRE(false == usage.has("bool"));
    REQUIRE(true == *usage.get<bool>('b'));

    REQUIRE(false == usage.has('S'));
    REQUIRE("Hello World!" == *usage.get('S'));

    REQUIRE(false == usage.has('I'));
    REQUIRE(13 == *usage.get<int>('I'));

    REQUIRE(false == usage.has('F'));
    REQUIRE(11.0 == *usage.get<double>('F'));

    REQUIRE(false == usage.has('B'));
    REQUIRE(true == *usage.get<bool>('B'));

    REQUIRE("foo" == usage.get(0));
}

