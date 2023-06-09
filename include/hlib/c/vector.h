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
//
#ifdef __cplusplus
extern "C"
{
#endif

#include "hlib/c/buffer.h"

#ifndef HLIB_C_VECTOR_H
#define HLIB_C_VECTOR_H

typedef struct hlib_vector_s
{
    size_t sizeof_type;
    hlib_buffer_t buffer;
} hlib_vector_t;

#define HLIB_VECTOR_INIT(type) { sizeof(type) }

HLIB_C_VISIBILITY void hlib_vector_init(hlib_vector_t* vector, size_t sizeof_type);
HLIB_C_VISIBILITY void hlib_vector_free(hlib_vector_t* vector);

HLIB_C_VISIBILITY int hlib_vector_copy(hlib_vector_t* vector, hlib_vector_t const* that);
HLIB_C_VISIBILITY void hlib_vector_move(hlib_vector_t* vector, hlib_vector_t* that);

HLIB_C_VISIBILITY size_t hlib_vector_capacity(hlib_vector_t const* vector);
HLIB_C_VISIBILITY size_t hlib_vector_size(hlib_vector_t const* vector);
HLIB_C_VISIBILITY void* hlib_vector_data(hlib_vector_t const* vector);

HLIB_C_VISIBILITY void hlib_vector_clear(hlib_vector_t* vector);
HLIB_C_VISIBILITY void hlib_vector_shrink(hlib_vector_t* vector);
HLIB_C_VISIBILITY void* hlib_vector_reserve(hlib_vector_t* vector, size_t capacity);
HLIB_C_VISIBILITY void* hlib_vector_resize(hlib_vector_t* vector, size_t size);
HLIB_C_VISIBILITY int hlib_vector_assign(hlib_vector_t* vector, void const* values, size_t size);
HLIB_C_VISIBILITY int hlib_vector_append(hlib_vector_t* vector, void const* values, size_t size);
HLIB_C_VISIBILITY int hlib_vector_insert(hlib_vector_t* vector, size_t index, void const* values, size_t size);
HLIB_C_VISIBILITY void hlib_vector_erase(hlib_vector_t* vector, size_t index, size_t size);

#define HLIB_VECTOR_PUSH_BACK(vector, type, value)                          \
    do {                                                                    \
        type v = value;                                                     \
        hlib_vector_push_back(vector, &v);                                  \
    } while (0)

HLIB_C_VISIBILITY int hlib_vector_push_back(hlib_vector_t* vector, void* value);
HLIB_C_VISIBILITY void hlib_vector_pop_back(hlib_vector_t* vector);

#define HLIB_VECTOR_AT(vector, type, index)                                 \
    *((type*)hlib_vector_at(vector, index))

HLIB_C_VISIBILITY void* hlib_vector_at(hlib_vector_t* vector, size_t index);


#endif // HLIB_C_VECTOR_H

#ifdef HLIB_C_VECTOR_IMPL

void hlib_vector_init(hlib_vector_t* vector, size_t sizeof_type)
{
    vector->sizeof_type = sizeof_type;
    hlib_buffer_init(&vector->buffer);
}

void hlib_vector_free(hlib_vector_t* vector)
{
    hlib_buffer_free(&vector->buffer);
}

int hlib_vector_copy(hlib_vector_t* vector, hlib_vector_t const* that)
{
    if (-1 == hlib_buffer_copy(&vector->buffer, &that->buffer)) {
        return -1;
    }
    vector->sizeof_type = that->sizeof_type;
    return 0;
}

void hlib_vector_move(hlib_vector_t* vector, hlib_vector_t* that)
{
    hlib_buffer_move(&vector->buffer, &that->buffer);
    vector->sizeof_type = that->sizeof_type;
    that->sizeof_type = 0;
}

size_t hlib_vector_capacity(hlib_vector_t const* vector)
{
    return vector->buffer.capacity / vector->sizeof_type;
}

size_t hlib_vector_size(hlib_vector_t const* vector)
{
    return vector->buffer.size / vector->sizeof_type;
}

void* hlib_vector_data(hlib_vector_t const* vector)
{
    return vector->buffer.data;
}

void hlib_vector_clear(hlib_vector_t* vector)
{
    hlib_buffer_clear(&vector->buffer);
}

void hlib_vector_shrink(hlib_vector_t* vector)
{
    hlib_buffer_shrink(&vector->buffer);
}

void* hlib_vector_reserve(hlib_vector_t* vector, size_t capacity)
{
    return hlib_buffer_reserve(&vector->buffer, capacity * vector->sizeof_type);
}

void* hlib_vector_resize(hlib_vector_t* vector, size_t size)
{
    return hlib_buffer_resize(&vector->buffer, size * vector->sizeof_type);
}

int hlib_vector_assign(hlib_vector_t* vector, void const* values, size_t size)
{
    return hlib_buffer_assign(&vector->buffer, values, size * vector->sizeof_type);
}

int hlib_vector_append(hlib_vector_t* vector, void const* values, size_t size)
{
    return hlib_buffer_append(&vector->buffer, values, size * vector->sizeof_type);
}

int hlib_vector_insert(hlib_vector_t* vector, size_t index, void const* values, size_t size)
{
    return hlib_buffer_insert(&vector->buffer, index * vector->sizeof_type, values, size * vector->sizeof_type);
}

void hlib_vector_erase(hlib_vector_t* vector, size_t index, size_t size)
{
    hlib_buffer_erase(&vector->buffer, index * vector->sizeof_type, size * vector->sizeof_type);
}

int hlib_vector_push_back(hlib_vector_t* vector, void* value)
{
    return hlib_vector_append(vector, value, 1);
}

void hlib_vector_pop_back(hlib_vector_t* vector)
{
    assert(vector->buffer.size > vector->sizeof_type);
    vector->buffer.size -= vector->sizeof_type;
}

void* hlib_vector_at(hlib_vector_t* vector, size_t index)
{
    if (index * vector->sizeof_type >= vector->buffer.size) {
        return NULL;
    }

    return ((uint8_t*)vector->buffer.data) + index * vector->sizeof_type;
}

#endif // HLIB_C_VECTOR_IMPL

#ifdef __cplusplus
}
#endif
