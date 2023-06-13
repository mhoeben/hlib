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

#include "hlib/c/buffer.h"
#include "hlib/c/error.h"

struct hlib_buffer_s;

#ifndef HLIB_C_CODEC_H
#define HLIB_C_CODEC_H

typedef int32_t hlib_codec_type_id_t;
typedef size_t hlib_codec_type_size_t;

typedef struct hlib_codec_type_s
{
    hlib_codec_type_id_t    id_;
    hlib_codec_type_size_t  size_;
} hlib_codec_type_t;

struct hlib_codec_encoder_s;
typedef void(*hlib_codec_encode_type_t)(struct hlib_codec_encoder_s*, hlib_codec_type_t const*);

struct hlib_codec_decoder_s;
typedef void(*hlib_codec_decode_type_t)(struct hlib_codec_decoder_s*, hlib_codec_type_t*);

typedef struct hlib_codec_string_s
{
    char const* data;
    size_t length;
} hlib_codec_string_t;

extern char* hlib_codec_string_copy(char* dest, hlib_codec_string_t const* src, size_t size);

typedef struct hlib_codec_binary_s
{
    void const* data;
    size_t size;
} hlib_codec_binary_t;

typedef struct hlib_codec_encoder_s
{
    hlib_error_t error;

    void (*destroy)(struct hlib_codec_encoder_s* encoder);

    int (*is_binary)(struct hlib_codec_encoder_s* encoder);

    void (*open_type)(struct hlib_codec_encoder_s* encoder, char const* name, hlib_codec_type_t const* value);
    void (*open_array)(struct hlib_codec_encoder_s* encoder, char const* name, size_t value);
    void (*open_map)(struct hlib_codec_encoder_s* encoder, char const* name, size_t value);
    void (*encode_bool)(struct hlib_codec_encoder_s* encoder, char const* name, bool value);
    void (*encode_int32)(struct hlib_codec_encoder_s* encoder, char const* name, int32_t value);
    void (*encode_int64)(struct hlib_codec_encoder_s* encoder, char const* name, int64_t value);
    void (*encode_float)(struct hlib_codec_encoder_s* encoder, char const* name, float value);
    void (*encode_double)(struct hlib_codec_encoder_s* encoder, char const* name, double value);
    void (*encode_string)(struct hlib_codec_encoder_s* encoder, char const* name, hlib_codec_string_t const* value);
    void (*encode_binary)(struct hlib_codec_encoder_s* encoder, char const* name, hlib_codec_binary_t const* value);
    void (*close)(struct hlib_codec_encoder_s* encoder);
} hlib_codec_encoder_t;

HLIB_C_VISIBILITY hlib_codec_encoder_t* hlib_codec_encoder_create(char const* name, struct hlib_buffer_s* buffer);

typedef struct hlib_codec_decoder_s
{
    hlib_error_t error;

    void (*destroy)(struct hlib_codec_decoder_s* decoder);

    void (*reset)(struct hlib_codec_decoder_s* decoder, void const* data, size_t size);

    void (*open_type)(struct hlib_codec_decoder_s* decoder, char const* name, hlib_codec_type_t* value);
    void (*open_array)(struct hlib_codec_decoder_s* decoder, char const* name, size_t* value);
    void (*open_map)(struct hlib_codec_decoder_s* decoder, char const* name, size_t* value);
    void (*decode_bool)(struct hlib_codec_decoder_s* decoder, char const* name, bool* value);
    void (*decode_int32)(struct hlib_codec_decoder_s* decoder, char const* name, int32_t* value);
    void (*decode_int64)(struct hlib_codec_decoder_s* decoder, char const* name, int64_t* value);
    void (*decode_float)(struct hlib_codec_decoder_s* decoder, char const* name, float* value);
    void (*decode_double)(struct hlib_codec_decoder_s* decoder, char const* name, double* value);
    void (*decode_string)(struct hlib_codec_decoder_s* decoder, char const* name, hlib_codec_string_t* value);
    void (*decode_binary)(struct hlib_codec_decoder_s* decoder, char const* name, hlib_codec_binary_t* value);
    void (*close)(struct hlib_codec_decoder_s* decoder);

    int (*peek)(struct hlib_codec_decoder_s* decoder);
} hlib_codec_decoder_t;

HLIB_C_VISIBILITY hlib_codec_decoder_t* hlib_codec_decoder_create(char const* name, void const* data, size_t size);

#endif // HLIB_C_CODEC_H

#ifdef HLIB_C_CODEC_IMPL

#ifndef HLIB_C_CODEC_IMPL_ONCE
#define HLIB_C_CODEC_IMPL_ONCE

#include <string.h>

extern hlib_codec_encoder_t* hlib_codec_encoder_binary_create(struct hlib_buffer_s* buffer);
extern hlib_codec_decoder_t* hlib_codec_decoder_binary_create(void const* data, size_t size);

//
// Public
//
char* hlib_codec_string_copy(char* dest, hlib_codec_string_t const* src, size_t size)
{
    assert(size > 0);

    size_t const n = src->length < size - 1 ? src->length : size - 1;
    memcpy(dest, src->data, n);
    dest[n] = 0;
    return dest;
}

hlib_codec_encoder_t* hlib_codec_encoder_create(char const* name, struct hlib_buffer_s* buffer)
{
    if (0 == strcmp("binary", name)) {
        return hlib_codec_encoder_binary_create(buffer);
    }

    return NULL;
}

hlib_codec_decoder_t* hlib_codec_decoder_create(char const* name, void const* data, size_t size)
{
    if (0 == strcmp("binary", name)) {
        return hlib_codec_decoder_binary_create(data, size);
    }

    return NULL;
}

#endif // HLIB_C_CODEC_IMPL_ONCE
#endif // HLIB_C_CODEC_IMPL

#ifdef __cplusplus
}
#endif

