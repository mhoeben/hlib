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

#include "hlib/c/endian.h"

namespace hlib
{
namespace be
{

inline std::int16_t int16(void const* data) { return hlib_be_int16(data); }
inline std::int32_t int32(void const* data) { return hlib_be_int32(data); }
inline std::int64_t int64(void const* data) { return hlib_be_int64(data); }

inline std::uint16_t uint16(void const* data) { return hlib_be_uint16(data); }
inline std::uint32_t uint32(void const* data) { return hlib_be_uint32(data); }
inline std::uint64_t uint64(void const* data) { return hlib_be_uint64(data); }

inline std::size_t size(void const* data) { return hlib_be_size(data); }

} // namespace be

namespace le
{

inline std::int16_t int16(void const* data) { return hlib_le_int16(data); }
inline std::int32_t int32(void const* data) { return hlib_le_int32(data); }
inline std::int64_t int64(void const* data) { return hlib_le_int64(data); }

inline std::uint16_t uint16(void const* data) { return hlib_le_uint16(data); }
inline std::uint32_t uint32(void const* data) { return hlib_le_uint32(data); }
inline std::uint64_t uint64(void const* data) { return hlib_le_uint64(data); }

inline std::size_t size(void const* data) { return hlib_le_size(data); }

} // namespace le
} // namespace hlib

