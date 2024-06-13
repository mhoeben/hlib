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
#include "hlib/buffer.hpp"
#include "hlib/type_traits.hpp"
#include <array>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace hlib;

TEST_CASE("Has Reserve Method", "[types]")
{
    REQUIRE(false == has_reserve_method<std::array<int, 1>>::value);
    REQUIRE(false == has_reserve_method<std::forward_list<int>>::value);
    REQUIRE(false == has_reserve_method<std::list<int>>::value);
    REQUIRE(true == has_reserve_method<std::vector<int>>::value);

    REQUIRE(false == has_reserve_method<std::map<int, int>>::value);
    REQUIRE(false == has_reserve_method<std::set<int>>::value);
    REQUIRE(false == has_reserve_method<std::multimap<int, int>>::value);
    REQUIRE(false == has_reserve_method<std::multiset<int>>::value);
    REQUIRE(true == has_reserve_method<std::unordered_map<int, int>>::value);
    REQUIRE(true == has_reserve_method<std::unordered_set<int>>::value);
    REQUIRE(true == has_reserve_method<std::unordered_multimap<int, int>>::value);
    REQUIRE(true == has_reserve_method<std::unordered_multiset<int>>::value);

    REQUIRE(true == has_reserve_method<Buffer>::value);
    REQUIRE(true == has_reserve_method<std::string>::value);
}

TEST_CASE("Has Size Method", "[types]")
{
    struct Foo
    {
        std::size_t size() const noexcept;
    };

    struct FooBar : Foo
    {
    };

    struct Baz
    {
    };

    REQUIRE(true == has_size_method<Foo>::value);
    REQUIRE(true == has_size_method<FooBar>::value);
    REQUIRE(false == has_size_method<Baz>::value);

    REQUIRE(true == has_size_method<std::array<int, 1>>::value);
    REQUIRE(false == has_size_method<std::forward_list<int>>::value);
    REQUIRE(true == has_size_method<std::list<int>>::value);
    REQUIRE(true == has_size_method<std::vector<int>>::value);

    REQUIRE(true == has_size_method<Buffer>::value);
    REQUIRE(true == has_size_method<std::string>::value);
}

TEST_CASE("Is Pair Iterator", "[types]")
{
    REQUIRE(false == is_pair_iterator<std::array<int, 1>::iterator>::value);
    REQUIRE(false == is_pair_iterator<std::forward_list<int>::iterator>::value);
    REQUIRE(false == is_pair_iterator<std::list<int>::iterator>::value);
    REQUIRE(false == is_pair_iterator<std::vector<int>::iterator>::value);

    REQUIRE(true  == is_pair_iterator<std::map<int, int>::iterator>::value);
    REQUIRE(false == is_pair_iterator<std::set<int>::iterator>::value);
    REQUIRE(true  == is_pair_iterator<std::multimap<int, int>::iterator>::value);
    REQUIRE(false == is_pair_iterator<std::multiset<int>::iterator>::value);
    REQUIRE(true  == is_pair_iterator<std::unordered_map<int, int>::iterator>::value);
    REQUIRE(false == is_pair_iterator<std::unordered_set<int>::iterator>::value);
    REQUIRE(true  == is_pair_iterator<std::unordered_multimap<int, int>::iterator>::value);
    REQUIRE(false == is_pair_iterator<std::unordered_multiset<int>::iterator>::value);
}

TEST_CASE("Is Associative", "[types]")
{
    REQUIRE(false == is_associative<std::array<int, 1>>::value);
    REQUIRE(false == is_associative<std::forward_list<int>>::value);
    REQUIRE(false == is_associative<std::list<int>>::value);
    REQUIRE(false == is_associative<std::vector<int>>::value);

    REQUIRE(true == is_associative<std::map<int, int>>::value);
    REQUIRE(true == is_associative<std::set<int>>::value);
    REQUIRE(true == is_associative<std::multimap<int, int>>::value);
    REQUIRE(true == is_associative<std::multiset<int>>::value);
    REQUIRE(true == is_associative<std::unordered_map<int, int>>::value);
    REQUIRE(true == is_associative<std::unordered_set<int>>::value);
}

TEST_CASE("Is Smart Pointer", "[types]")
{
    REQUIRE(false == is_unique_ptr<int>::value);
    REQUIRE(true  == is_unique_ptr<std::unique_ptr<int>>::value);
    REQUIRE(false == is_unique_ptr<std::shared_ptr<int>>::value);
    REQUIRE(false == is_unique_ptr<std::weak_ptr<int>>::value);

    REQUIRE(false == is_shared_ptr<int>::value);
    REQUIRE(false == is_shared_ptr<std::unique_ptr<int>>::value);
    REQUIRE(true  == is_shared_ptr<std::shared_ptr<int>>::value);
    REQUIRE(false == is_shared_ptr<std::weak_ptr<int>>::value);

    REQUIRE(false == is_weak_ptr<int>::value);
    REQUIRE(false == is_weak_ptr<std::unique_ptr<int>>::value);
    REQUIRE(false == is_weak_ptr<std::shared_ptr<int>>::value);
    REQUIRE(true  == is_weak_ptr<std::weak_ptr<int>>::value);

    REQUIRE(false == is_smart_ptr<int>::value);
    REQUIRE(true  == is_smart_ptr<std::unique_ptr<int>>::value);
    REQUIRE(true  == is_smart_ptr<std::shared_ptr<int>>::value);
    REQUIRE(true  == is_smart_ptr<std::weak_ptr<int>>::value);
}

