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

#ifndef HLIB_C_CIRCULAR_BUFFER_H
#define HLIB_C_CIRCULAR_BUFFER_H

#include "hlib/c/base.h"
#include <stdbool.h>
#include <pthread.h>

typedef struct hlib_circular_buffer_s
{
    size_t capacity;

    pthread_mutexattr_t mutexattr;
    pthread_mutex_t mutex;

    pthread_condattr_t condattr;
    pthread_cond_t produced;
    pthread_cond_t consumed;
    size_t size;
    size_t head;
    size_t tail;

    uint8_t data[1];
} hlib_circular_buffer_t;

HLIB_C_VISIBILITY void hlib_circular_buffer_init(hlib_circular_buffer_t* buffer, size_t capacity, bool ipc);
HLIB_C_VISIBILITY void hlib_circular_buffer_free(hlib_circular_buffer_t* buffer);

HLIB_C_VISIBILITY hlib_circular_buffer_t* hlib_circular_buffer_create(size_t capacity);
HLIB_C_VISIBILITY void hlib_circular_buffer_destroy(hlib_circular_buffer_t* buffer);

HLIB_C_VISIBILITY int hlib_circular_buffer_produce(hlib_circular_buffer_t* buffer, void const* data, size_t size, bool wait);
HLIB_C_VISIBILITY int hlib_circular_buffer_consume(hlib_circular_buffer_t* buffer, void* data, size_t size, bool wait);

#endif // HLIB_C_CIRCULAR_BUFFER_H

#ifdef HLIB_C_CIRCULAR_BUFFER_IMPL

#include <errno.h>

HLIB_C_VISIBILITY_IMPL void hlib_circular_buffer_init(hlib_circular_buffer_t* buffer, size_t capacity, bool ipc)
{
    assert(NULL != buffer);
    assert(capacity > 0);

    memset(buffer, 0, sizeof(*buffer));

    int pshared = true == ipc ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE;

    buffer->capacity = capacity;

    pthread_mutexattr_init(&buffer->mutexattr);
    pthread_mutexattr_setpshared(&buffer->mutexattr, pshared);
    pthread_mutex_init(&buffer->mutex, &buffer->mutexattr);

    pthread_condattr_init(&buffer->condattr);
    pthread_condattr_setpshared(&buffer->condattr, pshared);
    pthread_cond_init(&buffer->produced, &buffer->condattr);
    pthread_cond_init(&buffer->consumed, &buffer->condattr);
}

HLIB_C_VISIBILITY_IMPL void hlib_circular_buffer_free(hlib_circular_buffer_t* buffer)
{
    assert(NULL != buffer);

    pthread_cond_destroy(&buffer->consumed);
    pthread_cond_destroy(&buffer->produced);
    pthread_condattr_destroy(&buffer->condattr);

    pthread_mutex_destroy(&buffer->mutex);
    pthread_mutexattr_destroy(&buffer->mutexattr);
}

HLIB_C_VISIBILITY_IMPL hlib_circular_buffer_t* hlib_circular_buffer_create(size_t capacity)
{
    assert(capacity > 0);

    hlib_circular_buffer_t* buffer = (hlib_circular_buffer_t*)malloc(sizeof(hlib_circular_buffer_t) + capacity);
    if (NULL == buffer) {
        return NULL;
    }

    hlib_circular_buffer_init(buffer, capacity, false);
    return buffer;
}

HLIB_C_VISIBILITY_IMPL void hlib_circular_buffer_destroy(hlib_circular_buffer_t* buffer)
{
    if (NULL == buffer) {
        return;
    }

    hlib_circular_buffer_free(buffer);
    free(buffer);
}

HLIB_C_VISIBILITY_IMPL int hlib_circular_buffer_produce(hlib_circular_buffer_t* buffer, void const* data, size_t size, bool wait)
{
    assert(NULL != buffer);
    assert(NULL != data);
    assert(0 != size);
    assert(size <= buffer->capacity);

    if (0 != pthread_mutex_lock(&buffer->mutex)) {
        return -1;
    }

    while (buffer->capacity - buffer->size < size) {
        if (false == wait) {
            pthread_mutex_unlock(&buffer->mutex);
            errno = EAGAIN;
            return -1;
        }

        pthread_cond_wait(&buffer->consumed, &buffer->mutex);
    }

    if (buffer->head >= buffer->tail) {
        assert(buffer->capacity - buffer->head + buffer->tail >= size);

        size_t trailing_capacity = buffer->capacity - buffer->head;

        if (trailing_capacity < size) {
            memcpy(buffer->data + buffer->head, data, trailing_capacity);
            memcpy(buffer->data, (uint8_t const*)data + trailing_capacity, size - trailing_capacity);
        }
        else {
            memcpy(buffer->data + buffer->head, data, size);
        }
    }
    else {
        assert(buffer->tail - buffer->head >= size);

        memcpy(buffer->data + buffer->head, data, size);
    }

    buffer->size += size;
    buffer->head = (buffer->head + size) % buffer->capacity;

    pthread_mutex_unlock(&buffer->mutex);
    pthread_cond_signal(&buffer->produced);
    return 0;
}

HLIB_C_VISIBILITY_IMPL int hlib_circular_buffer_consume(hlib_circular_buffer_t* buffer, void* data, size_t size, bool wait)
{
    assert(NULL != buffer);
    assert(NULL != data);
    assert(size > 0);

    if (0 != pthread_mutex_lock(&buffer->mutex)) {
        return -1;
    }

    while (buffer->size < size) {
        if (false == wait) {
            pthread_mutex_unlock(&buffer->mutex);
            errno = EAGAIN;
            return -1;
        }

        pthread_cond_wait(&buffer->consumed, &buffer->mutex);
    }

    if (buffer->tail >= buffer->head) {
        size_t trailing_capacity = buffer->capacity - buffer->tail;

        if (trailing_capacity < size) {
            memcpy(data, buffer->data + buffer->tail, trailing_capacity);
            memcpy((uint8_t*)data + trailing_capacity, buffer->data, size - trailing_capacity);
        }
        else {
            memcpy(data, buffer->data + buffer->tail, size);
        }
    }
    else {
        assert(buffer->head - buffer->tail >= size);

        memcpy(data, buffer->data + buffer->tail, size);
    }

    buffer->size -= size;
    buffer->tail = (buffer->tail + size) % buffer->capacity;

    pthread_mutex_unlock(&buffer->mutex);
    pthread_cond_signal(&buffer->consumed);
    return 0;
}

#endif // HLIB_C_BUFFER_IMPL

#ifdef __cplusplus
}
#endif

