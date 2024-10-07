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
#include "hlib/usage.hpp"
#include <sstream>

using namespace hlib;

//
// Public (Usage::Option)
//
Usage::Option::Option(char tag_short, std::string tag_long, std::string description, Callback callback)
    : m_short{ tag_short }
    , m_long(std::move(tag_long))
    , m_description(std::move(description))
    , m_callback(std::move(callback))
{
    (void)m_short;
}

Usage::Option::Option(char tag_short, std::string description, Callback callback)
    : m_short{ tag_short }
    , m_description(std::move(description))
    , m_callback(std::move(callback))
{
}

Usage::Option::Option(std::string tag_long, std::string description, Callback callback)
    : m_long(std::move(tag_long))
    , m_description(std::move(description))
    , m_callback(std::move(callback))
{
}

//
// Public (Usage::Argument)
//
Usage::Argument::Argument(std::string tag, std::string description, bool optional)
    : m_tag(std::move(tag))
    , m_description(std::move(description))
    , m_optional{ optional }
{
    (void)m_optional;
}

Usage::Argument::Argument(std::string tag, std::string description, Callback callback, bool optional)
    : m_tag(std::move(tag))
    , m_description(std::move(description))
    , m_callback(std::move(callback))
    , m_optional{ optional }
{
}

//
// Public (Usage)
//
Usage::Usage(std::string description, std::vector<Option> options, std::vector<Argument> arguments)
    : m_description(std::move(description))
    , m_options(std::move(options))
    , m_arguments(std::move(arguments))
{
}

std::string Usage::string(std::string const& executable) const
{
    std::stringstream s;

    s << "Usage: " << executable;

    if (false == m_options.empty()) {
        s << " [OPTIONS]";
    }

    if (false == m_arguments.empty()) {
        s << " [ARGUMENTS]";
    }

    s << std::endl;

    if (false == m_description.empty()) {
        s << std::endl;
        s << "Description:" << std::endl;
        s << "\t" << m_description << std::endl;
    }

    if (false == m_options.empty()) {
        s << std::endl;
        s << "Options:" << std::endl;
    }

    return s.str();
}


Result<> Usage::parse(int argc, char** argv) const
{
    (void)argc;
    (void)argv;

    return {};
}

