//
// MIT License
//
// Copyright (c) 2019 Maarten Hoeben
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
//
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef HLIB_C_BUFFER_H
#define HLIB_C_BUFFER_H

#include "hlib/c/base.h"
#include <stdlib.h>

typedef struct hlib_buffer_s
{
    size_t capacity;    // Never modify directly.
    size_t size;        // Shall be <= capacity.
    void* data;         // Never modify directly.
} hlib_buffer_t;

#define HLIB_BUFFER_INIT { 0 }

HLIB_C_VISIBILITY void hlib_buffer_init(hlib_buffer_t* buffer);
HLIB_C_VISIBILITY void hlib_buffer_free(hlib_buffer_t* buffer);

HLIB_C_VISIBILITY hlib_buffer_t* hlib_buffer_create(void);
HLIB_C_VISIBILITY void hlib_buffer_destroy(hlib_buffer_t* buffer);

HLIB_C_VISIBILITY int hlib_buffer_copy(hlib_buffer_t* buffer, hlib_buffer_t const* that);
HLIB_C_VISIBILITY void hlib_buffer_move(hlib_buffer_t* buffer, hlib_buffer_t* that);

HLIB_C_VISIBILITY void hlib_buffer_clear(hlib_buffer_t* buffer);
HLIB_C_VISIBILITY int hlib_buffer_shrink(hlib_buffer_t* buffer);
HLIB_C_VISIBILITY void* hlib_buffer_reserve(hlib_buffer_t* buffer, size_t capacity);
HLIB_C_VISIBILITY void* hlib_buffer_resize(hlib_buffer_t* buffer, size_t size);
HLIB_C_VISIBILITY void* hlib_buffer_extend(hlib_buffer_t* buffer, size_t capacity);
HLIB_C_VISIBILITY int hlib_buffer_assign(hlib_buffer_t* buffer, void const* data, size_t size);
HLIB_C_VISIBILITY int hlib_buffer_append(hlib_buffer_t* buffer, void const* data, size_t size);
HLIB_C_VISIBILITY int hlib_buffer_insert(hlib_buffer_t* buffer, size_t offset, void const* data, size_t size);
HLIB_C_VISIBILITY void hlib_buffer_erase(hlib_buffer_t* buffer, size_t offset, size_t size);

HLIB_C_VISIBILITY int hlib_buffer_strcpy(hlib_buffer_t* buffer, char const* string);
HLIB_C_VISIBILITY int hlib_buffer_strncpy(hlib_buffer_t* buffer, char const* string, size_t max);
HLIB_C_VISIBILITY int hlib_buffer_printf(hlib_buffer_t* buffer, char const* format, ...);
HLIB_C_VISIBILITY int hlib_buffer_appendf(hlib_buffer_t* buffer, char const* format, ...);

#endif // HLIB_C_BUFFER_H

#ifdef HLIB_C_BUFFER_IMPL

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

//
// Implementation
//
static void* hlib_buffer_realloc(hlib_buffer_t* buffer, size_t capacity, int shrink)
{
    if (0 == shrink && capacity <= buffer->capacity) {
        return buffer->data;
    }

    void* data;

    if (capacity > 0) {
        data = realloc(buffer->data, capacity);
        if (NULL == data) {
            return NULL;
        }
    }
    else {
        if (NULL != buffer->data) {
            free(buffer->data);
        }
        data = NULL;
    }

    buffer->data = data;
    buffer->capacity = capacity;
    if (buffer->capacity < buffer->size) {
        buffer->size = buffer->capacity;
    }
    return data;
}

static int hlib_buffer_vappendf(hlib_buffer_t* buffer, char const* format, va_list ap)
{
    ssize_t length;

    va_list apl;
    va_copy(apl, ap);

    length = vsnprintf(((char*)buffer->data) + buffer->size,
        buffer->capacity - buffer->size, format, apl);
    if (length < 0) {
        return -1;
    }

    // Take into account that vsnprintf also needs space for a 0 terminator, */
    // but don't include the zero terminator in the size. */
    if (buffer->size + length + 1 < buffer->capacity) {
        buffer->size += length;
        return 0;
    }

    if (NULL == hlib_buffer_realloc(buffer, buffer->size + length + 1, 0)) {
        return -1;
    }

    length = vsnprintf(((char*)buffer->data) + buffer->size,
        buffer->capacity - buffer->size, format, ap);
    if (length < 0) {
        return -1;
    }

    buffer->size += length;
    return length;
}

//
// Public
//
HLIB_C_VISIBILITY_IMPL void hlib_buffer_init(hlib_buffer_t* buffer)
{
    memset(buffer, 0, sizeof(*buffer));
}

HLIB_C_VISIBILITY_IMPL void hlib_buffer_free(hlib_buffer_t* buffer)
{
    if (NULL != buffer->data) {
        free(buffer->data);
    }
    memset(buffer, 0, sizeof(*buffer));
}

HLIB_C_VISIBILITY_IMPL hlib_buffer_t* hlib_buffer_create(void)
{
    hlib_buffer_t* buffer = (hlib_buffer_t*)malloc(sizeof(hlib_buffer_t));
    if (NULL == buffer) {
        return NULL;
    }

    hlib_buffer_init(buffer);
    return buffer;
}

HLIB_C_VISIBILITY_IMPL void hlib_buffer_destroy(hlib_buffer_t* buffer)
{
    if (NULL == buffer) {
        return;
    }

    hlib_buffer_free(buffer);
    free(buffer);
}

