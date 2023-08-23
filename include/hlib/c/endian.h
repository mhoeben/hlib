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

#ifndef HLIB_C_ENDIAN_H
#define HLIB_C_ENDIAN_H

#include <stdint.h>

inline int16_t hlib_be_int16(void const* data)
{
    uint8_t const* ptr = (uint8_t const*)data;
    return ((int16_t)ptr[0] << 8)
         | ((int16_t)ptr[1] << 0);
}

inline int32_t hlib_be_int32(void const* data)
{
    uint8_t const* ptr = (uint8_t const*)data;
    return ((int32_t)ptr[0] << 24)
         | ((int32_t)ptr[1] << 16)
         | ((int32_t)ptr[2] <<  8)
         | ((int32_t)ptr[3] <<  0);
}

inline int64_t hlib_be_int64(void const* data)
{
    uint8_t const* ptr = (uint8_t const*)data;
    return ((int64_t)ptr[0] << 56)
         | ((int64_t)ptr[1] << 48)
         | ((int64_t)ptr[2] << 40)
         | ((int64_t)ptr[3] << 32)
         | ((int64_t)ptr[4] << 24)
         | ((int64_t)ptr[5] << 16)
         | ((int64_t)ptr[6] <<  8)
         | ((int64_t)ptr[7] <<  0);
}

inline uint16_t hlib_be_uint16(void const* data)
{
    uint8_t const* ptr = (uint8_t const*)data;
    return ((uint16_t)ptr[0] << 8)
         | ((uint16_t)ptr[1] << 0);
}

inline uint32_t hlib_be_uint32(void const* data)
{
    uint8_t const* ptr = (uint8_t const*)data;
    return ((uint32_t)ptr[0] << 24)
         | ((uint32_t)ptr[1] << 16)
         | ((uint32_t)ptr[2] <<  8)
         | ((uint32_t)ptr[3] <<  0);
}

inline uint64_t hlib_be_uint64(void const* data)
{
    uint8_t const* ptr = (uint8_t const*)data;
    return ((uint64_t)ptr[0] << 56)
         | ((uint64_t)ptr[1] << 48)
         | ((uint64_t)ptr[2] << 40)
         | ((uint64_t)ptr[3] << 32)
         | ((uint64_t)ptr[4] << 24)
         | ((uint64_t)ptr[5] << 16)
         | ((uint64_t)ptr[6] <<  8)
         | ((uint64_t)ptr[7] <<  0);
}

inline uint64_t hlib_be_size(void const* data)
{
    uint8_t const* ptr = (uint8_t const*)data;

    return ((size_t)ptr[0] << 56)
         | ((size_t)ptr[1] << 48)
         | ((size_t)ptr[2] << 40)
         | ((size_t)ptr[3] << 32)
#if 8 == __SIZEOF_SIZE_T__
         | ((size_t)ptr[4] << 24)
         | ((size_t)ptr[5] << 16)
         | ((size_t)ptr[6] <<  8)
         | ((size_t)ptr[7] <<  0)
#endif
         ;
}

inline int16_t hlib_le_int16(void const* data)
{
    uint8_t const* ptr = (uint8_t const*)data;
    return ((int16_t)ptr[0] << 0)
         | ((int16_t)ptr[1] << 8);
}

inline int32_t hlib_le_int32(void const* data)
{
    uint8_t const* ptr = (uint8_t const*)data;
    return ((int32_t)ptr[0] <<  0)
         | ((int32_t)ptr[1] <<  8)
         | ((int32_t)ptr[2] << 16)
         | ((int32_t)ptr[3] << 24);
}

inline int64_t hlib_le_int64(void const* data)
{
    uint8_t const* ptr = (uint8_t const*)data;
    return ((int64_t)ptr[0] <<  0)
         | ((int64_t)ptr[1] <<  8)
         | ((int64_t)ptr[2] << 16)
         | ((int64_t)ptr[3] << 24)
         | ((int64_t)ptr[4] << 32)
         | ((int64_t)ptr[5] << 40)
         | ((int64_t)ptr[6] << 48)
         | ((int64_t)ptr[7] << 56);
}

inline uint16_t hlib_le_uint16(void const* data)
{
    uint8_t const* ptr = (uint8_t const*)data;
    return ((uint16_t)ptr[0] << 0)
         | ((uint16_t)ptr[1] << 8);
}

inline uint32_t hlib_le_uint32(void const* data)
{
    uint8_t const* ptr = (uint8_t const*)data;
    return ((uint32_t)ptr[0] <<  0)
         | ((uint32_t)ptr[1] <<  8)
         | ((uint32_t)ptr[2] << 16)
         | ((uint32_t)ptr[3] << 24);
}

inline uint64_t hlib_le_uint64(void const* data)
{
    uint8_t const* ptr = (uint8_t const*)data;
    return ((uint64_t)ptr[0] <<  0)
         | ((uint64_t)ptr[1] <<  8)
         | ((uint64_t)ptr[2] << 16)
         | ((uint64_t)ptr[3] << 24)
         | ((uint64_t)ptr[4] << 32)
         | ((uint64_t)ptr[5] << 40)
         | ((uint64_t)ptr[6] << 48)
         | ((uint64_t)ptr[7] << 56);
}

inline uint64_t hlib_le_size(void const* data)
{
    uint8_t const* ptr = (uint8_t const*)data;

    return ((size_t)ptr[0] <<  0)
         | ((size_t)ptr[1] <<  8)
         | ((size_t)ptr[2] << 16)
         | ((size_t)ptr[3] << 24)
#if 8 == __SIZEOF_SIZE_T__
         | ((size_t)ptr[4] << 32)
         | ((size_t)ptr[5] << 40)
         | ((size_t)ptr[6] << 48)
         | ((size_t)ptr[7] << 56)
#endif
         ;
}

#endif // HLIB_C_ERROR_IMPL

#ifdef __cplusplus
}
#endif

