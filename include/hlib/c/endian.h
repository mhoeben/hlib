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
#include <stdlib.h>

//
// Big Endian Set
//
inline void* hlib_be_set_int8(void* data, int8_t value)
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)value;
    return ptr + sizeof(int8_t);
}

inline void* hlib_be_set_int16(void* data, int16_t value)
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >>  8);
    ptr[1] = (uint8_t)(value >>  0);
    return ptr + sizeof(int16_t);
}

inline void* hlib_be_set_int32(void* data, int32_t value)
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >> 24);
    ptr[1] = (uint8_t)(value >> 16);
    ptr[2] = (uint8_t)(value >>  8);
    ptr[3] = (uint8_t)(value >>  0);
    return ptr + sizeof(int32_t);
}

inline void* hlib_be_set_int64(void* data, int64_t value)
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >> 56);
    ptr[1] = (uint8_t)(value >> 48);
    ptr[2] = (uint8_t)(value >> 40);
    ptr[3] = (uint8_t)(value >> 32);
    ptr[4] = (uint8_t)(value >> 24);
    ptr[5] = (uint8_t)(value >> 16);
    ptr[6] = (uint8_t)(value >>  8);
    ptr[7] = (uint8_t)(value >>  0);
    return ptr + sizeof(int64_t);
}

inline void* hlib_be_set_uint8(void* data, uint8_t value)
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = value;
    return ptr + sizeof(uint8_t);
}

inline void* hlib_be_set_uint16(void* data, uint16_t value)
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >>  8);
    ptr[1] = (uint8_t)(value >>  0);
    return ptr + sizeof(uint16_t);
}

inline void* hlib_be_set_uint32(void* data, uint32_t value)
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >> 24);
    ptr[1] = (uint8_t)(value >> 16);
    ptr[2] = (uint8_t)(value >>  8);
    ptr[3] = (uint8_t)(value >>  0);
    return ptr + sizeof(uint32_t);
}

inline void* hlib_be_set_uint64(void* data, uint64_t value)
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >> 56);
    ptr[1] = (uint8_t)(value >> 48);
    ptr[2] = (uint8_t)(value >> 40);
    ptr[3] = (uint8_t)(value >> 32);
    ptr[4] = (uint8_t)(value >> 24);
    ptr[5] = (uint8_t)(value >> 16);
    ptr[6] = (uint8_t)(value >>  8);
    ptr[7] = (uint8_t)(value >>  0);
    return ptr + sizeof(uint64_t);
}

inline void* hlib_be_set_size(void* data, size_t value)
{
    uint8_t* ptr = (uint8_t*)data;

#if 8 == __SIZEOF_SIZE_T__
    ptr[0] = (uint8_t)(value >> 56);
    ptr[1] = (uint8_t)(value >> 48);
    ptr[2] = (uint8_t)(value >> 40);
    ptr[3] = (uint8_t)(value >> 32);
    ptr[4] = (uint8_t)(value >> 24);
    ptr[5] = (uint8_t)(value >> 16);
    ptr[6] = (uint8_t)(value >>  8);
    ptr[7] = (uint8_t)(value >>  0);
#else
    ptr[0] = (uint8_t)(value >> 24);
    ptr[1] = (uint8_t)(value >> 16);
    ptr[2] = (uint8_t)(value >>  8);
    ptr[3] = (uint8_t)(value >>  0);
#endif
    return ptr + sizeof(size_t);
}

inline void* hlib_be_set_float(void* data, float value)
{
    union
    {
        float f;
        uint32_t n;
    } convert = { value };
    return hlib_be_set_uint32(data, convert.n);
}

inline void* hlib_be_set_double(void* data, double value)
{
    union
    {
        double f;
        uint64_t n;
    } convert = { value };
    return hlib_be_set_uint64(data, convert.n);
}

