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
#include "hlib/string.hpp"
#include "hlib/format.hpp"
#include <cstdlib>
#include <climits>
#include <locale>
#include <limits>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

#define HWS_VISIBILITY_STATIC
#define HWS_HAVE_OPENSSL
#define HWS_IMPL
#include "../third_party/hserv/include/hws.h"
#pragma GCC diagnostic pop

//
// Public
//
std::optional<bool> hlib::stob(std::string const& value, std::nothrow_t)
{
    if ("true" == value) {
        return true;
    }
    else if ("false" == value) {
        return false;
    }

    return std::nullopt;
}

std::optional<std::int8_t> hlib::stoi8(std::string const& value, int base, std::nothrow_t)
{
    if (true == value.empty() || isspace(value.front())) {
        return std::nullopt;
    }

    char* pos = nullptr;
    long r = strtol(value.c_str(), &pos, base);
    if (nullptr == pos
     || 0 != *pos
     || r < std::numeric_limits<std::int8_t>::min()
     || r > std::numeric_limits<std::int8_t>::max()) {
        return std::nullopt;
    }
    return r;
}

std::optional<int16_t> hlib::stoi16(std::string const& value, int base, std::nothrow_t)
{
    if (true == value.empty() || isspace(value.front())) {
        return std::nullopt;
    }

    char* pos = nullptr;
    long r = strtol(value.c_str(), &pos, base);
    if (nullptr == pos
     || 0 != *pos
     || r < std::numeric_limits<int16_t>::min()
     || r > std::numeric_limits<int16_t>::max()) {
        return std::nullopt;
    }
    return r;
}

std::optional<int32_t> hlib::stoi32(std::string const& value, int base, std::nothrow_t)
{
    if (true == value.empty() || isspace(value.front())) {
        return std::nullopt;
    }

    errno = 0;

    char* pos = nullptr;
    long r = strtol(value.c_str(), &pos, base);
    if (nullptr == pos
     || 0 != *pos
     || r < std::numeric_limits<int32_t>::min()
     || r > std::numeric_limits<int32_t>::max()
     || ((LONG_MIN == r || LONG_MAX == r) && ERANGE == errno)) {
        return std::nullopt;
    }
    return r;
}

std::optional<int64_t> hlib::stoi64(std::string const& value, int base, std::nothrow_t)
{
    if (true == value.empty() || isspace(value.front())) {
        return std::nullopt;
    }

    errno = 0;

    char* pos = nullptr;
    long long r = strtoll(value.c_str(), &pos, base);
    if (nullptr == pos
     || 0 != *pos
     || ((LLONG_MIN == r || LLONG_MAX == r) && ERANGE == errno)) {
        return std::nullopt;
    }
    return r;
}

std::optional<std::uint8_t> hlib::stoui8(std::string const& value, int base, std::nothrow_t)
{
    if (true == value.empty() || isspace(value.front())) {
        return std::nullopt;
    }

    char* pos = nullptr;
    unsigned long r = strtoul(value.c_str(), &pos, base);
    if (nullptr == pos
     || 0 != *pos
     || r < std::numeric_limits<std::uint8_t>::min()
     || r > std::numeric_limits<std::uint8_t>::max()) {
        return std::nullopt;
    }
    return r;
}

std::optional<std::uint16_t> hlib::stoui16(std::string const& value, int base, std::nothrow_t)
{
    if (true == value.empty() || isspace(value.front())) {
        return std::nullopt;
    }

    char* pos = nullptr;
    unsigned long r = strtoul(value.c_str(), &pos, base);
    if (nullptr == pos
     || 0 != *pos
     || r < std::numeric_limits<std::uint16_t>::min()
     || r > std::numeric_limits<std::uint16_t>::max()) {
        return std::nullopt;
    }
    return r;
}

std::optional<std::uint32_t> hlib::stoui32(std::string const& value, int base, std::nothrow_t)
{
    if (true == value.empty() || isspace(value.front())) {
        return std::nullopt;
    }

    errno = 0;

    char* pos = nullptr;
    unsigned long r = strtoul(value.c_str(), &pos, base);
    if (nullptr == pos
     || 0 != *pos
     || r < std::numeric_limits<std::uint32_t>::min()
     || r > std::numeric_limits<std::uint32_t>::max()
     || (ULONG_MAX == r && ERANGE == errno)) {
        return std::nullopt;
    }
    return r;
}

