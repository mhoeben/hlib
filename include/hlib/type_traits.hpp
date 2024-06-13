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
#include <memory>
#include <type_traits>

namespace hlib
{

// has_equal_range_method
template<typename T, typename = void>
struct has_equal_range_method : std::false_type {};

template<typename T>
struct has_equal_range_method<T, std::void_t<decltype(std::declval<T>().equal_range(typename T::key_type{}))>> : std::true_type {};

// has_push_front_method
template<typename T, typename = void>
struct has_push_front_method : std::false_type {};

template<typename T>
struct has_push_front_method<T, std::void_t<decltype(std::declval<T>().push_front(typename T::value_type{}))>> : std::true_type {};

// has_push_back_method
template<typename T, typename = void>
struct has_push_back_method : std::false_type {};

template<typename T>
struct has_push_back_method<T, std::void_t<decltype(std::declval<T>().push_back(typename T::value_type{}))>> : std::true_type {};

// has_reserve_method
template<typename T, typename = void>
struct has_reserve_method : std::false_type {};

template<typename T>
struct has_reserve_method<T, std::void_t<decltype(std::declval<T>().reserve(std::size_t{}))>> : std::true_type {};

// has_resize_method
template<typename T, typename = void>
struct has_resize_method : std::false_type {};

template<typename T>
struct has_resize_method<T, std::void_t<decltype(std::declval<T>().resize(std::size_t{}))>> : std::true_type {};

// has_size_method
template<typename T, typename = void>
struct has_size_method : std::false_type {};

template<typename T>
struct has_size_method<T, std::void_t<decltype(std::declval<T>().size())>> : std::true_type {};

// has_empty_method
template<typename T, typename = void>
struct has_empty_method : std::false_type {};

template<typename T>
struct has_empty_method<T, std::void_t<decltype(std::declval<T>().empty())>> : std::true_type {};

// has_data_method
template<typename T, typename = void>
struct has_data_method : std::false_type {};

template<typename T>
struct has_data_method<T, std::void_t<decltype(std::declval<T>().data())>> : std::true_type {};

// is_pair_iterator
template <typename T, typename = void>
struct is_pair_iterator : std::false_type {};

template <typename T>
struct is_pair_iterator<T, std::void_t<
    typename T::value_type,
    typename T::value_type::first_type,
    typename T::value_type::second_type>> : std::is_same<
            typename T::value_type,
            std::pair<typename T::value_type::first_type, typename T::value_type::second_type >
        > {};
template<bool v, typename T>
struct bool_type
{ 
    static constexpr bool value = v; 
};

// is_associative
template <typename Container, typename = void>
struct is_associative : std::false_type {};

template <typename Container>
struct is_associative<Container, std::void_t<typename Container::key_type>> : std::true_type {};

// is_unique_ptr
template<typename T>
struct is_unique_ptr : std::false_type {};
template<typename T>
struct is_unique_ptr<std::unique_ptr<T>> : std::true_type {};

// is_shared_ptr
template<typename T>
struct is_shared_ptr : std::false_type {};
template<typename T>
struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

// is_weak_ptr
template<typename T>
struct is_weak_ptr : std::false_type {};
template<typename T>
struct is_weak_ptr<std::weak_ptr<T>> : std::true_type {};

// is_smart_ptr
template<typename T>
struct is_smart_ptr : std::false_type {};
template<typename T>
struct is_smart_ptr<std::unique_ptr<T>> : std::true_type {};
template<typename T>
struct is_smart_ptr<std::shared_ptr<T>> : std::true_type {};
template<typename T>
struct is_smart_ptr<std::weak_ptr<T>> : std::true_type {};

} // namespace hlib