//
// Big Endian Get
//
inline void const* hlib_be_get_int8(void const* data, int8_t* value)
{
    uint8_t const* ptr = (uint8_t const*)data;
    *value = (int8_t)ptr[0];
    return ptr + sizeof(int8_t);
}

inline void const* hlib_be_get_int16(void const* data, int16_t* value)
{
    uint8_t const* ptr = (uint8_t const*)data;
    *value = ((int16_t)ptr[0] << 8)
           | ((int16_t)ptr[1] << 0);
    return ptr + sizeof(int16_t);
}

inline void const* hlib_be_get_int32(void const* data, int32_t* value)
{
    uint8_t const* ptr = (uint8_t const*)data;
    *value = ((int32_t)ptr[0] << 24)
           | ((int32_t)ptr[1] << 16)
           | ((int32_t)ptr[2] <<  8)
           | ((int32_t)ptr[3] <<  0);
    return ptr + sizeof(int32_t);
}

inline void const* hlib_be_get_int64(void const* data, int64_t* value)
{
    uint8_t const* ptr = (uint8_t const*)data;
    *value = ((int64_t)ptr[0] << 56)
           | ((int64_t)ptr[1] << 48)
           | ((int64_t)ptr[2] << 40)
           | ((int64_t)ptr[3] << 32)
           | ((int64_t)ptr[4] << 24)
           | ((int64_t)ptr[5] << 16)
           | ((int64_t)ptr[6] <<  8)
           | ((int64_t)ptr[7] <<  0);
    return ptr + sizeof(int64_t);
}

inline void const* hlib_be_get_uint8(void const* data, uint8_t* value)
{
    uint8_t const* ptr = (uint8_t const*)data;
    *value = ptr[0];
    return ptr + sizeof(uint8_t);
}

inline void const* hlib_be_get_uint16(void const* data, uint16_t* value)
{
    uint8_t const* ptr = (uint8_t const*)data;
    *value = ((uint16_t)ptr[0] << 8)
           | ((uint16_t)ptr[1] << 0);
    return ptr + sizeof(uint16_t);
}

inline void const* hlib_be_get_uint32(void const* data, uint32_t* value)
{
    uint8_t const* ptr = (uint8_t const*)data;
    *value = ((uint32_t)ptr[0] << 24)
           | ((uint32_t)ptr[1] << 16)
           | ((uint32_t)ptr[2] <<  8)
           | ((uint32_t)ptr[3] <<  0);
    return ptr + sizeof(uint32_t);
}

inline void const* hlib_be_get_uint64(void const* data, uint64_t* value)
{
    uint8_t const* ptr = (uint8_t const*)data;
    *value = ((uint64_t)ptr[0] << 56)
           | ((uint64_t)ptr[1] << 48)
           | ((uint64_t)ptr[2] << 40)
           | ((uint64_t)ptr[3] << 32)
           | ((uint64_t)ptr[4] << 24)
           | ((uint64_t)ptr[5] << 16)
           | ((uint64_t)ptr[6] <<  8)
           | ((uint64_t)ptr[7] <<  0);
    return ptr + sizeof(uint64_t);
}

inline void const* hlib_be_get_size(void const* data, size_t* value)
{
    uint8_t const* ptr = (uint8_t const*)data;

#if 8 == __SIZEOF_SIZE_T__
    *value = ((size_t)ptr[0] << 56)
           | ((size_t)ptr[1] << 48)
           | ((size_t)ptr[2] << 40)
           | ((size_t)ptr[3] << 32)
           | ((size_t)ptr[4] << 24)
           | ((size_t)ptr[5] << 16)
           | ((size_t)ptr[6] <<  8)
           | ((size_t)ptr[7] <<  0);
#else
    *value = ((size_t)ptr[0] << 24)
           | ((size_t)ptr[1] << 16)
           | ((size_t)ptr[2] <<  8)
           | ((size_t)ptr[3] <<  0);
#endif
    return ptr + sizeof(size_t);
}

