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
#include "hlib/usage.hpp"
#include "hlib_format.hpp"
#include "hlib/string.hpp"
#include <getopt.h>
#include <sstream>

using namespace hlib;

//
// Implementation
//
namespace
{


} // namespace

//
// Public (Usage::Option)
//
Usage::Option::Option(char a_brief, std::string a_extended, std::string a_description, std::string a_arg_name, Value a_arg_value)
    : brief{ a_brief }
    , extended(std::move(a_extended))
    , description(std::move(a_description))
    , arg_name(std::move(a_arg_name))
    , arg_value(std::move(a_arg_value))
{
}

//
// Public (Usage::Argument)
//
Usage::Argument::Argument(std::string a_name, std::string a_description, bool a_optional)
    : name(std::move(a_name))
    , description(std::move(a_description))
    , optional{ a_optional }
{
}

//
// Public (Usage)
//
Usage::Usage(std::string description, std::vector<Option> options, std::vector<Argument> arguments, bool varargs)
    : m_description(std::move(description))
    , m_options(std::move(options))
    , m_arguments(std::move(arguments))
    , m_varargs{ varargs }
{
}

bool Usage::has(char const brief) const noexcept
{
    if (m_options.size() != m_option_values.size()) {
        return false;
    }

    for (std::size_t i = 0; i < m_options.size(); ++i) {
        if (brief == m_options[i].brief && true == m_option_set[i]) {
            return true;
        }
    }

    return false;
}

bool Usage::has(std::string const& extended) const noexcept
{
    if (m_options.size() != m_option_values.size()) {
        return false;
    }

    for (std::size_t i = 0; i < m_options.size(); ++i) {
        if (extended == m_options[i].extended && true == m_option_set[i]) {
            return true;
        }
    }

    return false;
}

Result<std::size_t> Usage::parse(int argc, char const* const* argv)
{
    std::string arg;
    std::string value;
    std::size_t index;
    int i;

    m_option_set.clear();
    m_option_set.resize(m_options.size(), false);

    m_option_values.clear();
    for (Option const& option: m_options) {
        m_option_values.push_back(option.arg_value);
    }

    m_argument_values.clear();

    for (i = 1; i < argc; ++i) {
        arg = argv[i];

        if (false ==  starts_with(arg, '-')) {
            break;
        }

        index = arg.find_first_of('=');
        if (std::string::npos != index) {
            value = arg.substr(index + 1);
            arg = arg.substr(0, index);
        }
        else {
            value.clear();
        }

        if (false == starts_with(arg, "--")) {
            if (2 == arg.length()) {
                for (index = 0; index < m_options.size(); ++index) {
                    if (m_options[index].brief == arg.back()) {
                        break;
                    }
                }
            }
            else {
                index = m_options.size();
            }
        }
        else {
            for (index = 0; index < m_options.size(); ++index) {
                if (m_options[index].extended == arg.substr(2)) {
                    break;
                }
            }
        }

        if (index >= m_options.size()) {
            return Error(std::invalid_argument(format("invalid option '%s'", arg.c_str())));
        }

        m_option_set[index] = true;

        if (false == m_options[index].arg_name.empty()) {
            if (true == value.empty()) {
                if (index + 1 >= static_cast<std::size_t>(argc)) {
                    return Error(std::invalid_argument(format("option '%s' requires an argument", arg.c_str())));
                }
                value = argv[++i];
            }

            std::optional<Value> result;

            switch (m_options[index].arg_value.index()) {
            case None:
                return Error(std::invalid_argument(format("option '%s' does not take an argument", arg.c_str())));
            case String:    result = value; break;
            case Integer:   result = stoi64(value, 0, std::nothrow); break;
            case Float:     result = stof64(value, std::nothrow); break;
            case Boolean:   result = stob(value, std::nothrow); break;
            default:
                assert(false);
                return Error(std::logic_error("invalid option argument index"));
            }
            if (std::nullopt == result) {
                return Error(std::invalid_argument(format("invalid '%s' argument '%s'", arg.c_str(), value.c_str())));
            }

            m_option_values[index] = std::move(*result);
        }
    }

    bool optional = false;

    for (index = 0; i < argc && index < m_arguments.size(); ++i, ++index) {
        m_argument_values.emplace_back(argv[i]);
        optional |= m_arguments[index].optional;
    }

    if (index < m_arguments.size() && false == (optional|m_arguments[index].optional)) {
        return Error(std::invalid_argument(format("missing argument '%s'", m_arguments[index].name.c_str())));
    }

    if (argc > i) {
        if (false == m_varargs) {
            return Error(std::invalid_argument(format("too many arguments")));
        }

        for (; i < argc; ++i) {
            m_argument_values.emplace_back(argv[i]);
            ++index;
        }
    }

    return index;
}

std::string Usage::toString(std::string const& executable) const
{
    std::stringstream s;
    std::string string;

    s << "Usage: " << executable;

    if (false == m_options.empty()) {
        s << " [OPTIONS]";
    }

    if (false == m_arguments.empty()) {
        std::size_t optional = 0;
        for (Argument const& argument: m_arguments) {
            if (true == argument.optional) {
                s << " [" << argument.name;
                ++optional;
            }
            else {
                s << ' ' << argument.name;
            }
        }

        if (true == m_varargs) {
            s << "...";
        }

        s << std::string(optional, ']');
    }

    s << std::endl;

    if (false == m_description.empty()) {
        s << std::endl;
        s << "Description:" << std::endl;
        s << "    " << m_description << std::endl;
    }

    if (false == m_options.empty()) {
        auto prefix = [](Option const& option) -> std::string {
            std::string str = 0 != option.brief ? format("-%c", option.brief) : "  ";
            if (false == option.extended.empty()) {
                str += 0 != option.brief ? ", ":"  ";
                str += format("--%s", option.extended.c_str());
            }
            if (false == option.arg_name.empty()) {
                str += format("=%s", option.arg_name.c_str());
            }
            return str;
        };

        std::size_t length = 0;
        for (Option const& option : m_options) {
            length = std::max(prefix(option).length(), length);
        }

        s << std::endl;
        s << "Options:" << std::endl;
        for (Option const& option : m_options) {
            s << "  " << fill_right(prefix(option), length + 1) << option.description << std::endl;
        }
    }

    if (false == m_arguments.empty()) {
        std::size_t length = 0;
        for(Argument const& argument: m_arguments) {
            length = std::max(argument.name.length(), length);
        }

        s << std::endl;
        s << "Arguments:" << std::endl;
        for(Argument const& argument: m_arguments) {
            s << "  " << fill_right(argument.name, length + 1) << argument.description << std::endl;
        }
    }

    return s.str();
}