std::optional<std::uint64_t> hlib::stoui64(std::string const& value, int base, std::nothrow_t)
{
    if (true == value.empty() || isspace(value.front())) {
        return std::nullopt;
    }

    errno = 0;

    char* pos = nullptr;
    unsigned long long r = strtoull(value.c_str(), &pos, base);
    if (nullptr == pos
     || 0 != *pos
     || (ULLONG_MAX == r && ERANGE == errno)) {
        return std::nullopt;
    }
    return r;
}

std::optional<float> hlib::stof32(std::string const& value, std::nothrow_t)
{
    if (true == value.empty() || isspace(value.front())) {
        return std::nullopt;
    }

    char* pos = nullptr;
    float r = strtof(value.c_str(), &pos);
    if (nullptr == pos
     || 0 != *pos
     || ERANGE == errno) {
        return std::nullopt;
    }
    return r;
}

std::optional<double> hlib::stof64(std::string const& value, std::nothrow_t)
{
    if (true == value.empty() || isspace(value.front())) {
        return std::nullopt;
    }

    char* pos = nullptr;
    double r = strtod(value.c_str(), &pos);
    if (nullptr == pos
     || 0 != *pos
     || ERANGE == errno) {
        return std::nullopt;
    }
    return r;
}

bool hlib::stob(std::string const& value)
{
    if ("true" == value) {
        return true;
    }
    else if ("false" == value) {
        return false;
    }

    throw std::invalid_argument("stob");
}

std::int8_t hlib::stoi8(std::string const& value, int base)
{
    if (true == value.empty() || isspace(value.front())) {
        throw std::invalid_argument("stoi8");
    }

    char* pos = nullptr;
    long r = strtol(value.c_str(), &pos, base);
    if (nullptr == pos || 0 != *pos) {
        throw std::invalid_argument("stoi8");
    }
    if (r < std::numeric_limits<std::int8_t>::min()
     || r > std::numeric_limits<std::int8_t>::max()) {
        throw std::range_error("stoi8");
    }
    return r;
}

int16_t hlib::stoi16(std::string const& value, int base)
{
    if (true == value.empty() || isspace(value.front())) {
        throw std::invalid_argument("stoi16");
    }

    char* pos = nullptr;
    long r = strtol(value.c_str(), &pos, base);
    if (nullptr == pos || 0 != *pos) {
        throw std::invalid_argument("stoi16");
    }
    if (r < std::numeric_limits<int16_t>::min()
     || r > std::numeric_limits<int16_t>::max()) {
        throw std::range_error("stoi16");
    }
    return r;
}

int32_t hlib::stoi32(std::string const& value, int base)
{
    if (true == value.empty() || isspace(value.front())) {
        throw std::invalid_argument("stoi32");
    }

    errno = 0;

    char* pos = nullptr;
    long r = strtol(value.c_str(), &pos, base);
    if (nullptr == pos || 0 != *pos) {
        throw std::invalid_argument("stoi32");
    }
    if (r < std::numeric_limits<int32_t>::min()
     || r > std::numeric_limits<int32_t>::max()
     || ((LONG_MIN == r || LONG_MAX == r) && ERANGE == errno)) {
        throw std::range_error("stoi32");
    }
    return r;
}

int64_t hlib::stoi64(std::string const& value, int base)
{
    if (true == value.empty() || isspace(value.front())) {
        throw std::invalid_argument("stoi64");
    }

    errno = 0;

    char* pos = nullptr;
    long long r = strtoll(value.c_str(), &pos, base);
    if (nullptr == pos || 0 != *pos) {
        throw std::invalid_argument("stoi64");
    }
    if (((LLONG_MIN == r || LLONG_MAX == r) && ERANGE == errno)) {
        throw std::range_error("stoi64");
    }
    return r;
}

std::uint8_t hlib::stoui8(std::string const& value, int base)
{
    if (true == value.empty() || isspace(value.front())) {
        throw std::invalid_argument("stoui8");
    }

    char* pos = nullptr;
    unsigned long r = strtoul(value.c_str(), &pos, base);
    if (nullptr == pos || 0 != *pos) {
        throw std::invalid_argument("stoui8");
    }
    if (r < std::numeric_limits<std::uint8_t>::min()
     || r > std::numeric_limits<std::uint8_t>::max()) {
        throw std::range_error("stoui8");
    }
    return r;
}