inline void const* hlib_be_get_float(void const* data, float* value)
{
    union
    {
        uint32_t n;
        float f;
    } convert;
    data = hlib_be_get_uint32(data, &convert.n);
    *value = convert.f;
    return data;
}

inline void const* hlib_be_get_double(void const* data, double* value)
{
    union
    {
        uint64_t n;
        double f;
    } convert;
    data = hlib_be_get_uint64(data, &convert.n);
    *value = convert.f;
    return data;
}

//
// Little Endian Set
//
inline void* hlib_le_set_int8(void* data, int8_t value)
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)value;
    return ptr + sizeof(int8_t);
}

inline void* hlib_le_set_int16(void* data, int16_t value)
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >>  0);
    ptr[1] = (uint8_t)(value >>  8);
    return ptr + sizeof(int16_t);
}

inline void* hlib_le_set_int32(void* data, int32_t value)
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >>  0);
    ptr[1] = (uint8_t)(value >>  8);
    ptr[2] = (uint8_t)(value >> 16);
    ptr[3] = (uint8_t)(value >> 24);
    return ptr + sizeof(int32_t);
}

inline void* hlib_le_set_int64(void* data, int64_t value)
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >>  0);
    ptr[1] = (uint8_t)(value >>  8);
    ptr[2] = (uint8_t)(value >> 16);
    ptr[3] = (uint8_t)(value >> 24);
    ptr[4] = (uint8_t)(value >> 32);
    ptr[5] = (uint8_t)(value >> 40);
    ptr[6] = (uint8_t)(value >> 48);
    ptr[7] = (uint8_t)(value >> 56);
    return ptr + sizeof(int64_t);
}

inline void* hlib_le_set_uint8(void* data, uint8_t value)
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = value;
    return ptr + sizeof(uint8_t);
}

inline void* hlib_le_set_uint16(void* data, uint16_t value)
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >>  0);
    ptr[1] = (uint8_t)(value >>  8);
    return ptr + sizeof(uint16_t);
}

inline void* hlib_le_set_uint32(void* data, uint32_t value)
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >>  0);
    ptr[1] = (uint8_t)(value >>  8);
    ptr[2] = (uint8_t)(value >> 16);
    ptr[3] = (uint8_t)(value >> 24);
    return ptr + sizeof(uint32_t);
}

inline void* hlib_le_set_uint64(void* data, uint64_t value)
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >>  0);
    ptr[1] = (uint8_t)(value >>  8);
    ptr[2] = (uint8_t)(value >> 16);
    ptr[3] = (uint8_t)(value >> 24);
    ptr[4] = (uint8_t)(value >> 32);
    ptr[5] = (uint8_t)(value >> 40);
    ptr[6] = (uint8_t)(value >> 48);
    ptr[7] = (uint8_t)(value >> 56);
    return ptr + sizeof(uint64_t);
}

inline void* hlib_le_set_size(void* data, size_t value)
{
    uint8_t* ptr = (uint8_t*)data;

    ptr[0] = (uint8_t)(value >>  0);
    ptr[1] = (uint8_t)(value >>  8);
    ptr[2] = (uint8_t)(value >> 16);
    ptr[3] = (uint8_t)(value >> 24);
#if 8 == __SIZEOF_SIZE_T__
    ptr[4] = (uint8_t)(value >> 32);
    ptr[5] = (uint8_t)(value >> 40);
    ptr[6] = (uint8_t)(value >> 48);
    ptr[7] = (uint8_t)(value >> 56);
#endif
    return ptr + sizeof(size_t);
}

inline void* hlib_le_set_float(void* data, float value)
{
    union
    {
        float f;
        uint32_t n;
    } convert = { value };
    return hlib_le_set_uint32(data, convert.n);
}

inline void* hlib_le_set_double(void* data, double value)
{
    union
    {
        double f;
        uint64_t n;
    } convert = { value };
    return hlib_le_set_uint64(data, convert.n);
}

