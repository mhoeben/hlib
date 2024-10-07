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

#include "hlib/result.hpp"
#include <functional>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace hlib
{

class Usage final
{
public:
    typedef std::variant<
        std::function<Result<bool>(std::string const& value)>,
        std::function<Result<bool>(std::int64_t value)>,
        std::function<Result<bool>(double value)>,
        std::function<Result<bool>()>
    > Callback;

    class Option
    {
    public:
        Option(char tag_short, std::string tag_long, std::string description, Callback callback);
        Option(char tag_short, std::string description, Callback callback);
        Option(std::string tag_long, std::string description, Callback callback);

    private:
        char m_short{ 0 };
        std::string m_long;
        std::string m_argument;
        std::string m_description;
        Callback m_callback;
    };

    class Argument
    {
    public:
        Argument(std::string tag, std::string description, bool optional = false);
        Argument(std::string tag, std::string description, Callback callback, bool optional = false);

    private:
        std::string m_tag;
        std::string m_description;
        Callback m_callback;
        bool m_optional;
    };

public:
    Usage(std::string description, std::vector<Option> options, std::vector<Argument> arguments);

    std::optional<Option> option() const;
    std::optional<Argument> argument() const;

    std::string string(std::string const& executable) const;
    Result<> parse(int argc, char** argv) const;

private:
    std::string m_description;
    std::vector<Option> m_options;
    std::vector<Argument> m_arguments;
};

} // namespace hlib

