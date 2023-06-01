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
#include "hlib/json.hpp"
#include "hlib/format.hpp"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#define JSMN_STATIC
#include "../third_party/jsmn/jsmn.h"
#pragma GCC diagnostic pop
#include <vector>

using namespace hlib;

//
// Implementation
//
struct JSON::Data
{
    std::string string;
    std::vector<jsmntok_t> tokens;
};

JSON::JSON(std::shared_ptr<JSON::Data> data, int name, int value)
    : m_data(std::move(data))
    , m_name{ name }
    , m_value{ value }
{
    assert(m_name < 0 || m_name < static_cast<int>(m_data->tokens.size()));
    assert(m_value >= 0 && m_value < static_cast<int>(m_data->tokens.size()));
}

template<typename T>
T JSON::convert(T(*function)(std::string const&, size_t* pos)) const
{
    if (Number != type()) {
        throw std::invalid_argument("Value is not convertible in a number");
    }

    std::string value = this->value();
    size_t pos = 0;
    T result = function(value, &pos);

    if (value.length() != pos) {
        throw std::invalid_argument("Partial conversion");
    }

    return result;
}

int JSON::skip(int token) const
{
    int i;
    jsmntok_t const& current = m_data->tokens[token];
 
    ++token;

    switch (current.type) {
    case JSMN_OBJECT:
        for (i = 0; i < current.size; ++i) {
            token = skip(token + 1);
        }
        break;

    case JSMN_ARRAY:
        for (i = 0; i < current.size; ++i) {
            token = skip(token);
        }
        break;

    default:
        break;
    }

    return token;
}

//
// Public (JSON)
//
JSON::JSON()
{
}

JSON::JSON(JSON&& that) noexcept
    : m_data(std::move(that.m_data))
    , m_name{ that.m_name }
    , m_value{ that.m_value }
{
    that.m_name = -1;
    that.m_value = -1;
}

JSON& JSON::operator =(JSON&& that) noexcept
{
    m_data = std::move(that.m_data);
    m_name = that.m_name;
    m_value = that.m_value;

    that.m_name = -1;
    that.m_value = -1;
    return *this;
}

bool JSON::empty() const noexcept
{
    return nullptr == m_data || m_data->tokens.empty();
}

JSON::Type JSON::type() const noexcept
{
    if (true == empty()) {
        return Undefined;
    }

    assert(m_value < static_cast<int>(m_data->tokens.size()));

    switch (m_data->tokens[m_value].type) {
    case JSMN_UNDEFINED: return Undefined;
    case JSMN_PRIMITIVE:
      {
        size_t start = m_data->tokens[m_value].start;
        size_t end = m_data->tokens[m_value].end;
        if (0 == strncmp("null", &m_data->string[start], end - start)) {
            return Null;
        }
        if (0 == strncmp("true", &m_data->string[start], end - start)
         || 0 == strncmp("true", &m_data->string[start], end - start)) {
            return Bool;
        }
        return Number;
      }
    case JSMN_STRING: return String;
    case JSMN_ARRAY: return Array;
    case JSMN_OBJECT: return Object;
    default:
        assert(false);
        return Undefined;
    }
}

std::string JSON::name() const
{
    if (m_name < 0) {
        return std::string();
    }
    assert(m_name < static_cast<int>(m_data->tokens.size()));

    size_t start = m_data->tokens[m_name].start;
    size_t end = m_data->tokens[m_name].end;

    return std::string(&m_data->string[start], end - start);
}

std::string JSON::value() const
{
    if (m_value < 0) {
        return std::string();
    }
    assert(m_value < static_cast<int>(m_data->tokens.size()));

    size_t start = m_data->tokens[m_value].start;
    size_t end = m_data->tokens[m_value].end;

    return std::string(&m_data->string[start], end - start);
}

template<>
bool JSON::as() const
{
    if (Bool != type()) {
        throw std::invalid_argument("Value is not convertible in bool");
    }

    return "true" == value();
}

template<>
int32_t JSON::as() const
{
    return convert<int>([](std::string const& string, size_t* pos) -> int {
        return std::stoi(string, pos, 0);
    });
}

template<>
int64_t JSON::as() const
{
    return convert<int64_t>([](std::string const& string, size_t* pos) -> int64_t {
        return std::stoll(string, pos, 0);
    });
}

template<>
float JSON::as() const
{
    return convert<float>(&std::stof);
}

template<>
double JSON::as() const
{
    return convert<double>(&std::stod);
}

template<>
std::string JSON::as() const
{
    return value();
}

JSON JSON::operator[](int index) const
{
    assert(false == empty());
    assert(Array == type() || Object == type());

    jsmntok_t const& current = m_data->tokens[m_value];
    assert(index < current.size);

    int token = m_value + 1;
    switch (current.type) {
    case JSMN_ARRAY:
        for (; index > 0; --index) {
            token = skip(token);
        }
        return JSON(m_data, 0, token);

    case JSMN_OBJECT:
        for(; index > 0; --index) { 
            token = skip(token + 1);
        }
        return JSON(m_data, token, token + 1);

    default:
        return JSON();
    }
}

JSON JSON::operator[](std::string const& name) const
{
    assert(false == empty());
    assert(Object == type());

    jsmntok_t const& current = m_data->tokens[m_value];

    for (int it = 0, token = m_value + 1; it < current.size; ++it) {
        size_t start = m_data->tokens[token].start;
        size_t end = m_data->tokens[token].end;
        if (0 == strncmp(name.c_str(), &m_data->string[start], end - start)) {
            return JSON(m_data, token, token + 1);
        }
        token = skip(token + 1);
    }

    return JSON();
}

JSON JSON::at(int index) const
{
    if (Array != type() && Object != type()) {
        return JSON();
    }
    if (index < 0 || index >= m_data->tokens[m_value].size) {
        return JSON();
    }

    return (*this)[index];
}

JSON JSON::at(std::string const& name) const
{
    if (Object != type()) {
        return JSON();
    }
    return (*this)[name];
}

void JSON::clear()
{
    m_data.reset();
    m_name = -1;
    m_value = -1;
}

void JSON::parse(std::string string)
{
    jsmn_parser parser;
    int tokens;

    auto throw_error = [this](jsmnerr error)
    {
        clear();

        std::string str;

        switch (error) {
        case JSMN_ERROR_NOMEM: str = "No memory"; break;
        case JSMN_ERROR_INVAL: str = "Invalid character"; break;
        case JSMN_ERROR_PART:  str = "Partial JSON fragment"; break;
        default:
            str = "Unknown error";
            break;
        }
        throw std::runtime_error(fmt::format("Failed to JSON parse string ({})", str));
    };

    // Clear, create new data and assign string.
    clear();
    m_data = std::make_shared<Data>();
    m_data->string = std::move(string);

    // Parse the string to find the number of JSON tokens it contains.
    jsmn_init(&parser);
    tokens = jsmn_parse(&parser, m_data->string.data(), m_data->string.length(), nullptr, 0);
    if (tokens < 0) { 
        throw_error(static_cast<jsmnerr>(tokens));
    }
    m_data->tokens.resize(tokens);

    // Parse the string.
    jsmn_init(&parser);
    tokens = jsmn_parse(&parser, m_data->string.data(), m_data->string.length(), m_data->tokens.data(), m_data->tokens.size());
    if (tokens < 0) {
        throw_error(static_cast<jsmnerr>(tokens));
    }
    assert(static_cast<int>(m_data->tokens.size()) == tokens);

    m_name = -1;
    m_value = 0;
}