//
// Little Endian Get
//
inline void const* hlib_le_get_int8(void const* data, int8_t* value)
{
    uint8_t const* ptr = (uint8_t const*)data;
    *value = (int8_t)ptr[0];
    return ptr + sizeof(int8_t);
}

inline void const* hlib_le_get_int16(void const* data, int16_t* value)
{
    uint8_t const* ptr = (uint8_t const*)data;
    *value = ((int16_t)ptr[0] << 0)
           | ((int16_t)ptr[1] << 8);
    return ptr + sizeof(int16_t);
}

inline void const* hlib_le_get_int32(void const* data, int32_t* value)
{
    uint8_t const* ptr = (uint8_t const*)data;
    *value = ((int32_t)ptr[0] <<  0)
           | ((int32_t)ptr[1] <<  8)
           | ((int32_t)ptr[2] << 16)
           | ((int32_t)ptr[3] << 24);
    return ptr + sizeof(int32_t);
}

inline void const* hlib_le_get_int64(void const* data, int64_t* value)
{
    uint8_t const* ptr = (uint8_t const*)data;
    *value = ((int64_t)ptr[0] <<  0)
           | ((int64_t)ptr[1] <<  8)
           | ((int64_t)ptr[2] << 16)
           | ((int64_t)ptr[3] << 24)
           | ((int64_t)ptr[4] << 32)
           | ((int64_t)ptr[5] << 40)
           | ((int64_t)ptr[6] << 48)
           | ((int64_t)ptr[7] << 56);
    return ptr + sizeof(int64_t);
}

inline void const* hlib_le_get_uint8(void const* data, uint8_t* value)
{
    uint8_t const* ptr = (uint8_t const*)data;
    *value = ptr[0];
    return ptr + sizeof(uint8_t);
}

inline void const* hlib_le_get_uint16(void const* data, uint16_t* value)
{
    uint8_t const* ptr = (uint8_t const*)data;
    *value = ((uint16_t)ptr[0] << 0)
           | ((uint16_t)ptr[1] << 8);
    return ptr + sizeof(uint16_t);
}

inline void const* hlib_le_get_uint32(void const* data, uint32_t* value)
{
    uint8_t const* ptr = (uint8_t const*)data;
    *value = ((uint32_t)ptr[0] <<  0)
           | ((uint32_t)ptr[1] <<  8)
           | ((uint32_t)ptr[2] << 16)
           | ((uint32_t)ptr[3] << 24);
    return ptr + sizeof(uint32_t);
}

inline void const* hlib_le_get_uint64(void const* data, uint64_t* value)
{
    uint8_t const* ptr = (uint8_t const*)data;
    *value = ((uint64_t)ptr[0] <<  0)
           | ((uint64_t)ptr[1] <<  8)
           | ((uint64_t)ptr[2] << 16)
           | ((uint64_t)ptr[3] << 24)
           | ((uint64_t)ptr[4] << 32)
           | ((uint64_t)ptr[5] << 40)
           | ((uint64_t)ptr[6] << 48)
           | ((uint64_t)ptr[7] << 56);
    return ptr + sizeof(uint64_t);
}

inline void const* hlib_le_get_size(void const* data, size_t* value)
{
    uint8_t const* ptr = (uint8_t const*)data;

    *value = ((size_t)ptr[0] <<  0)
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
    return ptr + sizeof(size_t);
}

inline void const* hlib_le_get_float(void const* data, float* value)
{
    union
    {
        uint32_t n;
        float f;
    } convert;
    data = hlib_le_get_uint32(data, &convert.n);
    *value = convert.f;
    return data;
}

inline void const* hlib_le_get_double(void const* data, double* value)
{
    union
    {
        uint64_t n;
        double f;
    } convert;
    data = hlib_le_get_uint64(data, &convert.n);
    *value = convert.f;
    return data;
}

#endif // HLIB_C_ERROR_IMPL

#ifdef __cplusplus
}
#endif

