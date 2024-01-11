//
// MIT License
//
// Copyright (c) 2024 Maarten Hoeben
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

#ifndef HLIB_C_IPC_H
#define HLIB_C_IPC_H

#include "hlib/c/base.h"

typedef enum hlib_ipc_type_e
{
    HLIB_IPC_NAMED_PIPE,
    HLIB_IPC_SHARED_MEMORY
} hlib_ipc_type_t;

typedef struct hlib_ipc_config_s
{
    hlib_ipc_type_t type;
    char const* filepath;
    mode_t mode;
    int flags;
    size_t capacity;
} hlib_ipc_config_t;

typedef struct hlib_ipc_s hlib_ipc_t;

HLIB_C_VISIBILITY hlib_ipc_t* hlib_ipc_create(hlib_ipc_config_t const* config, bool producer);
HLIB_C_VISIBILITY void hlib_ipc_destroy(hlib_ipc_t* ipc);

HLIB_C_VISIBILITY int hlib_ipc_get_fd(hlib_ipc_t* ipc);

HLIB_C_VISIBILITY ssize_t hlib_ipc_produce(hlib_ipc_t* ipc, void const* data, size_t size);
HLIB_C_VISIBILITY ssize_t hlib_ipc_consume(hlib_ipc_t* ipc, void* data, size_t size);

#endif // HLIB_C_IPC_H

#ifdef HLIB_C_IPC_IMPL

#include "circular_buffer.h"
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/mman.h>

typedef struct hlib_ipc_named_pipe_s
{
    bool producer;

    char* filepath;
    int flags;

    int fd;
} hlib_ipc_named_pipe_t;

typedef struct hlib_ipc_shared_memory_data_s
{
    int references;
    hlib_circular_buffer_t buffer;
} hlib_ipc_shared_memory_data_t;

typedef struct hlib_ipc_shared_memory_s
{
    key_t key;
    int id;

    hlib_ipc_shared_memory_data_t* data;
} hlib_ipc_shared_memory_t;

struct hlib_ipc_s
{
    ssize_t (*produce)(hlib_ipc_t* ipc, void const* data, size_t size);
    ssize_t (*consume)(hlib_ipc_t* ipc, void* data, size_t size);
    void (*close)(hlib_ipc_t* ipc);

    hlib_ipc_type_t type;
    union
    {
        hlib_ipc_named_pipe_t named_pipe;
        hlib_ipc_shared_memory_t shared_memory;
    };
};

//
// Implementation
//
static inline int hlib_ipc_atomic_increment(int *value)
{
    return __sync_add_and_fetch(value, 1);
}

// Atomic decrement
static inline int hlib_ipc_atomic_decrement(int *value)
{
    return __sync_sub_and_fetch(value, 1);
}

static int hlib_ipc_fd_change_flags(int fd, int flags, bool set)
{
    int fl = fcntl(fd, F_GETFL, 0);
    if (-1 == fl) {
        return -1;
    }
    return true == set ? fcntl(fd, F_SETFL, (fl | (flags)))
                       : fcntl(fd, F_SETFL, (fl & (~(flags))));
}

int hlib_ipc_named_pipe_open(hlib_ipc_t* ipc)
{
    hlib_ipc_named_pipe_t* self = &ipc->named_pipe;
    int flags = self->producer ? O_WRONLY:O_RDONLY;

    self->fd = open(self->filepath, flags);
    if (-1 == self->fd) {
        return -1;
    }

    return hlib_ipc_fd_change_flags(self->fd, self->flags, true);
}

void hlib_ipc_named_pipe_close(hlib_ipc_t* ipc)
{
    hlib_ipc_named_pipe_t* self = &ipc->named_pipe;

    if (-1 != self->fd) {
        close(self->fd);
    }

    if (NULL != self->filepath) {
        free(self->filepath);
    }
}

ssize_t hlib_ipc_named_pipe_produce(hlib_ipc_t* ipc, void const* data, size_t size)
{
    hlib_ipc_named_pipe_t* self = &ipc->named_pipe;

    if (-1 == self->fd) {
        if (-1 == hlib_ipc_named_pipe_open(ipc)) {
            return -1;
        }
    }

    return write(self->fd, data, size);
}

ssize_t hlib_ipc_named_pipe_consume(hlib_ipc_t* ipc, void* data, size_t size)
{
    hlib_ipc_named_pipe_t* self = &ipc->named_pipe;

    if (-1 == self->fd) {
        if (-1 == hlib_ipc_named_pipe_open(ipc)) {
            return -1;
        }
    }

    return read(self->fd, data, size);
}

static int hlib_ipc_named_pipe_get_fd(hlib_ipc_t* ipc)
{
    hlib_ipc_named_pipe_t* self = &ipc->named_pipe;
    return self->fd;
}

