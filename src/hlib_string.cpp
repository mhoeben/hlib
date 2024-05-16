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
// Base64 code based on: https://github.com/tomcumming/base64.
//
#include "hlib/string.hpp"
#include <cstdlib>
#include <climits>
#include <cstring>
#include <locale>
#include <limits>

// #pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Wunused-function"

using namespace hlib;

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

std::optional<std::int16_t> hlib::stoi16(std::string const& value, int base, std::nothrow_t)
{
    if (true == value.empty() || isspace(value.front())) {
        return std::nullopt;
    }

    char* pos = nullptr;
    long r = strtol(value.c_str(), &pos, base);
    if (nullptr == pos
     || 0 != *pos
     || r < std::numeric_limits<std::int16_t>::min()
     || r > std::numeric_limits<std::int16_t>::max()) {
        return std::nullopt;
    }
    return r;
}

std::optional<std::int32_t> hlib::stoi32(std::string const& value, int base, std::nothrow_t)
{
    if (true == value.empty() || isspace(value.front())) {
        return std::nullopt;
    }

    errno = 0;

    char* pos = nullptr;
    long r = strtol(value.c_str(), &pos, base);
    if (nullptr == pos
     || 0 != *pos
     || r < std::numeric_limits<std::int32_t>::min()
     || r > std::numeric_limits<std::int32_t>::max()
     || ((LONG_MIN == r || LONG_MAX == r) && ERANGE == errno)) {
        return std::nullopt;
    }
    return r;
}

std::optional<std::int64_t> hlib::stoi64(std::string const& value, int base, std::nothrow_t)
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

std::int16_t hlib::stoi16(std::string const& value, int base)
{
    if (true == value.empty() || isspace(value.front())) {
        throw std::invalid_argument("stoi16");
    }

    char* pos = nullptr;
    long r = strtol(value.c_str(), &pos, base);
    if (nullptr == pos || 0 != *pos) {
        throw std::invalid_argument("stoi16");
    }
    if (r < std::numeric_limits<std::int16_t>::min()
     || r > std::numeric_limits<std::int16_t>::max()) {
        throw std::range_error("stoi16");
    }
    return r;
}

std::int32_t hlib::stoi32(std::string const& value, int base)
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
    if (r < std::numeric_limits<std::int32_t>::min()
     || r > std::numeric_limits<std::int32_t>::max()
     || ((LONG_MIN == r || LONG_MAX == r) && ERANGE == errno)) {
        throw std::range_error("stoi32");
    }
    return r;
}

std::int64_t hlib::stoi64(std::string const& value, int base)
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
    if (true == vector.empty()) {
        return std::string();
    }

    std::size_t length = 0;

    for (std::size_t i = 0; i < vector.size() - 1; ++i) {
        length += vector[i].length();
        length += separator.length();
    }
    length += vector.back().length();

    std::string string;
    string.reserve(length);

    for (std::size_t i = 0; i < vector.size() - 1; ++i) {
        string += vector[i];
        string += separator;
    }
    string += vector.back();

    return string;
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
    return ((size + 2) / 3) << 2;
}

bool hlib::base64_encode(Buffer& buffer, void const* data, std::size_t size) noexcept
{
    static char const encode_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    auto encode_start = [](uint8_t* dst, uint8_t const *src)
    {
        uint8_t t;
        *dst++ = encode_table[(src[0] & 0xfc) >> 2];

        t  = (src[0] & 0x03) << 4;
        t |= (src[1] & 0xf0) >> 4;
        *dst++ = encode_table[t];

        return dst;
    };

    std::size_t const encoded_length = base64_encode_get_length(size);
    if (0 == encoded_length) {
        return true;
    }

    uint8_t const* src = static_cast<uint8_t const*>(data);
    uint8_t* dst = static_cast<uint8_t*>(buffer.extend(encoded_length, std::nothrow));
    if (nullptr == dst) {
        return false;
    }

    while (size >= 3) {
        // 1st and 2nd chars.
        dst = encode_start(dst, src);

        // 3rd char.
        *dst  = (src[1] & 0x0f) << 2;
        *dst |= (src[2] & 0xc0) >> 6;
        *dst  = encode_table[*dst];
        ++dst;

        // 4th char.
        *dst++ = encode_table[src[2] & 0x3f];

        src += 3;
        size -= 3;
    }

    switch (size) {
    case 2:
     {
        // 1st and 2nd chars + padding.
         dst   = encode_start(dst, src);
        *dst++ = encode_table[(src[1] & 0x0f) << 2];
        *dst++ = '=';
        break;
     }
    case 1:
        // 1st and 2nd chars + padding.
        *dst++ = encode_table[(src[0] & 0xfc) >> 2];
        *dst++ = encode_table[(src[0] & 0x03) << 4];
        *dst++ = '=';
        *dst++ = '=';
        break;

    default:
        break;
    }

    buffer.resize(buffer.size() + encoded_length);
    return true;
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
    return (3 * (length >> 2)) + 2;
}

bool hlib::base64_decode(Buffer& buffer, char const* data, std::size_t length) noexcept
{
    std::size_t const decoded_size = base64_decode_get_size(length);
    if (0 == decoded_size) {
        return true;
    }

    auto decode_char = [](char c) -> uint8_t
    {
        if (c >= 'A' && c <= 'Z') {
            return (static_cast<uint8_t>(c)) - 'A' + 0;
        }
        if (c >= 'a' && c <= 'z') {
            return (static_cast<uint8_t>(c)) - 'a' + 26;
        }
        if (c >= '0' && c <= '9') {
            return (static_cast<uint8_t>(c)) - '0' + 52;
        }

        return '+' == c ? 62 : 63;
    };

    auto decode2 = [&decode_char](uint8_t *dst, uint8_t const* src) -> uint8_t*
    {
        *dst  = (decode_char(src[0])       ) << 2;
        *dst |= (decode_char(src[1]) & 0x30) >> 4;
        return ++dst;
    };

    auto decode3 = [&decode_char, &decode2](uint8_t *dst, uint8_t const* src) -> uint8_t*
    {
        dst = decode2(dst, src);

        *dst  = (decode_char(src[1]) & 0x0f) << 4;
        *dst |= (decode_char(src[2]) & 0x3c) >> 2;
        return ++dst;
    };

    auto decode4 = [&decode_char, &decode3](uint8_t *dst, uint8_t const* src) -> uint8_t*
    {
        dst = decode3(dst, src);

        *dst  = (decode_char(src[2]) & 0x03) << 6;
        *dst |= (decode_char(src[3])       );
        return ++dst;
    };

    uint8_t const* src = reinterpret_cast<uint8_t const*>(data);
    uint8_t* dst = static_cast<uint8_t*>(buffer.extend(decoded_size));
    if (nullptr == dst) {
        return false;
    }

    while (length >= 5) {
        dst = decode4(dst, src);
        src += 4;
        length -= 4;
    }

    switch (length) {
    case 4:
        if ('=' != src[3]) {
            dst = decode4(dst, src);
        }
        else if ('=' != src[2]) {
            dst = decode3(dst, src);
        }
        else {
            dst = decode2(dst, src);
        }
        break;

    case 3:
        dst = decode3(dst, src);
        break;

    case 2:
        break;

    default:
        assert(0);
        return false;
    }

    buffer.resize(buffer.size() + decoded_size);
    return true;
}

hlib::Buffer hlib::base64_decode(std::string const& string)
{
    Buffer buffer;
    base64_decode(buffer, string.data(), string.length());
    return buffer;
}

