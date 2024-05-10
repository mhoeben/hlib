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
#include "hlib_format.hpp"
#include <cstdio>
#include <stdarg.h>

#pragma GCC diagnostic ignored "-Wformat-nonliteral"

//
// Public
//
std::string hlib::format(char const* format_string, ...) noexcept
{
    va_list ap;

    va_start(ap, format_string);
    int formatted_length = vsnprintf(nullptr, 0, format_string, ap);
    va_end(ap);

    if (formatted_length < 0) {
        return std::string();
    }

    std::string string;

    try {
        string.resize(formatted_length + 1);
    }
    catch (...) {
        return std::string();
    }

    va_start(ap, format_string);
    HVERIFY(vsnprintf(string.data(), formatted_length + 1, format_string, ap) >= 0);
    va_end(ap);

    string.pop_back();
    return string;
}

bool hlib::format_to(std::string& string, char const* format_string, ...) noexcept
{
    va_list ap;

    va_start(ap, format_string);
    int formatted_length = vsnprintf(nullptr, 0, format_string, ap);
    va_end(ap);

    if (formatted_length < 0) {
        return false;
    }

    std::size_t const string_length = string.length();

    try {
        string.resize(string_length + formatted_length + 1);
    }
    catch (...) {
        return false;
    }

    va_start(ap, format_string);
    HVERIFY(vsnprintf(string.data() + string_length, formatted_length + 1, format_string, ap) >= 0);
    va_end(ap);

    string.pop_back();
    return true;
}

