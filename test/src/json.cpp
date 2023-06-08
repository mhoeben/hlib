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
#include "hlib/json.hpp"

using namespace hlib;

namespace
{

std::string const test_json(
R"""({
    "firstName": "John",
    "lastName": "Smith",
    "isAlive": true,
    "age": 27,
    "address": {
        "streetAddress": "21 2nd Street",
        "city": "New York",
        "state": "NY",
        "postalCode": "10021-3100"
    },
    "phoneNumbers": [
        {
            "type": "home",
            "number": "212 555-1234"
        },
        {
            "type": "office",
            "number": "646 555-4567"
        }
    ],
    "children": [
        "Catherine",
        "Thomas",
        "Trevor"
    ],
    "spouse": null
})""");

} // namespace

TEST_CASE("JSON", "[json]")
{
    JSON json;
    json.parse(test_json);

    REQUIRE("John"  == json["firstName"].value());
    REQUIRE("Smith" == json["lastName"].value());
    REQUIRE("true"  == json["isAlive"].value());
    REQUIRE("27"    == json["age"].value());

    JSON address = json["address"];
    REQUIRE("21 2nd Street" == address["streetAddress"].value());
    REQUIRE("New York"      == address["city"].value());
    REQUIRE("NY"            == address["state"].value());
    REQUIRE("10021-3100"    == address["postalCode"].value());

    JSON phone_numbers = json["phoneNumbers"];
    REQUIRE("home"          == phone_numbers[0]["type"].value());
    REQUIRE("212 555-1234"  == phone_numbers[0]["number"].value());
    REQUIRE("office"        == phone_numbers[1]["type"].value());
    REQUIRE("646 555-4567"  == phone_numbers[1]["number"].value());

    JSON children = json["children"];
    REQUIRE("Catherine" == children[0].value());
    REQUIRE("Thomas"    == children[1].value());
    REQUIRE("Trevor"    == children[2].value());

    REQUIRE("null" == json["spouse"].value());

    REQUIRE(JSON::Object    == json.type());
    REQUIRE(JSON::String    == json["firstName"].type());
    REQUIRE(JSON::Bool      == json["isAlive"].type());
    REQUIRE(JSON::Number    == json["age"].type());
    REQUIRE(JSON::Object    == json["address"].type());
    REQUIRE(JSON::Array     == json["phoneNumbers"].type());
    REQUIRE(JSON::Null      == json["spouse"].type());

    REQUIRE(JSON::Undefined == json.at("foo").type());
    REQUIRE(JSON::Undefined == json.at("foo").at("bar").type());

    REQUIRE("John"  == json["firstName"].as<std::string>());
    REQUIRE(true    == json["isAlive"].as<bool>());
    REQUIRE(27      == json["age"].as<int32_t>());

    REQUIRE(8 == json.size());
    REQUIRE(0 == json["firstName"].size());
    REQUIRE(4 == json["address"].size());
    REQUIRE(2 == json["phoneNumbers"].size());
    REQUIRE(3 == json["children"].size());

    JSON foo = json["phoneNumbers"][0];
}

TEST_CASE("JSON Empty Object", "[json]")
{
    JSON json;
    json.parse("{}");
    JSON foo = json.at("foo");
    REQUIRE(true == foo.empty());
    REQUIRE(JSON::Undefined == foo.type());
}

