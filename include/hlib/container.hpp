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
#include "hlib/type_traits.hpp"
#include <algorithm>
#include <iterator>
#include <set>
#include <utility>

namespace hlib
{
namespace container
{
//
// Functions
//
template<typename T>
std::size_t get_size(T const& container)
{
    if constexpr (true == has_size_method<T>::value) {
        return container.size();
    } else if constexpr (std::is_same_v<typename T::iterator, decltype(container.begin())>) {
        return std::distance(container.begin(), container.end());
    } else {
        return std::distance(std::begin(container), std::end(container));
    }
}


template<typename T>
bool contains(T const& container, typename T::value_type const& value)
{
    if constexpr (true == is_associative<T>::value) {
        if constexpr (true == is_pair_iterator<typename T::const_iterator>::value) {
            auto range = container.equal_range(value.first);
            for (auto it = range.first; it != range.second; ++it) {
                if (*it == value) {
                    return true;
                }
            }
            return false;
        }
        else {
            return container.end() != container.find(value);
        }
    }
    else {
        return container.end() != std::find(container.begin(), container.end(), value);
    }
}

template<typename T>
typename T::mapped_type& replace(T& container, typename T::key_type&& key, typename T::mapped_type&& value)
{
    static_assert(true == is_associative<T>::value);

    auto it = container.find(key);
    if (container.end() == it) {
        container.emplace_hint(it, std::forward<typename T::key_type>(key), std::forward<typename T::mapped_type>(value));
    }
    else {
        it->second = std::forward<typename T::mapped_type>(value);
    }

    return it->second;
}

template<typename T>
typename T::mapped_type const& find_or_default(T const& container, typename T::key_type const& key, typename T::mapped_type const& default_value)
{
    static_assert(true == is_associative<T>::value);

    auto it = container.find(key);
    return container.end() != it ? it->second : default_value;
}

template<typename T, typename F>
T& for_each(T& container, F function)
{
    for (auto& value : container) {
        function(value);
    }

    return container;
}

template<typename T, typename F>
T for_each(T&& container, F function)
{
    for (auto& value : container) {
        function(value);
    }

    return container;
}

template<typename T>
T make_union(T const& lhs, T const& rhs)
{
    std::set<typename T::value_type> set;

    set.insert(lhs.begin(), lhs.end());
    set.insert(rhs.begin(), rhs.end());

    return T(set.begin(), set.end());
}

template<typename T>
T make_intersection(T const& lhs, T const& rhs)
{
    std::set<typename T::value_type> set(lhs.begin(), lhs.end());

    T result;
    if constexpr (true == has_reserve_method<T>::value) {
        result.reserve(std::min(lhs.size(), rhs.size()));
    }

    for (auto const& value : rhs) {
        if (set.count(value) > 0) {
            if constexpr (true == has_push_back_method<T>::value) {
                result.push_back(value);
            }
            else if constexpr (true == has_push_front_method<T>::value) {
                result.push_front(value);
            }
            else {
                result.insert(value);
            }

            set.erase(value);
        }
    }

    return result;
}

template<typename T>
T make_complement(T const& lhs, T const& rhs)
{
    std::set<typename T::value_type> lhs_set(lhs.begin(), lhs.end());
    std::set<typename T::value_type> rhs_set(rhs.begin(), rhs.end());
    std::set<typename T::value_type> result;

    for (auto& value : lhs) {
        if (0 == rhs_set.count(value)) {
            result.emplace(std::move(value));
        }
    }

    for (auto& value : rhs) {
        if (0 == lhs_set.count(value)) {
            result.emplace(std::move(value));
        }
    }

    return T(result.begin(), result.end());
}

} // namespace container
} // namespace hlib

