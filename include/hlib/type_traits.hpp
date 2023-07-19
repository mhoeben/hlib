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

#include <type_traits>

namespace hlib
{

#if 0
namespace detail
{
    // Needed for some older versions of GCC.
    template<typename...> struct voider { using type = void; };

    template<typename T, typename U = void> struct is_associative : std::false_type {};
    template<typename T> struct is_associative<T, std::void_t<typename T::key_type, typename T::mapped_type, decltype(std::declval<T&>()[std::declval<const typename T::key_type&>()])>> : std::true_type {};
} // namespace detail

template<typename T> struct is_associative : detail::is_associative<T>::type {};

#endif

template <typename Container, typename = void>
struct is_associative : std::false_type {};

template <typename Container>
struct is_associative<Container, std::void_t<typename Container::key_type>> : std::true_type {};


} // namespace hlib

