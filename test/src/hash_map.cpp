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
#include "hlib/c/hash_map.h"

TEST_CASE("HashMap")
{
    char const* key0 = "foo"; char const* value0 = "FOO";
    char const* key1 = "bar"; char const* value1 = "BAR";
    char const* key2 = "baz"; char const* value2 = "BAZ";

    hlib_hash_map_t map;
    REQUIRE(0 == hlib_hash_map_init(&map, 0));

    REQUIRE(0 == hlib_hash_map_insert(&map, key0, strlen(key0), value0));
    REQUIRE(0 == hlib_hash_map_insert(&map, key1, strlen(key1), value1));
    REQUIRE(0 == hlib_hash_map_insert(&map, key2, strlen(key2), value2));
    REQUIRE(4 == map.capacity);
    REQUIRE(3 == map.size);

    REQUIRE(0 == strcmp(value0, static_cast<char const*>(hlib_hash_map_find(&map, key0, strlen(key0)))));
    REQUIRE(0 == strcmp(value1, static_cast<char const*>(hlib_hash_map_find(&map, key1, strlen(key1)))));
    REQUIRE(0 == strcmp(value2, static_cast<char const*>(hlib_hash_map_find(&map, key2, strlen(key2)))));

    REQUIRE(hlib_hash_map_reserve(&map, 8) >= 0);

    REQUIRE(0 == strcmp(value0, static_cast<char const*>(hlib_hash_map_find(&map, key0, strlen(key0)))));
    REQUIRE(0 == strcmp(value1, static_cast<char const*>(hlib_hash_map_find(&map, key1, strlen(key1)))));
    REQUIRE(0 == strcmp(value2, static_cast<char const*>(hlib_hash_map_find(&map, key2, strlen(key2)))));

    REQUIRE(nullptr != hlib_hash_map_erase(&map, key1, strlen(key1)));
    REQUIRE(nullptr == hlib_hash_map_erase(&map, key1, strlen(key1)));

    for (hlib_hash_map_iterator_t it = hlib_hash_map_begin(&map);
             hlib_hash_map_end(&map) != it;
             it = hlib_hash_map_next(&map, it)) {

        char const* key = static_cast<char const*>(hlib_hash_map_get_key(&map, it));
        if (0 == strcmp(key0, key)) { 
            REQUIRE(0 == strcmp(value0, static_cast<char const*>(hlib_hash_map_get_value(&map, it))));
        }
        else if (0 == strcmp(key2, key)) { 
            REQUIRE(0 == strcmp(value2, static_cast<char const*>(hlib_hash_map_get_value(&map, it))));
        }
        else {
            REQUIRE(false);
        }
    }

    hlib_hash_map_free(&map);
}

