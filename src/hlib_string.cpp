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

//
// Public
//
std::optional<bool> hlib::try_stob(std::string const& value)
{
    if ("true" == value) {
        return true;
    }
    else if ("false" == value) {
        return false;
    }

    return std::optional<bool>();
}

std::optional<int8_t> hlib::try_stoi8(std::string const& value, int base)
{
    if (true == value.empty() || isspace(value.front())) {
        return std::optional<int8_t>();
    }

    char* pos = nullptr;
    long r = strtol(value.c_str(), &pos, base);
    if (nullptr == pos
     || 0 != *pos
     || r < std::numeric_limits<int8_t>::min()
     || r > std::numeric_limits<int8_t>::max()) {
        return std::optional<int8_t>();
    }
    return r;
}

std::optional<int16_t> hlib::try_stoi16(std::string const& value, int base)
{
    if (true == value.empty() || isspace(value.front())) {
        return std::optional<int16_t>();
    }

    char* pos = nullptr;
    long r = strtol(value.c_str(), &pos, base);
    if (nullptr == pos
     || 0 != *pos
     || r < std::numeric_limits<int16_t>::min()
     || r > std::numeric_limits<int16_t>::max()) {
        return std::optional<int16_t>();
    }
    return r;
}

std::optional<int32_t> hlib::try_stoi32(std::string const& value, int base)
{
    if (true == value.empty() || isspace(value.front())) {
        return std::optional<int32_t>();
    }

    errno = 0;

    char* pos = nullptr;
    long r = strtol(value.c_str(), &pos, base);
    if (nullptr == pos
     || 0 != *pos
     || r < std::numeric_limits<int32_t>::min()
     || r > std::numeric_limits<int32_t>::max()
     || ((LONG_MIN == r || LONG_MAX == r) && ERANGE == errno)) {
        return std::optional<int32_t>();
    }
    return r;
}

std::optional<int64_t> hlib::try_stoi64(std::string const& value, int base)
{
    if (true == value.empty() || isspace(value.front())) {
        return std::optional<int64_t>();
    }

    errno = 0;

    char* pos = nullptr;
    long long r = strtoll(value.c_str(), &pos, base);
    if (nullptr == pos
     || 0 != *pos
     || ((LLONG_MIN == r || LLONG_MAX == r) && ERANGE == errno)) {
        return std::optional<int64_t>();
    }
    return r;
}

std::optional<uint8_t> hlib::try_stoui8(std::string const& value, int base)
{
    if (true == value.empty() || isspace(value.front())) {
        return std::optional<uint8_t>();
    }

    char* pos = nullptr;
    unsigned long r = strtoul(value.c_str(), &pos, base);
    if (nullptr == pos
     || 0 != *pos
     || r < std::numeric_limits<uint8_t>::min()
     || r > std::numeric_limits<uint8_t>::max()) {
        return std::optional<uint8_t>();
    }
    return r;
}

std::optional<uint16_t> hlib::try_stoui16(std::string const& value, int base)
{
    if (true == value.empty() || isspace(value.front())) {
        return std::optional<uint16_t>();
    }

    char* pos = nullptr;
    unsigned long r = strtoul(value.c_str(), &pos, base);
    if (nullptr == pos
     || 0 != *pos
     || r < std::numeric_limits<uint16_t>::min()
     || r > std::numeric_limits<uint16_t>::max()) {
        return std::optional<uint16_t>();
    }
    return r;
}

std::optional<uint32_t> hlib::try_stoui32(std::string const& value, int base)
{
    if (true == value.empty() || isspace(value.front())) {
        return std::optional<uint32_t>();
    }

    errno = 0;

    char* pos = nullptr;
    unsigned long r = strtoul(value.c_str(), &pos, base);
    if (nullptr == pos
     || 0 != *pos
     || r < std::numeric_limits<uint32_t>::min()
     || r > std::numeric_limits<uint32_t>::max()
     || (ULONG_MAX == r && ERANGE == errno)) {
        return std::optional<uint32_t>();
    }
    return r;
}

std::optional<uint64_t> hlib::try_stoui64(std::string const& value, int base)
{
    if (true == value.empty() || isspace(value.front())) {
        return std::optional<uint64_t>();
    }

    errno = 0;

    char* pos = nullptr;
    unsigned long long r = strtoull(value.c_str(), &pos, base);
    if (nullptr == pos
     || 0 != *pos
     || (ULLONG_MAX == r && ERANGE == errno)) {
        return std::optional<int64_t>();
    }
    return r;
}

std::optional<float> hlib::try_stof32(std::string const& value)
{
    if (true == value.empty() || isspace(value.front())) {
        return std::optional<float>();
    }

    char* pos = nullptr;
    float r = strtof(value.c_str(), &pos);
    if (nullptr == pos
     || 0 != *pos
     || ERANGE == errno) {
        return std::optional<float>();
    }
    return r;
}

std::optional<double> hlib::try_stof64(std::string const& value)
{
    if (true == value.empty() || isspace(value.front())) {
        return std::optional<double>();
    }

    char* pos = nullptr;
    double r = strtod(value.c_str(), &pos);
    if (nullptr == pos
     || 0 != *pos
     || ERANGE == errno) {
        return std::optional<double>();
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

int8_t hlib::stoi8(std::string const& value, int base)
{
    if (true == value.empty() || isspace(value.front())) {
        throw std::invalid_argument("stoi8");
    }

    char* pos = nullptr;
    long r = strtol(value.c_str(), &pos, base);
    if (nullptr == pos || 0 != *pos) {
        throw std::invalid_argument("stoi8");
    }
    if (r < std::numeric_limits<int8_t>::min()
     || r > std::numeric_limits<int8_t>::max()) {
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

uint8_t hlib::stoui8(std::string const& value, int base)
{
    if (true == value.empty() || isspace(value.front())) {
        throw std::invalid_argument("stoui8");
    }

    char* pos = nullptr;
    unsigned long r = strtoul(value.c_str(), &pos, base);
    if (nullptr == pos || 0 != *pos) {
        throw std::invalid_argument("stoui8");
    }
    if (r < std::numeric_limits<uint8_t>::min()
     || r > std::numeric_limits<uint8_t>::max()) {
        throw std::range_error("stoui8");
    }
    return r;
}

uint16_t hlib::stoui16(std::string const& value, int base)
{
    if (true == value.empty() || isspace(value.front())) {
        throw std::invalid_argument("stoui16");
    }

    char* pos = nullptr;
    unsigned long r = strtoul(value.c_str(), &pos, base);
    if (nullptr == pos || 0 != *pos) {
        throw std::invalid_argument("stoui16");
    }
    if (r < std::numeric_limits<uint16_t>::min()
     || r > std::numeric_limits<uint16_t>::max()) {
        throw std::range_error("stoui16");
    }
    return r;
}

uint32_t hlib::stoui32(std::string const& value, int base)
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
    if (r < std::numeric_limits<uint32_t>::min()
     || r > std::numeric_limits<uint32_t>::max()
     || (ULONG_MAX == r && ERANGE == errno)) {
        throw std::range_error("stoui32");
    }
    return r;
}

uint64_t hlib::stoui64(std::string const& value, int base)
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