std::uint16_t hlib::stoui16(std::string const& value, int base)
{
    if (true == value.empty() || isspace(value.front())) {
        throw std::invalid_argument("stoui16");
    }

    char* pos = nullptr;
    unsigned long r = strtoul(value.c_str(), &pos, base);
    if (nullptr == pos || 0 != *pos) {
        throw std::invalid_argument("stoui16");
    }
    if (r < std::numeric_limits<std::uint16_t>::min()
     || r > std::numeric_limits<std::uint16_t>::max()) {
        throw std::range_error("stoui16");
    }
    return r;
}

std::uint32_t hlib::stoui32(std::string const& value, int base)
{
    if (true == value.empty() || isspace(value.front())) {
        throw std::invalid_argument("stoui32");
    }

    errno = 0;

    char* pos = nullptr;
    unsigned long r = strtoul(value.c_str(), &pos, base);
    if (nullptr == pos || 0 != *pos) {
        throw std::invalid_argument("stoui32");
    }
    if (r < std::numeric_limits<std::uint32_t>::min()
     || r > std::numeric_limits<std::uint32_t>::max()
     || (ULONG_MAX == r && ERANGE == errno)) {
        throw std::range_error("stoui32");
    }
    return r;
}

std::uint64_t hlib::stoui64(std::string const& value, int base)
{
    if (true == value.empty() || isspace(value.front())) {
        throw std::invalid_argument("stoui32");
    }

    errno = 0;

    char* pos = nullptr;
    unsigned long long r = strtoull(value.c_str(), &pos, base);
    if (nullptr == pos || 0 != *pos) {
        throw std::invalid_argument("stoui32");
    }
    if ((ULLONG_MAX == r && ERANGE == errno)) {
        throw std::range_error("stoui32");
    }
    return r;
}

float hlib::stof32(std::string const& value)
{
    if (true == value.empty() || isspace(value.front())) {
        throw std::invalid_argument("stof32");
    }

    char* pos = nullptr;
    float r = strtof(value.c_str(), &pos);
    if (nullptr == pos || 0 != *pos) {
        throw std::invalid_argument("stof32");
    }
    if (ERANGE == errno) {
        throw std::range_error("stof32");
    }
    return r;
}

double hlib::stof64(std::string const& value)
{
    if (true == value.empty() || isspace(value.front())) {
        throw std::invalid_argument("stof64");
    }

    char* pos = nullptr;
    double r = strtod(value.c_str(), &pos);
    if (nullptr == pos || 0 != *pos) {
        throw std::invalid_argument("stof64");
    }
    if (ERANGE == errno) {
        throw std::range_error("stof64");
    }
    return r;
}

bool hlib::iequals(std::string const &lhs, std::string const& rhs)
{
    if (lhs.size() != rhs.size()) {
        return false;
    }

    std::locale locale;

    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (std::tolower(lhs[i], locale) != std::tolower(rhs[i], locale)) {
            return false;
        }
    }

    return true;
}

bool hlib::starts_with(std::string const& string, std::string const& substring)
{
    if (string.length() < substring.length()) {
        return false;
    }

    return 0 == memcmp(string.data(), substring.data(), substring.length());
}

bool hlib::starts_with(std::string const& string, char character)
{
    if (true == string.empty()) {
        return false;
    }

    return string.front() == character;
}

bool hlib::ends_with(std::string const& string, std::string const& substring)
{
    if (string.length() < substring.length()) {
        return false;
    }

    std::size_t offset = string.length() - substring.length();
    return 0 == memcmp(string.data() + offset, substring.data(), substring.length());
}

bool hlib::ends_with(std::string const& string, char character)
{
    if (true == string.empty()) {
        return false;
    }

    return string.back() == character;
}

bool hlib::contains(std::string const& string, std::string const& substring)
{
    return std::string::npos != string.find(substring);
}

bool hlib::contains(std::string const& string, char character)
{
    return std::string::npos != string.find(character);
}

std::size_t hlib::count(std::string const& string, std::string const& substring)
{
    std::size_t count = 0;
    std::string::size_type pos = 0;

    do {
        pos = string.find(substring, pos);
        if (std::string::npos == pos) {
            return count;
        }

        pos += substring.length();
        ++count;
    }
    while (true);
}

