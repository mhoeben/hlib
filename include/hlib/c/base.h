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

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef HLIB_C_VISIBILITY_STATIC
#define HLIB_C_VISIBILITY static
#else
#define HLIB_C_VISIBILITY extern
#endif

#ifndef NDEBUG
#define hverify(expression)  assert((expression))
#else
#define hverify(expression)  do { (void)(expression); } while (false)
#endif

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