HLIB_C_VISIBILITY_IMPL int hlib_buffer_copy(hlib_buffer_t* buffer, hlib_buffer_t const* that)
{
    hlib_buffer_clear(buffer);
    return hlib_buffer_append(buffer, that->data, that->size);
}

HLIB_C_VISIBILITY_IMPL void hlib_buffer_move(hlib_buffer_t* buffer, hlib_buffer_t* that)
{
    hlib_buffer_free(buffer);
    memcpy(buffer, that, sizeof(*that));
    memset(that, 0, sizeof(*that));
}

HLIB_C_VISIBILITY_IMPL void hlib_buffer_clear(hlib_buffer_t* buffer)
{
    buffer->size = 0;
}

HLIB_C_VISIBILITY_IMPL int hlib_buffer_shrink(hlib_buffer_t* buffer)
{
    assert(buffer->size <= buffer->capacity);

    if (NULL == buffer->data || 0 == buffer->size) {
        hlib_buffer_free(buffer);
        return 0;
    }

    if (NULL == hlib_buffer_realloc(buffer, buffer->size, 1)) {
        return -1;
    }

    return 0;
}

HLIB_C_VISIBILITY_IMPL void* hlib_buffer_reserve(hlib_buffer_t* buffer, size_t capacity)
{
    assert(buffer->size <= buffer->capacity);

    if (NULL == hlib_buffer_realloc(buffer, capacity, 0)) {
        return NULL;
    }

    return buffer->data;
}

HLIB_C_VISIBILITY_IMPL void* hlib_buffer_resize(hlib_buffer_t* buffer, size_t size)
{
    assert(buffer->size <= buffer->capacity);

    if (size > 0 && NULL == hlib_buffer_realloc(buffer, size, 0)) {
        return NULL;
    }

    buffer->size = size;
    return buffer->data;
}

HLIB_C_VISIBILITY_IMPL void* hlib_buffer_extend(hlib_buffer_t* buffer, size_t capacity)
{
    assert(buffer->size <= buffer->capacity);

    if (capacity > 0 && NULL == hlib_buffer_realloc(buffer, buffer->size + capacity, 0)) {
        return NULL;
    }

    return ((char*)buffer->data) + buffer->size;
}

HLIB_C_VISIBILITY_IMPL int hlib_buffer_assign(hlib_buffer_t* buffer, void const* data, size_t size)
{
    buffer->size = 0;
    return hlib_buffer_append(buffer, data, size);
}

HLIB_C_VISIBILITY_IMPL int hlib_buffer_append(hlib_buffer_t* buffer, void const* data, size_t size)
{
    return hlib_buffer_insert(buffer, buffer->size, data, size);
}

HLIB_C_VISIBILITY_IMPL int hlib_buffer_insert(hlib_buffer_t* buffer, size_t offset, void const* data, size_t size)
{
    assert(offset <= buffer->size);
    assert(buffer->size <= buffer->capacity);

    if (0 == size) {
        return 0;
    }

    if (NULL == hlib_buffer_realloc(buffer, buffer->size + size, 0)) {
        return -1;
    }

    uint8_t* ptr = (uint8_t*)buffer->data;

    // Move tail. Note that when inserting at the tail, this is a NOP.
    memmove(ptr + offset + size, ptr + offset, buffer->size - offset);

    // Insert can be used to insert space in the middle without copying data.
    if (NULL != data) {
        memcpy(ptr + offset, data, size);
    }
    buffer->size += size;
    return 0;
}

HLIB_C_VISIBILITY_IMPL void hlib_buffer_erase(hlib_buffer_t* buffer, size_t offset, size_t size)
{
    assert(offset + size <= buffer->size);
    assert(buffer->size <= buffer->capacity);

    uint8_t* ptr = (uint8_t*)buffer->data;

    memmove(ptr + offset, ptr + offset + size, buffer->size - (offset + size));
    buffer->size -= size;
}

HLIB_C_VISIBILITY_IMPL int hlib_buffer_strcpy(hlib_buffer_t* buffer, char const* string)
{
    return hlib_buffer_assign(buffer, string, strlen(string) + 1);
}

HLIB_C_VISIBILITY_IMPL int hlib_buffer_strncpy(hlib_buffer_t* buffer, char const* string, size_t max)
{
    size_t length = strnlen(string, max);

    if (NULL == hlib_buffer_realloc(buffer, length + 1, 0)) {
        return -1;
    }

    char* ptr = (char*)buffer->data;
    memcpy(ptr, string, length);
    ptr[length] = 0;
    return 0;
}

HLIB_C_VISIBILITY_IMPL int hlib_buffer_printf(hlib_buffer_t* buffer, char const* format, ...)
{
    assert(NULL != format);

    va_list ap;
    int r;

    va_start(ap, format);
    hlib_buffer_clear(buffer);
    r = hlib_buffer_vappendf(buffer, format, ap);
    va_end(ap);

    return r;
}

HLIB_C_VISIBILITY_IMPL int hlib_buffer_appendf(hlib_buffer_t* buffer, char const* format, ...)
{
    assert(NULL != format);

    va_list ap;
    int r;

    va_start(ap, format);
    r = hlib_buffer_vappendf(buffer, format, ap);
    va_end(ap);

    return r;
}

#endif // HLIB_C_BUFFER_IMPL

#ifdef __cplusplus
}
#endif

