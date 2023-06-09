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
#ifdef __cplusplus
extern "C"
{
#endif

#include "hlib/c/base.h"

#ifndef HLIB_C_ERROR_H
#define HLIB_C_ERROR_H

typedef enum hlib_error_e
{
    HLIB_ERROR_NONE,
    HLIB_ERROR_BAD_ALLOC,
    HLIB_ERROR_PARSING
} hlib_error_t;

HLIB_C_VISIBILITY char const* hlib_error_to_string(hlib_error_t error);

#endif // HLIB_C_ERROR_H

#ifdef HLIB_C_ERROR_IMPL

#ifndef HLIB_C_ERROR_IMPL_ONCE
#define HLIB_C_ERROR_IMPL_ONCE

char const* hlib_error_to_string(hlib_error_t error)
{
    switch (error) {
    case HLIB_ERROR_NONE:       return "";
    case HLIB_ERROR_BAD_ALLOC:  return "Bad alloc";
    case HLIB_ERROR_PARSING:    return "Parse error";
    default:
        assert(0);
        return "";
    }
}

#endif // HLIB_C_ERROR_IMPL_ONCE
#endif // HLIB_C_ERROR_IMPL

#ifdef __cplusplus
}
#endif
