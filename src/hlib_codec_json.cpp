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
#include "hlib_codec_json.hpp"
#include "hlib/format.hpp"

using namespace hlib;
using namespace hlib::codec;

//
// Implementation
//
template<typename T>
void JSONEncoder::encode(char const* name, T const& value)
{
    if (true == m_state.map) {
        assert(nullptr != name);
        append_to(m_buffer, "{}\"{}\": {}{}", m_indent, name, value, newline());
    }
    else {
        append_to(m_buffer, "{}{}{}", m_indent, value, newline());
    }

    ++m_state.index;
}

char const* JSONEncoder::newline() const
{
    assert(m_state.index <= m_state.size);
    return m_state.index + 1 < m_state.size ? ",\n" : "\n";
}

//
// Public (JSONEncoder)
//
JSONEncoder::JSONEncoder(Buffer& buffer)
    : m_buffer(buffer)
{
}

void JSONEncoder::open(char const* name, Type const& value)
{
    return open(name, Map(static_cast<std::size_t>(value)));
}

void JSONEncoder::open(char const* name, Array const& value)
{
    (void)value;

    if (true == m_state.map) {
        assert(nullptr != name);
        append_to(m_buffer, "{}\"{}\": [\n", m_indent, name);
    }
    else {
        append_to(m_buffer, "{}[\n", m_indent);
    }

    m_stack.push_back(m_state);

    m_state.size = value.size;
    m_state.map = false;
    m_state.index = 0;
    m_indent = std::string(m_stack.size() * 4, ' ');
}

void JSONEncoder::open(char const* name, Map const& value)
{
    (void)value;

    if (true == m_state.map) {
        assert(nullptr != name);
        append_to(m_buffer, "{}\"{}\": {{\n", m_indent, name);
    }
    else {
        append_to(m_buffer, "{}{{\n", m_indent);
    }

    m_stack.push_back(m_state);

    m_state.size = value.size;
    m_state.map = true;
    m_state.index = 0;
    m_indent = std::string(m_stack.size() * 4, ' ');
}

void JSONEncoder::encode(char const* name, bool const& value)
{
    encode<>(name, false != value ? "true" : "false");
}

void JSONEncoder::encode(char const* name, std::int32_t const& value)
{
    encode<>(name, value);
}

void JSONEncoder::encode(char const* name, std::int64_t const& value)
{
    encode<>(name, value);
}

void JSONEncoder::encode(char const* name, float const& value)
{
    encode<>(name, value);
}

void JSONEncoder::encode(char const* name, double const& value)
{
    encode<>(name, value);
}

void JSONEncoder::encode(char const* name, std::string const& value)
{
    if (true == m_state.map) {
        assert(nullptr != name);
        append_to(m_buffer, "{}\"{}\": \"{}\"{}", m_indent, name, value, newline());
    }
    else {
        append_to(m_buffer, "{}\"{}\"{}", m_indent, value, newline());
    }

    ++m_state.index;
}

void JSONEncoder::encode(char const* name, Binary const& value)
{
    assert(false); // TODO
    (void)name;
    (void)value;
}

void JSONEncoder::close()
{
    assert(false == m_stack.empty());
    assert(m_state.size == m_state.index);

    bool map = m_state.map;

    m_state = m_stack.back();
    m_stack.pop_back();

    m_indent = std::string(m_stack.size() * 4, ' ');

    if (true == map) {
        append_to(m_buffer, "{}}}{}", m_indent, newline());
    }
    else {
        append_to(m_buffer, "{}]{}", m_indent, newline());
    }

    ++m_state.index;
}

//
// Implementation (JSONDecoder)
//
template<typename T>
void JSONDecoder::decode(char const* name, T& value)
{
    JSON node;

    switch (m_node.type()) {
    case JSON::Object:
        assert(nullptr != name);
        value = m_node.at(name).as<T>();
        break;

    case JSON::Array:
        value = m_node.at(m_index).as<T>();
        break;

    default:
        assert(false);
        break;
    }

    ++m_index;
}

//
// Public (JSONDecoder)
//
JSONDecoder::JSONDecoder(void const* data, std::size_t size)
    : m_root(std::string(static_cast<char const*>(data), size))
{
}

void JSONDecoder::reset(void const* data, std::size_t size)
{
    m_root.parse(std::string(static_cast<char const*>(data), size));
    m_node = JSON();
    m_index = 0;
    m_stack.clear();
}

void JSONDecoder::open(char const* name, Type& value)
{
    Map map;
    open(name, map);
    assert(static_cast<std::size_t>(value) == map.size);
}

void JSONDecoder::open(char const* name, Array& value)
{
    m_stack.push_back({ m_node, m_index });

    switch (m_node.type()) {
    case JSON::Undefined:
        m_node = m_root;
        break;

    case JSON::Object:
        assert(nullptr != name);
        m_node = m_node.at(name);
        break;

    case JSON::Array:
        m_node = m_node.at(m_index);
        break;

    default:
        assert(false);
        break;
    }

    assert(JSON::Array == m_node.type());
    m_index = 0;

    value.size = m_node.size();
}

void JSONDecoder::open(char const* name, Map& value)
{
    m_stack.push_back({ m_node, m_index });

    switch (m_node.type()) {
    case JSON::Undefined:
        m_node = m_root;
        break;

    case JSON::Object:
        assert(nullptr != name);
        m_node = m_node.at(name);
        break;

    case JSON::Array:
        m_node = m_node.at(m_index);
        break;

    default:
        assert(false);
        break;
    }

    assert(JSON::Object == m_node.type());
    m_index = 0;

    value.size = m_node.size();
}

void JSONDecoder::decode(char const* name, bool& value)
{
    decode<>(name, value);
}

void JSONDecoder::decode(char const* name, std::int32_t& value)
{
    decode<>(name, value);
}

void JSONDecoder::decode(char const* name, std::int64_t& value)
{
    decode<>(name, value);
}

void JSONDecoder::decode(char const* name, float& value)
{
    decode<>(name, value);
}

void JSONDecoder::decode(char const* name, double& value)
{
    decode<>(name, value);
}

void JSONDecoder::decode(char const* name, std::string& value)
{
    decode<>(name, value);
}

void JSONDecoder::decode(char const* name, Binary& value)
{
    assert(false); // TODO
    (void)name;
    (void)value;
}

void JSONDecoder::close()
{
    assert(false == m_stack.empty());
    assert(m_node.size() == m_index);

    State const& state = m_stack.back();
    m_node = state.node;
    m_index = state.index;

    m_stack.pop_back();
    ++m_index;
}

Type::Id JSONDecoder::peek() const
{
    if (JSON::Array != m_root.type()) {
        throwf<std::runtime_error>("Cannot unwrap non-array root data");
    }
    JSON id = m_root.at(0);
    if (JSON::Number != id.type()) {
        throwf<std::runtime_error>("Data is not a wrapped codec::Type");
    }

    return id.as<Type::Id>();
}

