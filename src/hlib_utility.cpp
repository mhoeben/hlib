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
#include "hlib/utility.hpp"
#include <stdlib.h>
#include <string.h>

using namespace hlib;

//
// Public
//
template<>
std::string hlib::get_env(std::string const& name, std::string const& alt)
{
    char const* value = getenv(name.c_str());
    if (nullptr == value) {
        return alt;
    }
    return value;
}

template<>
int32_t hlib::get_env(std::string const& name, int32_t const& alt)
{
    char const* value = getenv(name.c_str());
    if (nullptr == value) {
        return alt;
    }

    try {
        return std::stol(value);
    }
    catch (...) {
        return alt;
    }
}

template<>
int64_t hlib::get_env(std::string const& name, int64_t const& alt)
{
    char const* value = getenv(name.c_str());
    if (nullptr == value) {
        return alt;
    }

    try {
        return std::stoll(value);
    }
    catch (...) {
        return alt;
    }
}

template<>
double hlib::get_env(std::string const& name, double const& alt)
{
    char const* value = getenv(name.c_str());
    if (nullptr == value) {
        return alt;
    }

    try {
        return std::stod(value);
    }
    catch (...) {
        return alt;
    }
}

template<>
bool hlib::get_env(std::string const& name, bool const& alt)
{
    char const* value = getenv(name.c_str());
    if (nullptr == value) {
        return alt;
    }

    if (0 == strcmp("1", value) || 0 == strcasecmp("true", value)) {
        return true;
    }
    if (0 == strcmp("0", value) || 0 == strcasecmp("false", value)) {
        return true;
    }
    return alt;
}

template<>
size_t hlib::get_env(std::string const& name, size_t const& alt)
{
    char const* value = getenv(name.c_str());
    if (nullptr == value) {
        return alt;
    }

    try {
        return static_cast<size_t>(std::stoull(value));
    }
    catch (...) {
        return alt;
    }
}