std::size_t hlib::count(std::string const& string, char character)
{
    std::size_t count = 0;
    std::string::size_type pos = 0;

    do {
        pos = string.find(character, pos);
        if (std::string::npos == pos) {
            return count;
        }

        ++pos;
        ++count;
    }
    while (true);
}

std::string hlib::to_upper(std::string string)
{
    std::locale locale;

    for (char& c : string) {
        c = std::toupper(c, locale);
    }

    return string;
}

std::string hlib::to_lower(std::string string)
{
    std::locale locale;

    for (char& c : string) {
        c = std::tolower(c, locale);
    }

    return string;
}

std::string hlib::strip_left(std::string string, std::string const& chars)
{
    return string.erase(0, string.find_first_not_of(chars));
}

std::string hlib::strip_right(std::string string, std::string const& chars)
{
    std::string::size_type pos = 0;

    for (auto it = string.rbegin(); it != string.rend(); ++it) {
        if (std::string::npos == chars.find(*it)) {
            break;
        }
        ++pos;
    }

    string.resize(string.size() - pos);
    return string;
}

std::string hlib::strip(std::string string, std::string const& chars)
{
    string = strip_left(std::move(string), chars);
    string = strip_right(std::move(string), chars);
    return string;
}

std::vector<std::string> hlib::split(std::string const& string, std::function<bool(char)> const& is_delimiter, bool filter_empty)
{
    std::vector<std::string> tokens;
    std::string token;
    std::size_t count = 0;

    if (true == string.empty()) {
        return {};
    }

    for (std::string::size_type i = 0; i < string.length(); ++i) {
        char c = string[i];
        if (true == is_delimiter(c)) {
            if (false == token.empty() || filter_empty == false) {
                tokens.emplace_back(std::move(token));
                token.reserve(string.length() - i);
            }
            ++count;
        }
        else {
            token += c;
        }
    }

    if (false == token.empty()
     || (count > 0 && filter_empty == false)) {
        tokens.emplace_back(std::move(token));
    }

    return tokens;
}

std::vector<std::string> hlib::split(std::string const& string, char delimiter, bool filter_empty)
{
    return split(string, [delimiter](char c) noexcept { return delimiter == c; }, filter_empty);
}

std::string hlib::join(std::vector<std::string> const& vector, std::string const& separator)
{
    return fmt::format("{}", fmt::join(vector, separator));
}

std::string hlib::replace(std::string string, std::string const& literal, std::string const& value)
{
    std::string::size_type pos = string.find(literal);
    if (std::string::npos == pos) {
        return string;
    }

    string.replace(pos, literal.length(), value);
    return string;
}

std::size_t hlib::base64_encode_get_length(std::size_t size) noexcept
{
    return hws_base64_encode_get_length(size);
}

void hlib::base64_encode(Buffer& buffer, void const* data, std::size_t size)
{
    // Get encoded length.
    std::size_t length = hws_base64_encode_get_length(size);

    // Extend buffer and append base64 encoded string to buffer.
    length = hws_base64_encode(static_cast<char*>(buffer.extend(length)), data, size);

    // Resize buffer size to the extended length.
    buffer.resize(buffer.size() + length);
}

std::string hlib::base64_encode(void const* data, std::size_t size)
{
    Buffer buffer;
    base64_encode(buffer, data, size);
    return to_string(buffer);
}

std::string hlib::base64_encode(Buffer const& buffer)
{
    return base64_encode(buffer.data(), buffer.size());
}

std::size_t hlib::base64_decode_get_size(std::size_t length) noexcept
{
    return hws_base64_decode_get_size(length);
}

void hlib::base64_decode(Buffer& buffer, char const* data, std::size_t length)
{
    // Get decoded length.
    std::size_t size = hws_base64_decode_get_size(length);

    // Extend buffer and append decoded base64 data to buffer.
    size = hws_base64_decode(buffer.extend(size), data, length);

    // Resize buffer size to to extended size.
    buffer.resize(buffer.size() + size);
}

hlib::Buffer hlib::base64_decode(std::string const& string)
{
    Buffer buffer;
    base64_decode(buffer, string.data(), string.length());
    return buffer;
}

