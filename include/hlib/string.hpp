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
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include <type_traits>

namespace hlib
{

std::optional<bool> try_stob(std::string const& value);

std::optional<int8_t> try_stoi8(std::string const& value, int base = 10);
std::optional<int16_t> try_stoi16(std::string const& value, int base = 10);
std::optional<int32_t> try_stoi32(std::string const& value, int base = 10);
std::optional<int64_t> try_stoi64(std::string const& value, int base = 10);

std::optional<uint8_t> try_stoui8(std::string const& value, int base = 10);
std::optional<uint16_t> try_stoui16(std::string const& value, int base = 10);
std::optional<uint32_t> try_stoui32(std::string const& value, int base = 10);
std::optional<uint64_t> try_stoui64(std::string const& value, int base = 10);

std::optional<float> try_stof32(std::string const& value);
std::optional<double> try_stof64(std::string const& value);

template<typename T>
typename std::enable_if<std::is_same<bool, T>::value, std::optional<bool>>::type
try_to(std::string const& value)
{
    return try_stob(value);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_signed<T>::value
                     && 1 == sizeof(T), std::optional<T>>::type
try_to(std::string const& value, int base = 10)
{
    return try_stoi8(value, base);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_signed<T>::value
                     && 2 == sizeof(T), std::optional<T>>::type
try_to(std::string const& value, int base = 10)
{
    return try_stoi16(value, base);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_signed<T>::value
                     && 4 == sizeof(T), std::optional<T>>::type
try_to(std::string const& value, int base = 10)
{
    return try_stoi32(value, base);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_signed<T>::value
                     && 8 == sizeof(T), std::optional<T>>::type
try_to(std::string const& value, int base = 10)
{
    return try_stoi64(value, base);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_unsigned<T>::value
                     && 1 == sizeof(T), std::optional<T>>::type
try_to(std::string const& value, int base = 10)
{
    return try_stoui8(value, base);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_unsigned<T>::value
                     && 2 == sizeof(T), std::optional<T>>::type
try_to(std::string const& value, int base = 10)
{
    return try_stoui16(value, base);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_unsigned<T>::value
                     && 4 == sizeof(T), std::optional<T>>::type
try_to(std::string const& value, int base = 10)
{
    return try_stoui32(value, base);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_unsigned<T>::value
                     && 8 == sizeof(T), std::optional<T>>::type
try_to(std::string const& value, int base = 10)
{
    return try_stoui64(value, base);
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value
                     && 4 == sizeof(T), std::optional<T>>::type
try_to(std::string const& value)
{
    return try_stof32(value);
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value
                     && 8 == sizeof(T), std::optional<T>>::type
try_to(std::string const& value)
{
    return try_stof64(value);
}

bool stob(std::string const& value);

int8_t stoi8(std::string const& value, int base = 10);
int16_t stoi16(std::string const& value, int base = 10);
int32_t stoi32(std::string const& value, int base = 10);
int64_t stoi64(std::string const& value, int base = 10);

uint8_t stoui8(std::string const& value, int base = 10);
uint16_t stoui16(std::string const& value, int base = 10);
uint32_t stoui32(std::string const& value, int base = 10);
uint64_t stoui64(std::string const& value, int base = 10);

float stof32(std::string const& value);
double stof64(std::string const& value);

template<typename T>
typename std::enable_if<std::is_same<bool, T>::value, T>::type
to(std::string const& value)
{
    return stob(value);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_signed<T>::value
                     && 1 == sizeof(T), T>::type
to(std::string const& value, int base = 10)
{
    return stoi8(value, base);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_signed<T>::value
                     && 2 == sizeof(T), T>::type
to(std::string const& value, int base = 10)
{
    return stoi16(value, base);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_signed<T>::value
                     && 4 == sizeof(T), T>::type
to(std::string const& value, int base = 10)
{
    return stoi32(value, base);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_signed<T>::value
                     && 8 == sizeof(T), T>::type
to(std::string const& value, int base = 10)
{
    return stoi64(value, base);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_unsigned<T>::value
                     && 1 == sizeof(T), T>::type
to(std::string const& value, int base = 10)
{
    return stoui8(value, base);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_unsigned<T>::value
                     && 2 == sizeof(T), T>::type
to(std::string const& value, int base = 10)
{
    return stoui16(value, base);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_unsigned<T>::value
                     && 4 == sizeof(T), T>::type
to(std::string const& value, int base = 10)
{
    return stoui32(value, base);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_unsigned<T>::value
                     && 8 == sizeof(T), T>::type
to(std::string const& value, int base = 10)
{
    return stoui64(value, base);
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value
                     && 4 == sizeof(T), T>::type
to(std::string const& value)
{
    return stof32(value);
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value
                     && 8 == sizeof(T), T>::type
to(std::string const& value)
{
    return stof64(value);
}

bool iequals(std::string const &lhs, std::string const& rhs);

std::string to_upper(std::string string);
std::string to_lower(std::string string);

std::string strip_left(std::string string, std::string const& chars = " \t\v\f\r\n");
std::string strip_right(std::string string, std::string const& chars = " \t\v\f\r\n");
std::string strip(std::string, std::string const& chars = " \t\v\f\r\n");

std::vector<std::string> split(std::string const& string, std::function<bool(char)> const& is_delimiter, bool filter_empty = false);
std::vector<std::string> split(std::string const& string, char delimiter, bool filter_empty = false);

std::string join(std::vector<std::string> const& vector, std::string const& separator);

template<typename T>
std::string join(std::vector<T> const& vector, std::string const& separator)
{
    using namespace std;

    std::vector<std::string> strings;
    strings.reserve(vector.size());

    for (auto const& value : vector) {
        strings.emplace_back(to_string(value));
    }

    return join(strings, separator);
}

} // namespace hlib

