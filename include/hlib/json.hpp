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

#include "hlib/string.hpp"
#include <memory>
#include <variant>
#include <vector>

namespace hlib
{

struct JSONData;

class JSONIterator;
class JSONPointer;

class JSON final
{
    friend class JSONIterator;

public:
    enum Type
    {
        Undefined,
        Null,
        Bool,
        Number,
        String,
        Array,
        Object
    };

    using Iterator = JSONIterator;

public:
    JSON() = default;
    JSON(std::string string);
    JSON(JSON const& that);
    JSON(JSON&& that) noexcept;

    JSON& operator =(JSON const& that);
    JSON& operator =(JSON&& that) noexcept;

    bool empty() const noexcept;
    Type type() const noexcept;
    std::string name() const;
    std::string value() const;
    std::size_t size() const;

    int line() const;
    int column() const;
    int start() const;
    int stop() const;

    bool operator ==(JSON const& that) const;
    bool operator !=(JSON const& that) const;

    JSON operator[](int index) const;
    JSON operator[](std::string const& name) const;
    JSON operator[](JSONPointer const& name) const;

    JSON at(int index) const;
    JSON at(std::string const& name) const;
    JSON at(JSONPointer const& pointer) const;

    Iterator begin() const;
    Iterator end() const;

    Iterator find(std::string const& name) const;
    bool contains(std::string const& name) const;

    void clear();
    void parse(std::string string);

    std::string dump(int indent_count = -1, char indent_char = ' ') const;

private:
    std::shared_ptr<JSONData> m_data;
    int m_name{ -1 };
    int m_value{ -1 };

    JSON(std::shared_ptr<JSONData> data, int name, int value);

    void getPosition(int& line, int& column, int position) const;
};

template<typename T, typename U>
typename std::enable_if<!std::is_same<std::string, T>::value
                     &&  std::is_same<JSON, U>::value, std::optional<T>>::type
json_to(JSON const& json, std::nothrow_t) noexcept
{
    return string_to<T>(json.value(), std::nothrow);
}

template<typename T, typename U>
typename std::enable_if<std::is_same<std::string, T>::value
                    &&  std::is_same<JSON, U>::value, std::optional<T>>::type
json_to(U const& json, std::nothrow_t) noexcept
{
    try {
        return json.value();
    }
    catch (...) {
        return std::nullopt;
    }
}

template<typename T, typename U>
typename std::enable_if<!std::is_same<std::string, T>::value
                      && std::is_same<JSON, U>::value, T>::type
json_to(U const& json)
{
    return string_to<T>(json.value());
}

template<typename T, typename U>
typename std::enable_if<std::is_same<std::string, T>::value
                     && std::is_same<JSON, U>::value, T>::type
json_to(U const& json)
{
    return json.value();
}

inline std::string to_string(JSON const& json)
{
    return json.value();
}

class JSONIterator final
{
    friend class JSON;

public:
    JSON const* operator ->() const;
    JSON const& operator *() const;

    bool operator ==(JSONIterator const& that) const;
    bool operator !=(JSONIterator const& that) const;

    JSONIterator& operator++();

private:
    int m_index{ 0 };
    int m_value{ -1 };
    JSON m_current;

    JSONIterator() = default;
    JSONIterator(std::shared_ptr<JSONData> data, int value);
    int skip(int token) const;
};

template<typename T>
std::optional<T> to(JSONIterator const& it)
{
    return to<T>(*it);
}

inline std::string to_string(JSONIterator const& it)
{
    return it->value();
}

class JSONPointer final
{
public:
    typedef std::variant<std::size_t, std::string> Token;
    typedef std::vector<Token>::const_iterator Iterator;

public:
    JSONPointer() = default;
    JSONPointer(std::string pointer);

    bool empty() const noexcept;
    bool isDocument() const noexcept;
    std::size_t size() const noexcept;
    Token const& back() const noexcept;

    void clear() noexcept;
    void pushBack(std::size_t index);
    void pushBack(std::string name);
    void pushBack(Token token);
    void popBack() noexcept;

    Token const& operator[](std::size_t index) const noexcept;

    Iterator begin() const;
    Iterator end() const;

    void parse(std::string const& pointer);

private:
    std::vector<Token> m_tokens;

    void unescape(Token& token);
};

JSON operator"" _json(char const* pointer, std::size_t size);
JSONPointer operator"" _json_pointer(char const* pointer, std::size_t size);

} // namespace hlib

