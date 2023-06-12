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
#ifndef HLIB_C_IMPL
#define HLIB_C_IMPL

#define HLIB_C_BUFFER_IMPL
#include "hlib/c/buffer.h"
#undef HLIB_C_BUFFER_IMPL

#define HLIB_C_CODEC_IMPL
#include "hlib/c/codec.h"
#undef HLIB_C_CODEC_IMPL

#define HLIB_C_CODEC_BINARY_IMPL
#include "hlib/c/codec_binary.h"
#undef HLIB_C_CODEC_BINARY_IMPL

#define HLIB_C_ERROR_IMPL
#include "hlib/c/error.h"
#undef HLIB_C_ERROR_IMPL

#define HLIB_C_VECTOR_IMPL
#include "hlib/c/vector.h"
#undef HLIB_C_VECTOR_IMPL

#endif // HLIB_C_IMPL

