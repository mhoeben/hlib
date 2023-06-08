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

#include "hlib/c/base.h"
#include <cassert>
#include <stdexcept>
#include <string>

namespace hlib
{

#define HLIB_NOT_COPYABLE(name) \
    name(name const &) = delete; \
    name& operator = (name const &) = delete

#define HLIB_NOT_MOVABLE(name) \
    name(name &&) = delete; \
    name& operator = (name &&) = delete

std::string format_assert_string(char const* file, int line, char const* expression);

#define hcheck(expression) \
    do { \
        if (!(expression)) { \
            throw std::runtime_error(hlib::format_assert_string(__FILE__, __LINE__, #expression)); \
        } \
    } while (false)

} // namespace hlib

