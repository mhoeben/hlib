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
#include "hlib/buffer.hpp"
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include <type_traits>

namespace hlib
{

std::optional<bool> stob(std::string const& value, std::nothrow_t);

std::optional<std::int8_t> stoi8(std::string const& value, int base, std::nothrow_t);
std::optional<std::int16_t> stoi16(std::string const& value, int base, std::nothrow_t);
std::optional<std::int32_t> stoi32(std::string const& value, int base, std::nothrow_t);
std::optional<std::int64_t> stoi64(std::string const& value, int base, std::nothrow_t);

std::optional<std::uint8_t> stoui8(std::string const& value, int base, std::nothrow_t);
std::optional<std::uint16_t> stoui16(std::string const& value, int base, std::nothrow_t);
std::optional<std::uint32_t> stoui32(std::string const& value, int base, std::nothrow_t);
std::optional<std::uint64_t> stoui64(std::string const& value, int base, std::nothrow_t);

std::optional<float> stof32(std::string const& value, std::nothrow_t);
std::optional<double> stof64(std::string const& value, std::nothrow_t);

template<typename T>
typename std::enable_if<std::is_same<bool, T>::value, std::optional<bool>>::type
string_to(std::string const& value, std::nothrow_t)
{
    return stob(value, std::nothrow);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_signed<T>::value
                     && 1 == sizeof(T), std::optional<T>>::type
string_to(std::string const& value, int base, std::nothrow_t)
{
    return stoi8(value, base, std::nothrow);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_signed<T>::value
                     && 2 == sizeof(T), std::optional<T>>::type
string_to(std::string const& value, int base, std::nothrow_t)
{
    return stoi16(value, base, std::nothrow);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_signed<T>::value
                     && 4 == sizeof(T), std::optional<T>>::type
string_to(std::string const& value, int base, std::nothrow_t)
{
    return stoi32(value, base, std::nothrow);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_signed<T>::value
                     && 8 == sizeof(T), std::optional<T>>::type
string_to(std::string const& value, int base, std::nothrow_t)
{
    return stoi64(value, base, std::nothrow);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_unsigned<T>::value
                     && 1 == sizeof(T), std::optional<T>>::type
string_to(std::string const& value, int base, std::nothrow_t)
{
    return stoui8(value, base, std::nothrow);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_unsigned<T>::value
                     && 2 == sizeof(T), std::optional<T>>::type
string_to(std::string const& value, int base, std::nothrow_t)
{
    return stoui16(value, base, std::nothrow);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_unsigned<T>::value
                     && 4 == sizeof(T), std::optional<T>>::type
string_to(std::string const& value, int base, std::nothrow_t)
{
    return stoui32(value, base, std::nothrow);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_unsigned<T>::value
                     && 8 == sizeof(T), std::optional<T>>::type
string_to(std::string const& value, int base, std::nothrow_t)
{
    return stoui64(value, base, std::nothrow);
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value
                     && 4 == sizeof(T), std::optional<T>>::type
string_to(std::string const& value, std::nothrow_t)
{
    return stof32(value, std::nothrow);
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value
                     && 8 == sizeof(T), std::optional<T>>::type
string_to(std::string const& value, std::nothrow_t)
{
    return stof64(value, std::nothrow);
}

bool stob(std::string const& value);

std::int8_t stoi8(std::string const& value, int base = 10);
std::int16_t stoi16(std::string const& value, int base = 10);
std::int32_t stoi32(std::string const& value, int base = 10);
std::int64_t stoi64(std::string const& value, int base = 10);

std::uint8_t stoui8(std::string const& value, int base = 10);
std::uint16_t stoui16(std::string const& value, int base = 10);
std::uint32_t stoui32(std::string const& value, int base = 10);
std::uint64_t stoui64(std::string const& value, int base = 10);

float stof32(std::string const& value);
double stof64(std::string const& value);

template<typename T>
typename std::enable_if<std::is_same<bool, T>::value, T>::type
string_to(std::string const& value)
{
    return stob(value);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_signed<T>::value
                     && 1 == sizeof(T), T>::type
string_to(std::string const& value, int base = 10)
{
    return stoi8(value, base);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_signed<T>::value
                     && 2 == sizeof(T), T>::type
string_to(std::string const& value, int base = 10)
{
    return stoi16(value, base);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_signed<T>::value
                     && 4 == sizeof(T), T>::type
string_to(std::string const& value, int base = 10)
{
    return stoi32(value, base);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_signed<T>::value
                     && 8 == sizeof(T), T>::type
string_to(std::string const& value, int base = 10)
{
    return stoi64(value, base);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_unsigned<T>::value
                     && 1 == sizeof(T), T>::type
string_to(std::string const& value, int base = 10)
{
    return stoui8(value, base);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_unsigned<T>::value
                     && 2 == sizeof(T), T>::type
string_to(std::string const& value, int base = 10)
{
    return stoui16(value, base);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_unsigned<T>::value
                     && 4 == sizeof(T), T>::type
string_to(std::string const& value, int base = 10)
{
    return stoui32(value, base);
}

template<typename T>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value
                     && std::is_unsigned<T>::value
                     && 8 == sizeof(T), T>::type
string_to(std::string const& value, int base = 10)
{
    return stoui64(value, base);
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value
                     && 4 == sizeof(T), T>::type
string_to(std::string const& value)
{
    return stof32(value);
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value
                     && 8 == sizeof(T), T>::type
string_to(std::string const& value)
{
    return stof64(value);
}

bool iequals(std::string const &lhs, std::string const& rhs);

bool starts_with(std::string const& string, std::string const& substring);
bool starts_with(std::string const& string, char character);

bool ends_with(std::string const& string, std::string const& substring);
bool ends_with(std::string const& string, char character);

bool contains(std::string const& string, std::string const& substring);
bool contains(std::string const& string, char character);

std::size_t count(std::string const& string, std::string const& substring);
std::size_t count(std::string const& string, char character);

std::string to_upper(std::string string);
std::string to_lower(std::string string);

std::string trim_left(std::string string, std::string const& chars = " \t\v\f\r\n");
std::string trim_right(std::string string, std::string const& chars = " \t\v\f\r\n");
std::string trim(std::string, std::string const& chars = " \t\v\f\r\n");

std::vector<std::string> split(std::string const& string, std::function<bool(char)> const& is_delimiter, bool filter_empty = false);
std::vector<std::string> split(std::string const& string, char delimiter, bool filter_empty = false);

std::string join(std::vector<std::string> const& vector, std::string const& separator = " ");

template<typename Container>
std::string join(Container const& container, std::string const& separator,
    std::function<std::string(typename Container::value_type const&)> formatter = [](typename Container::value_type const& value) noexcept { return std::to_string(value); })
{
    Buffer buffer(4096);
    bool separate = false;

    for (auto const& element : container) {
        if (true == separate) {
            buffer.append(separator);
        }
        buffer.append(formatter(element));
        separate = true;
    }

    return to_string(buffer);
}

std::string replace(std::string string, std::string const& literal, std::string const& value);

std::size_t base64_encode_get_length(std::size_t size) noexcept;
bool base64_encode(Buffer& buffer, void const* data, std::size_t size) noexcept;
std::string base64_encode(void const* data, std::size_t length);
std::string base64_encode(Buffer const& buffer);

std::size_t base64_decode_get_size(std::size_t length) noexcept;
bool base64_decode(Buffer& buffer, char const* data, std::size_t length) noexcept;
Buffer base64_decode(std::string const& string);

void memory_copy(void* dst, std::size_t dst_stride, void const* src, std::size_t src_stride,
    std::size_t line_size, std::size_t lines);

} // namespace hlib

