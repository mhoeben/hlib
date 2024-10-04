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

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <new>
#include <stdexcept>
#include <string>
#include "hlib/debug.hpp"

namespace hlib
{

#define HLIB_NOT_COPYABLE(name) \
    name(name const &) = delete; \
    name& operator = (name const &) = delete

#define HLIB_NOT_MOVABLE(name) \
    name(name &&) = delete; \
    name& operator = (name &&) = delete

#if defined(__clang__)
    #define HLIB_FALLTHROUGH [[clang::fallthrough]]
#elif defined(__GNUC__) || defined(__GNUG__)
    #if (__GNUC__ < 7)
        #define HLIB_FALLTHROUGH
    #else
        #define HLIB_FALLTHROUGH __attribute__((fallthrough))
    #endif
#else
    #error "Unsupported compiler."
#endif

#if defined(__cpp_rtti) && __cpp_rtti
    #define HLIB_RTTI_ENABLED
#else
    #define HLIB_RTTI_DISABLED
#endif

#define HASSERT(expression) assert((expression))

#ifndef NDEBUG
#define HVERIFY(expression) assert((expression))
#else
#define HVERIFY(expression) do { (void)(expression); } while (false)
#endif

} // namespace hlib

