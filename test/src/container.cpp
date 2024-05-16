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
#include "hlib/container.hpp"
#include <array>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace hlib;

TEST_CASE("Container Get Size", "[container]")
{
    REQUIRE(5 == container::get_size(std::array<int, 5>{ 1, 2, 3, 4, 5 }));
    REQUIRE(5 == container::get_size(std::forward_list<int>{ 1, 2, 3, 4, 5 }));
    REQUIRE(5 == container::get_size(std::list<int>{ 1, 2, 3, 4, 5 }));
    REQUIRE(5 == container::get_size(std::vector<int>{ 1, 2, 3, 4, 5 }));

    REQUIRE(5 == container::get_size(std::map<int, int>{ { 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 }, { 5, 5 } }));
    REQUIRE(5 == container::get_size(std::set<int>{ 1, 2, 3, 4, 5 }));
    REQUIRE(5 == container::get_size(std::multimap<int, int>{ { 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 }, { 5, 5 } }));
    REQUIRE(5 == container::get_size(std::multiset<int>{ 1, 2, 3, 4, 5 }));
    REQUIRE(5 == container::get_size(std::unordered_map<int, int>{ { 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 }, { 5, 5 } }));
    REQUIRE(5 == container::get_size(std::unordered_set<int>{ 1, 2, 3, 4, 5 }));
    REQUIRE(5 == container::get_size(std::unordered_multimap<int, int>{ { 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 }, { 5, 5 } }));
    REQUIRE(5 == container::get_size(std::unordered_multiset<int>{ 1, 2, 3, 4, 5 }));
}

