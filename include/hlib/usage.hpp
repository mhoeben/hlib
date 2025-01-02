//
// MIT License
//
// Copyright (c) 2024 Maarten Hoeben
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

#include "hlib/result.hpp"
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace hlib
{

class Usage final
{
public:
    enum Type
    {
        None,
        String,
        Integer,
        Float,
        Boolean
    };
    typedef std::variant<std::monostate, std::string, std::int64_t, double, bool> Value;

public:
    class Option
    {
        friend class Usage;

    public:
        Option(char a_brief, std::string a_extended, std::string a_description, std::string a_arg_name = "", Value a_arg_value = "");

    private:
        char const brief{ 0 };
        std::string const extended;
        std::string const description;
        std::string const arg_name;
        Value const arg_value;
    };

    class Argument
    {
        friend class Usage;

    public:
        Argument(std::string a_name, std::string a_description, bool a_optional = false);

    private:
        std::string const name;
        std::string const description;
        bool const optional;
    };

public:
    Usage(std::string description, std::vector<Option> options, std::vector<Argument> arguments, bool varargs = false);

    bool has(char const brief) const noexcept;
    bool has(std::string const& extended) const noexcept;
    bool has(std::size_t argument_index) const noexcept;

    template<typename T = std::string>
    std::optional<T> get(char const brief) const
    {
        if (m_options.size() != m_option_values.size()) {
            return std::nullopt;
        }

        for (std::size_t i = 0; i < m_options.size(); ++i) {
            if (brief == m_options[i].brief) {
                return to<T>(m_option_values[i]);
            }
        }

        return std::nullopt;
    }

    template<typename T = std::string>
    std::optional<T> get(std::string const& extended) const
    {
        if (m_options.size() != m_option_values.size()) {
            return std::nullopt;
        }

        for (std::size_t i = 0; i < m_options.size(); ++i) {
            if (extended == m_options[i].extended) {
                return to<T>(m_option_values[i]);
            }
        }

        return std::nullopt;
    }

    std::string get(std::size_t index) const;

    Result<std::size_t> parse(int argc, char const* const* argv);

    std::string toString(std::string const& executable) const;

private:
    std::string m_description;
    std::vector<Option> m_options;
    std::vector<Argument> m_arguments;
    bool m_varargs;

    std::vector<bool> m_option_set;
    std::vector<Value> m_option_values;
    std::vector<std::string> m_argument_values;

    template<typename T>
    std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>, std::optional<T>>
        to(Value const& value) const
    {
        if (false == std::holds_alternative<std::int64_t>(value)) {
            return std::nullopt;
        }

        return static_cast<T>(std::get<std::int64_t>(value));
    }

    template<typename T>
    std::enable_if_t<std::is_floating_point_v<T>, std::optional<T>>
        to(Value const& value) const
    {
        if (false == std::holds_alternative<double>(value)) {
            return std::nullopt;
        }

        return static_cast<T>(std::get<double>(value));
    }

    template<typename T>
    std::enable_if_t<std::is_same_v<T, std::string> || std::is_same_v<T, bool>, std::optional<T>>
        to(Value const& value) const
    {
        if (false == std::holds_alternative<T>(value)) {
            return std::nullopt;
        }

        return std::get<T>(value);
    }
};

} // namespace hlib