int hlib_ipc_named_pipe_init(hlib_ipc_t* ipc, hlib_ipc_config_t const* config, bool producer)
{
    hlib_ipc_named_pipe_t* self = &ipc->named_pipe;

    self->producer = producer;
    self->flags = config->flags;
    self->fd = -1;

    self->filepath = strdup(config->filepath);
    if (NULL == self->filepath) {
        goto error;
    }

    if (true == producer) {
        if (-1 == mkfifo(config->filepath, config->mode)) {
            goto error;
        }
    }

    ipc->produce = hlib_ipc_named_pipe_produce;
    ipc->consume = hlib_ipc_named_pipe_consume;
    ipc->close = hlib_ipc_named_pipe_close;
    return 0;

error:
    hlib_ipc_named_pipe_close(ipc);

    unlink(config->filepath);
    return -1;
}

ssize_t hlib_ipc_shared_memory_produce(hlib_ipc_t* ipc, void const* data, size_t size)
{
    hlib_ipc_shared_memory_t* self = &ipc->shared_memory;
    return hlib_circular_buffer_produce(&self->data->buffer, data, size, true);
}

ssize_t hlib_ipc_shared_memory_consume(hlib_ipc_t* ipc, void* data, size_t size)
{
    hlib_ipc_shared_memory_t* self = &ipc->shared_memory;
    return hlib_circular_buffer_consume(&self->data->buffer, data, size, true);
}

void hlib_ipc_shared_memory_close(hlib_ipc_t* ipc)
{
    hlib_ipc_shared_memory_t* self = &ipc->shared_memory;

    if (NULL != self->data) {
        int references = hlib_ipc_atomic_decrement(&self->data->references);
        if (0 == references) {
            hlib_circular_buffer_free(&self->data->buffer);
        }

        shmdt(self->data);
    }

    if (-1 != self->id) {
        shmctl(self->id, IPC_RMID, NULL);
    }
}

int hlib_ipc_shared_memory_init(hlib_ipc_t* ipc, hlib_ipc_config_t const* config, bool producer)
{
    hlib_ipc_shared_memory_t* self = &ipc->shared_memory;

    self->id = -1;

    self->key = ftok(config->filepath, 'h');
    if (-1 == self->key) {
        return -1;
    }

    size_t capacity = offsetof(hlib_ipc_shared_memory_data_t, buffer)
                    + offsetof(hlib_circular_buffer_t, data)
                    + config->capacity;

    int flags = producer ? IPC_CREAT|IPC_EXCL : 0;
    flags |= config->mode;

    self->id = shmget(self->key, capacity, flags);
    if (-1 == self->id) {
        goto error;
    }

    self->data = (hlib_ipc_shared_memory_data_t*)shmat(self->id, NULL, 0);
    if (NULL == self->data) {
        goto error;
    }

    if (true == producer) {
        self->data->references = 0;
        hlib_circular_buffer_init(&self->data->buffer, config->capacity, true);
    }

    hlib_ipc_atomic_increment(&self->data->references);

    ipc->produce = hlib_ipc_shared_memory_produce;
    ipc->consume = hlib_ipc_shared_memory_consume;
    ipc->close = hlib_ipc_shared_memory_close;
    return 0;

error:
    hlib_ipc_shared_memory_close(ipc);
    return -1;
}

//
// Public
//
HLIB_C_VISIBILITY_IMPL hlib_ipc_t* hlib_ipc_create(hlib_ipc_config_t const* config, bool producer)
{
    assert(NULL != config);
    (void)producer;

    hlib_ipc_t* ipc = (hlib_ipc_t*)calloc(sizeof(hlib_ipc_t), 1);
    if (NULL == ipc) {
        return NULL;
    }

    ipc->type = config->type;

    switch (ipc->type) {
    case HLIB_IPC_NAMED_PIPE:
        if (-1 == hlib_ipc_named_pipe_init(ipc, config, producer)) {
            goto error;
        }
        break;

    case HLIB_IPC_SHARED_MEMORY:
        if (-1 == hlib_ipc_shared_memory_init(ipc, config, producer)) {
            goto error;
        }
        break;

    default:
        assert(false);
        goto error;
    }

    return ipc;

error:
    free(ipc);
    return NULL;
}

HLIB_C_VISIBILITY_IMPL void hlib_ipc_destroy(hlib_ipc_t* ipc)
{
    if (NULL == ipc) {
        return;
    }

    ipc->close(ipc);
    free(ipc);
}

HLIB_C_VISIBILITY_IMPL int hlib_ipc_get_fd(hlib_ipc_t* ipc)
{
    assert(NULL != ipc);

    switch (ipc->type) {
    case HLIB_IPC_NAMED_PIPE: return hlib_ipc_named_pipe_get_fd(ipc);
    default:
        return -1;
    }
}

HLIB_C_VISIBILITY_IMPL ssize_t hlib_ipc_produce(hlib_ipc_t* ipc, void const* data, size_t size)
{
    assert(NULL != ipc);

    if (NULL == data || 0 == size) {
        return 0;
    }

    return ipc->produce(ipc, data, size);
}

HLIB_C_VISIBILITY_IMPL ssize_t hlib_ipc_consume(hlib_ipc_t* ipc, void* data, size_t size)
{
    assert(NULL != ipc);

    if (NULL == data || 0 == size) {
        return 0;
    }

    return ipc->consume(ipc, data, size);
}

#endif // HLIB_C_IPC_IMPL

#ifdef __cplusplus
}
#endif