TEST_CASE("Container Contains", "[container]")
{
    std::array<int, 3> array_int{ 1, 2, 3 };
    REQUIRE(true  == container::contains(array_int, 1));
    REQUIRE(true  == container::contains(array_int, 2));
    REQUIRE(true  == container::contains(array_int, 3));
    REQUIRE(false == container::contains(array_int, 4));

    std::forward_list<int> fwd_list_int{ 1, 2, 3 };
    REQUIRE(true  == container::contains(fwd_list_int, 1));
    REQUIRE(true  == container::contains(fwd_list_int, 2));
    REQUIRE(true  == container::contains(fwd_list_int, 3));
    REQUIRE(false == container::contains(fwd_list_int, 4));

    std::list<int> list_int{ 1, 2, 3 };
    REQUIRE(true  == container::contains(list_int, 1));
    REQUIRE(true  == container::contains(list_int, 2));
    REQUIRE(true  == container::contains(list_int, 3));
    REQUIRE(false == container::contains(list_int, 4));

    std::vector<int> vector_int{ 1, 2, 3 };
    REQUIRE(true  == container::contains(vector_int, 1));
    REQUIRE(true  == container::contains(vector_int, 2));
    REQUIRE(true  == container::contains(vector_int, 3));
    REQUIRE(false == container::contains(vector_int, 4));

    std::set<int> set_int{ 1, 2, 3 };
    REQUIRE(true  == container::contains(set_int, 1));
    REQUIRE(true  == container::contains(set_int, 2));
    REQUIRE(true  == container::contains(set_int, 3));
    REQUIRE(false == container::contains(set_int, 4));

    std::map<int, int> map_int{{ 1, 1 }, { 2, 2 }, { 3, 3 }};
    REQUIRE(true  == container::contains(map_int, { 1, 1 }));
    REQUIRE(false == container::contains(map_int, { 1, 2 }));
    REQUIRE(true  == container::contains(map_int, { 2, 2 }));
    REQUIRE(false == container::contains(map_int, { 2, 3 }));
    REQUIRE(true  == container::contains(map_int, { 3, 3 }));
    REQUIRE(false == container::contains(map_int, { 3, 4 }));
    REQUIRE(false == container::contains(map_int, { 4, 1 }));

    std::multimap<int, int> multimap_int{{ 1, 1 }, { 1, 2 }, { 2, 1 }, { 2, 2 }, { 3, 1}, { 3, 2 }};
    REQUIRE(true  == container::contains(multimap_int, { 1, 1 }));
    REQUIRE(true  == container::contains(multimap_int, { 1, 2 }));
    REQUIRE(false == container::contains(multimap_int, { 1, 3 }));
    REQUIRE(true  == container::contains(multimap_int, { 2, 1 }));
    REQUIRE(true  == container::contains(multimap_int, { 2, 2 }));
    REQUIRE(false == container::contains(multimap_int, { 2, 3 }));
    REQUIRE(true  == container::contains(multimap_int, { 3, 1 }));
    REQUIRE(true  == container::contains(multimap_int, { 3, 2 }));
    REQUIRE(false == container::contains(multimap_int, { 3, 3 }));
    REQUIRE(false == container::contains(multimap_int, { 4, 1 }));

    std::multiset<int> multiset_int{ 1, 1, 2, 2, 3, 3 };
    REQUIRE(true  == container::contains(multiset_int, 1));
    REQUIRE(true  == container::contains(multiset_int, 2));
    REQUIRE(true  == container::contains(multiset_int, 3));
    REQUIRE(false == container::contains(multiset_int, 4));

    std::unordered_set<int> u_set_int{ 1, 2, 3 };
    REQUIRE(true  == container::contains(u_set_int, 1));
    REQUIRE(true  == container::contains(u_set_int, 2));
    REQUIRE(true  == container::contains(u_set_int, 3));
    REQUIRE(false == container::contains(u_set_int, 4));

    std::unordered_map<int, int> u_map_int{{ 1, 1 }, { 2, 2 }, { 3, 3 }};
    REQUIRE(true  == container::contains(u_map_int, { 1, 1 }));
    REQUIRE(false == container::contains(u_map_int, { 1, 2 }));
    REQUIRE(true  == container::contains(u_map_int, { 2, 2 }));
    REQUIRE(false == container::contains(u_map_int, { 2, 3 }));
    REQUIRE(true  == container::contains(u_map_int, { 3, 3 }));
    REQUIRE(false == container::contains(u_map_int, { 3, 4 }));
    REQUIRE(false == container::contains(u_map_int, { 4, 1 }));

    std::unordered_multimap<int, int> u_multimap_int{{ 1, 1 }, { 1, 2 }, { 2, 1 }, { 2, 2 }, { 3, 1}, { 3, 2 }};
    REQUIRE(true  == container::contains(u_multimap_int, { 1, 1 }));
    REQUIRE(true  == container::contains(u_multimap_int, { 1, 2 }));
    REQUIRE(false == container::contains(u_multimap_int, { 1, 3 }));
    REQUIRE(true  == container::contains(u_multimap_int, { 2, 1 }));
    REQUIRE(true  == container::contains(u_multimap_int, { 2, 2 }));
    REQUIRE(false == container::contains(u_multimap_int, { 2, 3 }));
    REQUIRE(true  == container::contains(u_multimap_int, { 3, 1 }));
    REQUIRE(true  == container::contains(u_multimap_int, { 3, 2 }));
    REQUIRE(false == container::contains(u_multimap_int, { 3, 3 }));
    REQUIRE(false == container::contains(u_multimap_int, { 4, 1 }));

    std::unordered_multiset<int> u_multiset_int{ 1, 1, 2, 2, 3, 3 };
    REQUIRE(true  == container::contains(u_multiset_int, 1));
    REQUIRE(true  == container::contains(u_multiset_int, 2));
    REQUIRE(true  == container::contains(u_multiset_int, 3));
    REQUIRE(false == container::contains(u_multiset_int, 4));
}

TEST_CASE("Container Replace", "[container]")
{
    std::unordered_map<std::string, std::string> u_map
    {
        { "foo", "FOO" },
        { "bar", "BAR" },
        { "baz", "BAZ" }
    };

    REQUIRE("xyz" == container::replace(u_map, "bar", "xyz"));
    REQUIRE("xyz" == u_map.at("bar"));
}

TEST_CASE("Container Make Union", "[container]")
{
    auto test_make_union = [](auto expected, auto lhs, auto rhs)
    {
        auto result = container::make_union(lhs, rhs);
        REQUIRE(container::get_size(expected) == container::get_size(result));

        for (auto const& value : expected) {
            REQUIRE(true == container::contains(result, value));
        }
    };

    typedef std::forward_list<int> fwd_list_int;
    test_make_union(fwd_list_int{ 1, 2, 3, 4 }, fwd_list_int{ 1, 2, 3 }, fwd_list_int{ 2, 3, 4 });
    test_make_union(fwd_list_int{ 1, 2, 3, 4 }, fwd_list_int{ 1, 1, 2, 2, 3, 3 }, fwd_list_int{ 2, 3, 4 });
    test_make_union(fwd_list_int{ 1, 2, 3, 4 }, fwd_list_int{ 1, 1, 2, 2, 3, 3 }, fwd_list_int{ 2, 2, 3, 3, 4, 4 });
    test_make_union(fwd_list_int{ 1, 2, 3, 4 }, fwd_list_int{ 1, 2, 3 },          fwd_list_int{ 2, 2, 3, 3, 4, 4 });

    typedef std::list<int> list_int;
    test_make_union(list_int{ 1, 2, 3, 4 }, list_int{ 1, 2, 3 }, list_int{ 2, 3, 4 });
    test_make_union(list_int{ 1, 2, 3, 4 }, list_int{ 1, 1, 2, 2, 3, 3 }, list_int{ 2, 3, 4 });
    test_make_union(list_int{ 1, 2, 3, 4 }, list_int{ 1, 1, 2, 2, 3, 3 }, list_int{ 2, 2, 3, 3, 4, 4 });
    test_make_union(list_int{ 1, 2, 3, 4 }, list_int{ 1, 2, 3 },          list_int{ 2, 2, 3, 3, 4, 4 });

    typedef std::vector<int> vector_int;
    test_make_union(vector_int{ 1, 2, 3, 4 }, vector_int{ 1, 2, 3 }, vector_int{ 2, 3, 4 });
    test_make_union(vector_int{ 1, 2, 3, 4 }, vector_int{ 1, 1, 2, 2, 3, 3 }, vector_int{ 2, 3, 4 });
    test_make_union(vector_int{ 1, 2, 3, 4 }, vector_int{ 1, 1, 2, 2, 3, 3 }, vector_int{ 2, 2, 3, 3, 4, 4 });
    test_make_union(vector_int{ 1, 2, 3, 4 }, vector_int{ 1, 2, 3 },          vector_int{ 2, 2, 3, 3, 4, 4 });

    typedef std::map<int, int> map_int;
    test_make_union(map_int{{ 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 }}, map_int{{ 1, 1 }, { 2, 2 }, { 3, 3 }}, map_int{{ 2, 2 }, { 3, 3 }, { 4, 4 }});
    test_make_union(map_int{{ 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 }}, map_int{{ 1, 1 }, { 1, 1 }, { 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }}, map_int{{ 2, 2 }, { 3, 3 }, { 4, 4 }});
    test_make_union(map_int{{ 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 }}, map_int{{ 1, 1 }, { 1, 1 }, { 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }}, map_int{{ 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }, { 4, 4 }, {4, 4 }});
    test_make_union(map_int{{ 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 }}, map_int{{ 1, 1 }, { 2, 2 }, { 3, 3 }},                               map_int{{ 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }, { 4, 4 }, {4, 4 }});

    typedef std::set<int> set_int;
    test_make_union(set_int{ 1, 2, 3, 4 }, set_int{ 1, 2, 3 }, set_int{ 2, 3, 4 });

    typedef std::unordered_map<int, int> u_map_int;
    test_make_union(u_map_int{{ 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 }}, u_map_int{{ 1, 1 }, { 2, 2 }, { 3, 3 }}, u_map_int{{ 2, 2 }, { 3, 3 }, { 4, 4 }});
    test_make_union(u_map_int{{ 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 }}, u_map_int{{ 1, 1 }, { 1, 1 }, { 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }}, u_map_int{{ 2, 2 }, { 3, 3 }, { 4, 4 }});
    test_make_union(u_map_int{{ 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 }}, u_map_int{{ 1, 1 }, { 1, 1 }, { 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }}, u_map_int{{ 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }, { 4, 4 }, {4, 4 }});
    test_make_union(u_map_int{{ 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 }}, u_map_int{{ 1, 1 }, { 2, 2 }, { 3, 3 }},                               u_map_int{{ 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }, { 4, 4 }, {4, 4 }});

    typedef std::unordered_set<int> u_set_int;
    test_make_union(u_set_int{ 1, 2, 3, 4 }, u_set_int{ 1, 2, 3 }, u_set_int{ 2, 3, 4 });
}

TEST_CASE("Container Make Intersection", "[container]")
{
    auto test_make_intersection = [](auto expected, auto lhs, auto rhs)
    {
        auto result = container::make_intersection(lhs, rhs);
        REQUIRE(container::get_size(expected) == container::get_size(result));

        for (auto const& value : expected) {
            REQUIRE(true == container::contains(result, value));
        }
    };

    typedef std::forward_list<int> fwd_list_int;
    test_make_intersection(fwd_list_int{ 2, 3 }, fwd_list_int{ 1, 2, 3 }, fwd_list_int{ 2, 3, 4 });
    test_make_intersection(fwd_list_int{ 2, 3 }, fwd_list_int{ 1, 1, 2, 2, 3, 3 }, fwd_list_int{ 2, 3, 4 });
    test_make_intersection(fwd_list_int{ 2, 3 }, fwd_list_int{ 1, 1, 2, 2, 3, 3 }, fwd_list_int{ 2, 2, 3, 3, 4, 4 });
    test_make_intersection(fwd_list_int{ 2, 3 }, fwd_list_int{ 1, 2, 3 },          fwd_list_int{ 2, 2, 3, 3, 4, 4 });

    typedef std::list<int> list_int;
    test_make_intersection(list_int{ 2, 3 }, list_int{ 1, 2, 3 }, list_int{ 2, 3, 4 });
    test_make_intersection(list_int{ 2, 3 }, list_int{ 1, 1, 2, 2, 3, 3 }, list_int{ 2, 3, 4 });
    test_make_intersection(list_int{ 2, 3 }, list_int{ 1, 1, 2, 2, 3, 3 }, list_int{ 2, 2, 3, 3, 4, 4 });
    test_make_intersection(list_int{ 2, 3 }, list_int{ 1, 2, 3 },          list_int{ 2, 2, 3, 3, 4, 4 });

    typedef std::vector<int> vector_int;
    test_make_intersection(vector_int{ 2, 3 }, vector_int{ 1, 2, 3 }, vector_int{ 2, 3, 4 });
    test_make_intersection(vector_int{ 2, 3 }, vector_int{ 1, 1, 2, 2, 3, 3 }, vector_int{ 2, 3, 4 });
    test_make_intersection(vector_int{ 2, 3 }, vector_int{ 1, 1, 2, 2, 3, 3 }, vector_int{ 2, 2, 3, 3, 4, 4 });
    test_make_intersection(vector_int{ 2, 3 }, vector_int{ 1, 2, 3 },          vector_int{ 2, 2, 3, 3, 4, 4 });

    typedef std::map<int, int> map_int;
    test_make_intersection(map_int{{ 2, 2 }, { 3, 3 }}, map_int{{ 1, 1 }, { 2, 2 }, { 3, 3 }}, map_int{{ 2, 2 }, { 3, 3 }, { 4, 4 }});
    test_make_intersection(map_int{{ 2, 2 }, { 3, 3 }}, map_int{{ 1, 1 }, { 1, 1 }, { 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }}, map_int{{ 2, 2 }, { 3, 3 }, { 4, 4 }});
    test_make_intersection(map_int{{ 2, 2 }, { 3, 3 }}, map_int{{ 1, 1 }, { 1, 1 }, { 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }}, map_int{{ 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }, { 4, 4 }, { 4, 4 }});
    test_make_intersection(map_int{{ 2, 2 }, { 3, 3 }}, map_int{{ 1, 1 }, { 2, 2 }, { 3, 3 }},                               map_int{{ 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }, { 4, 4 }, { 4, 4 }});

    typedef std::set<int> set_int;
    test_make_intersection(set_int{ 2, 3 }, set_int{ 1, 2, 3 }, set_int{ 2, 3, 4 });

    typedef std::unordered_map<int, int> u_map_int;
    test_make_intersection(u_map_int{{ 2, 2 }, { 3, 3 }}, u_map_int{{ 1, 1 }, { 2, 2 }, { 3, 3 }}, u_map_int{{ 2, 2 }, { 3, 3 }, { 4, 4 }});
    test_make_intersection(u_map_int{{ 2, 2 }, { 3, 3 }}, u_map_int{{ 1, 1 }, { 1, 1 }, { 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }}, u_map_int{{ 2, 2 }, { 3, 3 }, { 4, 4 }});
    test_make_intersection(u_map_int{{ 2, 2 }, { 3, 3 }}, u_map_int{{ 1, 1 }, { 1, 1 }, { 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }}, u_map_int{{ 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }, { 4, 4 }, { 4, 4 }});
    test_make_intersection(u_map_int{{ 2, 2 }, { 3, 3 }}, u_map_int{{ 1, 1 }, { 2, 2 }, { 3, 3 }},                               u_map_int{{ 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }, { 4, 4 }, { 4, 4 }});

    typedef std::unordered_set<int> u_set_int;
    test_make_intersection(u_set_int{ 2, 3 }, u_set_int{ 1, 2, 3 }, u_set_int{ 2, 3, 4 });
}

TEST_CASE("Container Make Complement", "[container]")
{
    auto test_make_complement = [](auto expected, auto lhs, auto rhs)
    {
        auto result = container::make_complement(lhs, rhs);
        REQUIRE(container::get_size(expected) == container::get_size(result));

        for (auto const& value : expected) {
            REQUIRE(true == container::contains(result, value));
        }
    };

    typedef std::forward_list<int> fwd_list_int;
    test_make_complement(fwd_list_int{ 1, 4 }, fwd_list_int{ 1, 2, 3 }, fwd_list_int{ 2, 3, 4 });
    test_make_complement(fwd_list_int{ 1, 4 }, fwd_list_int{ 1, 1, 2, 2, 3, 3 }, fwd_list_int{ 2, 3, 4 });
    test_make_complement(fwd_list_int{ 1, 4 }, fwd_list_int{ 1, 1, 2, 2, 3, 3 }, fwd_list_int{ 2, 2, 3, 3, 4, 4 });
    test_make_complement(fwd_list_int{ 1, 4 }, fwd_list_int{ 1, 2, 3 },          fwd_list_int{ 2, 2, 3, 3, 4, 4 });

    typedef std::list<int> list_int;
    test_make_complement(list_int{ 1, 4 }, list_int{ 1, 2, 3 }, list_int{ 2, 3, 4 });
    test_make_complement(list_int{ 1, 4 }, list_int{ 1, 1, 2, 2, 3, 3 }, list_int{ 2, 3, 4 });
    test_make_complement(list_int{ 1, 4 }, list_int{ 1, 1, 2, 2, 3, 3 }, list_int{ 2, 2, 3, 3, 4, 4 });
    test_make_complement(list_int{ 1, 4 }, list_int{ 1, 2, 3 },          list_int{ 2, 2, 3, 3, 4, 4 });

    typedef std::vector<int> vector_int;
    test_make_complement(vector_int{ 1, 4 }, vector_int{ 1, 2, 3 }, vector_int{ 2, 3, 4 });
    test_make_complement(vector_int{ 1, 4 }, vector_int{ 1, 1, 2, 2, 3, 3 }, vector_int{ 2, 3, 4 });
    test_make_complement(vector_int{ 1, 4 }, vector_int{ 1, 1, 2, 2, 3, 3 }, vector_int{ 2, 2, 3, 3, 4, 4 });
    test_make_complement(vector_int{ 1, 4 }, vector_int{ 1, 2, 3 },          vector_int{ 2, 2, 3, 3, 4, 4 });

    typedef std::map<int, int> map_int;
    test_make_complement(map_int{{ 1, 1 }, { 4, 4 }}, map_int{{ 1, 1 }, { 2, 2 }, { 3, 3 }}, map_int{{ 2, 2 }, { 3, 3 }, { 4, 4 }});
    test_make_complement(map_int{{ 1, 1 }, { 4, 4 }}, map_int{{ 1, 1 }, { 1, 1 }, { 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }}, map_int{{ 2, 2 }, { 3, 3 }, { 4, 4 }});
    test_make_complement(map_int{{ 1, 1 }, { 4, 4 }}, map_int{{ 1, 1 }, { 1, 1 }, { 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }}, map_int{{ 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }, { 4, 4 }, {4, 4 }});
    test_make_complement(map_int{{ 1, 1 }, { 4, 4 }}, map_int{{ 1, 1 }, { 2, 2 }, { 3, 3 }},                               map_int{{ 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }, { 4, 4 }, {4, 4 }});

    typedef std::set<int> set_int;
    test_make_complement(set_int{ 1, 4 }, set_int{ 1, 2, 3 }, set_int{ 2, 3, 4 });

    typedef std::unordered_map<int, int> u_map_int;
    test_make_complement(u_map_int{{ 1, 1 }, { 4, 4 }}, u_map_int{{ 1, 1 }, { 2, 2 }, { 3, 3 }}, u_map_int{{ 2, 2 }, { 3, 3 }, { 4, 4 }});
    test_make_complement(u_map_int{{ 1, 1 }, { 4, 4 }}, u_map_int{{ 1, 1 }, { 1, 1 }, { 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }}, u_map_int{{ 2, 2 }, { 3, 3 }, { 4, 4 }});
    test_make_complement(u_map_int{{ 1, 1 }, { 4, 4 }}, u_map_int{{ 1, 1 }, { 1, 1 }, { 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }}, u_map_int{{ 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }, { 4, 4 }, {4, 4 }});
    test_make_complement(u_map_int{{ 1, 1 }, { 4, 4 }}, u_map_int{{ 1, 1 }, { 2, 2 }, { 3, 3 }},                               u_map_int{{ 2, 2 }, { 2, 2 }, { 3, 3 }, { 3, 3 }, { 4, 4 }, {4, 4 }});

    typedef std::unordered_set<int> u_set_int;
    test_make_complement(u_set_int{ 1, 4 }, u_set_int{ 1, 2, 3 }, u_set_int{ 2, 3, 4 });
}

