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

#include "hlib/c/codec.h"
#include <limits.h>

#ifndef HLIB_C_CODEC_BINARY_H
#define HLIB_C_CODEC_BINARY_H

HLIB_C_VISIBILITY void hlib_codec_encoder_binary_destroy(hlib_codec_encoder_t* encoder);
HLIB_C_VISIBILITY void hlib_codec_encoder_binary_open_type(hlib_codec_encoder_t* encoder, char const* name, hlib_codec_type_t const* value);
HLIB_C_VISIBILITY void hlib_codec_encoder_binary_open_array(hlib_codec_encoder_t* encoder, char const* name, size_t value);
HLIB_C_VISIBILITY void hlib_codec_encoder_binary_open_map(hlib_codec_encoder_t* encoder, char const* name, size_t value);
HLIB_C_VISIBILITY void hlib_codec_encoder_binary_encode_bool(hlib_codec_encoder_t* encoder, char const* name, bool value);
HLIB_C_VISIBILITY void hlib_codec_encoder_binary_encode_int32(hlib_codec_encoder_t* encoder, char const* name, int32_t value);
HLIB_C_VISIBILITY void hlib_codec_encoder_binary_encode_int64(hlib_codec_encoder_t* encoder, char const* name, int64_t value);
HLIB_C_VISIBILITY void hlib_codec_encoder_binary_encode_float(hlib_codec_encoder_t* encoder, char const* name, float value);
HLIB_C_VISIBILITY void hlib_codec_encoder_binary_encode_double(hlib_codec_encoder_t* encoder, char const* name, double value);
HLIB_C_VISIBILITY void hlib_codec_encoder_binary_encode_string(hlib_codec_encoder_t* encoder, char const* name, hlib_codec_string_t const* value);
HLIB_C_VISIBILITY void hlib_codec_encoder_binary_encode_binary(hlib_codec_encoder_t* encoder, char const* name, hlib_codec_binary_t const* value);
HLIB_C_VISIBILITY void hlib_codec_encoder_binary_close(hlib_codec_encoder_t* encoder);

HLIB_C_VISIBILITY void hlib_codec_decoder_binary_destroy(hlib_codec_decoder_t* decoder);
HLIB_C_VISIBILITY void hlib_codec_decoder_binary_reset(hlib_codec_decoder_t* decoder, void const* data, size_t size);
HLIB_C_VISIBILITY void hlib_codec_decoder_binary_open_type(hlib_codec_decoder_t* decoder, char const* name, hlib_codec_type_t* value);
HLIB_C_VISIBILITY void hlib_codec_decoder_binary_open_array(hlib_codec_decoder_t* decoder, char const* name, size_t* value);
HLIB_C_VISIBILITY void hlib_codec_decoder_binary_open_map(hlib_codec_decoder_t* decoder, char const* name, size_t* value);
HLIB_C_VISIBILITY void hlib_codec_decoder_binary_decode_bool(hlib_codec_decoder_t* decoder, char const* name, bool* value);
HLIB_C_VISIBILITY void hlib_codec_decoder_binary_decode_int32(hlib_codec_decoder_t* decoder, char const* name, int32_t* value);
HLIB_C_VISIBILITY void hlib_codec_decoder_binary_decode_int64(hlib_codec_decoder_t* decoder, char const* name, int64_t* value);
HLIB_C_VISIBILITY void hlib_codec_decoder_binary_decode_float(hlib_codec_decoder_t* decoder, char const* name, float* value);
HLIB_C_VISIBILITY void hlib_codec_decoder_binary_decode_double(hlib_codec_decoder_t* decoder, char const* name, double* value);
HLIB_C_VISIBILITY void hlib_codec_decoder_binary_decode_string(hlib_codec_decoder_t* decoder, char const* name, hlib_codec_string_t* value);
HLIB_C_VISIBILITY void hlib_codec_decoder_binary_decode_binary(hlib_codec_decoder_t* decoder, char const* name, hlib_codec_binary_t* value);
HLIB_C_VISIBILITY void hlib_codec_decoder_binary_close(hlib_codec_decoder_t* decoder);
HLIB_C_VISIBILITY int hlib_codec_decoder_binary_more(hlib_codec_decoder_t* decoder);
HLIB_C_VISIBILITY int hlib_codec_decoder_binary_peek(hlib_codec_decoder_t* decoder);

#endif // HLIB_C_CODEC_BINARY_H

#ifdef HLIB_C_CODEC_BINARY_IMPL

#ifndef HLIB_C_CODEC_BINARY_IMPL_ONCE
#define HLIB_C_CODEC_BINARY_IMPL_ONCE

#include "hlib/c/buffer.h"

//
// Implementation
//
typedef struct hlib_codec_encoder_binary_s
{
    hlib_codec_encoder_t base;
    hlib_buffer_t* buffer;
} hlib_codec_encoder_binary_t;

typedef struct hlib_codec_decoder_binary_s
{
    hlib_codec_decoder_t base;
    uint8_t const* data;
    size_t size;
    size_t offset;
} hlib_codec_decoder_binary_t;

#define HLIB_ENCODER_BINARY_ENCODE_INT(type, clz_function, encoder, value)  \
do {                                                                        \
    if (HLIB_ERROR_NONE != encoder->error) {                                \
        return;                                                             \
    }                                                                       \
                                                                            \
    hlib_codec_encoder_binary_t* self = (hlib_codec_encoder_binary_t*)encoder; \
                                                                            \
    uint8_t data[32];                                                       \
    size_t size = 0;                                                        \
                                                                            \
    uint8_t negative;                                                       \
    if (value < 0) {                                                        \
        value = -value;                                                     \
        negative = 0x40;                                                    \
    }                                                                       \
    else {                                                                  \
        negative = 0x00;                                                    \
    }                                                                       \
                                                                            \
    int const bits = (sizeof(type) * 8) - clz_function(value);              \
                                                                            \
    data[size++] = negative | (value & 0x3f);                               \
    if (bits > 6) {                                                         \
        data[size - 1] |= 0x80;                                             \
        value >>= 6;                                                        \
                                                                            \
        for (int i = 6; i < bits; i += 7) {                                 \
            data[size++] = 0x80 | (value & 0x7f);                           \
            value >>= 7;                                                    \
        }                                                                   \
                                                                            \
        data[size - 1] &= ~0x80;                                            \
    }                                                                       \
                                                                            \
    hlib_buffer_append(self->buffer, data, size);                           \
} while(0)

#define HLIB_ENCODER_BINARY_ENCODE_FLOAT(type, encoder, value)              \
do {                                                                        \
    if (HLIB_ERROR_NONE != encoder->error) {                                \
        return;                                                             \
    }                                                                       \
                                                                            \
    hlib_codec_encoder_binary_t* self = (hlib_codec_encoder_binary_t*)encoder; \
                                                                            \
    union                                                                   \
    {                                                                       \
        type f;                                                             \
        uint8_t b[sizeof(type)];                                            \
    } convert;                                                              \
                                                                            \
    convert.f = value;                                                      \
                                                                            \
    size_t size = self->buffer->size;                                       \
    uint8_t* ptr = ((uint8_t*)hlib_buffer_resize(self->buffer, size + sizeof(type))) + size; \
    if (NULL == ptr) {                                                      \
        encoder->error = HLIB_ERROR_BAD_ALLOC;                              \
        return;                                                             \
    }                                                                       \
                                                                            \
    for (size_t i = 0; i < sizeof(type); ++i) {                             \
        ptr[i] = convert.b[sizeof(type) - (i + 1)];                         \
    }                                                                       \
} while(0)


#define HLIB_DECODER_BINARY_DECODE_INT(type, decoder, value)                \
do {                                                                        \
    if (HLIB_ERROR_NONE != decoder->error) {                                \
        return;                                                             \
    }                                                                       \
                                                                            \
    hlib_codec_decoder_binary_t* self = (hlib_codec_decoder_binary_t*)decoder; \
                                                                            \
    uint8_t const* ptr = self->data + self->offset;                         \
    uint8_t const* end = self->data + self->size;                           \
                                                                            \
    if (ptr + 1 > end) {                                                    \
        decoder->error = HLIB_ERROR_PARSING;                                \
        return;                                                             \
    }                                                                       \
    uint8_t byte = *ptr++;                                                  \
    bool negative = !!(byte & 0x40);                                        \
    int shift = 6;                                                          \
                                                                            \
    *value = byte & 0x3f;                                                   \
                                                                            \
    while (byte & 0x80) {                                                   \
        if (ptr + 1 > end) {                                                \
            decoder->error = HLIB_ERROR_PARSING;                            \
            return;                                                         \
        }                                                                   \
        byte = *ptr++;                                                      \
                                                                            \
        *value |= ((type)(byte & 0x7f)) << shift;                           \
        shift += 7;                                                         \
    }                                                                       \
                                                                            \
    *value = negative ? -*value : *value;                                   \
    self->offset += ptr - (self->data + self->offset);                      \
} while(0)


#define HLIB_DECODER_BINARY_DECODE_FLOAT(type, decoder, value)              \
do {                                                                        \
    if (HLIB_ERROR_NONE != decoder->error) {                                \
        return;                                                             \
    }                                                                       \
                                                                            \
    hlib_codec_decoder_binary_t* self = (hlib_codec_decoder_binary_t*)decoder; \
    if (self->offset + sizeof(type) > self->size) {                         \
        decoder->error = HLIB_ERROR_PARSING;                                \
        return;                                                             \
    }                                                                       \
                                                                            \
    union                                                                   \
    {                                                                       \
        type f;                                                             \
        uint8_t b[sizeof(type)];                                            \
    } convert;                                                              \
                                                                            \
    uint8_t const* ptr = self->data + self->offset;                         \
                                                                            \
    for (size_t i = 0; i < sizeof(type); ++i) {                             \
        convert.b[i] = ptr[sizeof(type) - (i + 1)];                         \
    }                                                                       \
                                                                            \
    *value = convert.f;                                                     \
    self->offset += sizeof(type);                                           \
} while(0)

static int hlib_codec_encoder_binary_is_binary(hlib_codec_encoder_t* /* encoder */)
{
    return 1;
}

//
// Public
//
HLIB_C_VISIBILITY_IMPL hlib_codec_encoder_t* hlib_codec_encoder_binary_create(hlib_buffer_t* buffer)
{
    hlib_codec_encoder_binary_t* self = (hlib_codec_encoder_binary_t*)malloc(sizeof(hlib_codec_encoder_binary_t));
    if (NULL == self) {
        return NULL;
    }

    memset(self, 0, sizeof(*self));
    self->base.destroy = hlib_codec_encoder_binary_destroy;
    self->base.is_binary = hlib_codec_encoder_binary_is_binary;
    self->base.open_type = hlib_codec_encoder_binary_open_type;
    self->base.open_array = hlib_codec_encoder_binary_open_array;
    self->base.open_map = hlib_codec_encoder_binary_open_map;
    self->base.encode_bool = hlib_codec_encoder_binary_encode_bool;
    self->base.encode_int32 = hlib_codec_encoder_binary_encode_int32;
    self->base.encode_int64 = hlib_codec_encoder_binary_encode_int64;
    self->base.encode_float = hlib_codec_encoder_binary_encode_float;
    self->base.encode_double = hlib_codec_encoder_binary_encode_double;
    self->base.encode_string = hlib_codec_encoder_binary_encode_string;
    self->base.encode_binary = hlib_codec_encoder_binary_encode_binary;
    self->base.close = hlib_codec_encoder_binary_close;

    self->buffer = buffer;
    return (hlib_codec_encoder_t*)self;
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_encoder_binary_destroy(hlib_codec_encoder_t* encoder)
{
    free(encoder);
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_encoder_binary_open_type(hlib_codec_encoder_t* /* encoder */, char const* /* name */, hlib_codec_type_t const* /* value */)
{
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_encoder_binary_open_array(hlib_codec_encoder_t* encoder, char const* /* name */, size_t value)
{
    hlib_codec_encoder_binary_encode_int64(encoder, NULL, value);
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_encoder_binary_open_map(hlib_codec_encoder_t* encoder, char const* /* name */, size_t value)
{
    hlib_codec_encoder_binary_encode_int64(encoder, NULL, value);
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_encoder_binary_encode_bool(hlib_codec_encoder_t* encoder, char const* /* name */, bool value)
{
    if (HLIB_ERROR_NONE != encoder->error) {
        return;
    }

    hlib_codec_encoder_binary_t* self = (hlib_codec_encoder_binary_t*)encoder;

    bool b = !!value;
    hlib_buffer_append(self->buffer, &b, 1);
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_encoder_binary_encode_int32(hlib_codec_encoder_t* encoder, char const* /* name */, int32_t value)
{
    HLIB_ENCODER_BINARY_ENCODE_INT(int32_t, __builtin_clz, encoder, value);
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_encoder_binary_encode_int64(hlib_codec_encoder_t* encoder, char const* /* name */, int64_t value)
{
    HLIB_ENCODER_BINARY_ENCODE_INT(int64_t, __builtin_clzll, encoder, value);
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_encoder_binary_encode_float(hlib_codec_encoder_t* encoder, char const* /* name */, float value)
{
    HLIB_ENCODER_BINARY_ENCODE_FLOAT(float, encoder, value);
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_encoder_binary_encode_double(hlib_codec_encoder_t* encoder, char const* /* name */, double value)
{
    HLIB_ENCODER_BINARY_ENCODE_FLOAT(double, encoder, value);
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_encoder_binary_encode_string(hlib_codec_encoder_t* encoder, char const* /* name */, hlib_codec_string_t const* value)
{
    hlib_codec_encoder_binary_encode_int64(encoder, NULL, value->length);

    if (HLIB_ERROR_NONE != encoder->error) {
        return;
    }

    hlib_codec_encoder_binary_t* self = (hlib_codec_encoder_binary_t*)encoder;
    hlib_buffer_append(self->buffer, value->data, value->length);
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_encoder_binary_encode_binary(hlib_codec_encoder_t* encoder, char const* /* name */, hlib_codec_binary_t const* value)
{
    hlib_codec_encoder_binary_encode_int64(encoder, NULL, value->size);

    if (HLIB_ERROR_NONE != encoder->error) {
        return;
    }

    hlib_codec_encoder_binary_t* self = (hlib_codec_encoder_binary_t*)encoder;
    hlib_buffer_append(self->buffer, value->data, value->size);
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_encoder_binary_close(hlib_codec_encoder_t* /* encoder */)
{
}

HLIB_C_VISIBILITY_IMPL hlib_codec_decoder_t* hlib_codec_decoder_binary_create(void const* data, size_t size)
{
    hlib_codec_decoder_binary_t* self = (hlib_codec_decoder_binary_t*)malloc(sizeof(hlib_codec_decoder_binary_t));
    if (NULL == self) {
        return NULL;
    }

    memset(self, 0, sizeof(*self));
    self->base.destroy = hlib_codec_decoder_binary_destroy;
    self->base.reset = hlib_codec_decoder_binary_reset;
    self->base.open_type = hlib_codec_decoder_binary_open_type;
    self->base.open_array = hlib_codec_decoder_binary_open_array;
    self->base.open_map = hlib_codec_decoder_binary_open_map;
    self->base.decode_bool = hlib_codec_decoder_binary_decode_bool;
    self->base.decode_int32 = hlib_codec_decoder_binary_decode_int32;
    self->base.decode_int64 = hlib_codec_decoder_binary_decode_int64;
    self->base.decode_float = hlib_codec_decoder_binary_decode_float;
    self->base.decode_double = hlib_codec_decoder_binary_decode_double;
    self->base.decode_string = hlib_codec_decoder_binary_decode_string;
    self->base.decode_binary = hlib_codec_decoder_binary_decode_binary;
    self->base.close = hlib_codec_decoder_binary_close;
    self->base.more = hlib_codec_decoder_binary_more;
    self->base.peek = hlib_codec_decoder_binary_peek;

    self->data = (uint8_t const*)data;
    self->size = size;
    return (hlib_codec_decoder_t*)self;
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_decoder_binary_destroy(hlib_codec_decoder_t* decoder)
{
    free(decoder);
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_decoder_binary_reset(hlib_codec_decoder_t* decoder, void const* data, size_t size)
{
    hlib_codec_decoder_binary_t* self = (hlib_codec_decoder_binary_t*)decoder;
    self->data = (uint8_t const*)data;
    self->size = size;
    self->offset = 0;
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_decoder_binary_open_type(hlib_codec_decoder_t* /* decoder */, char const* /* name */, hlib_codec_type_t* /* value */)
{
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_decoder_binary_open_array(hlib_codec_decoder_t* decoder, char const* /* name */, size_t* value)
{
    if (HLIB_ERROR_NONE != decoder->error) {
        return;
    }

    int64_t size;
    hlib_codec_decoder_binary_decode_int64(decoder, NULL, &size);
    *value = size;
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_decoder_binary_open_map(hlib_codec_decoder_t* decoder, char const* /* name */, size_t* value)
{
    if (HLIB_ERROR_NONE != decoder->error) {
        return;
    }

    int64_t size;
    hlib_codec_decoder_binary_decode_int64(decoder, NULL, &size);
    *value = size;
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_decoder_binary_decode_bool(hlib_codec_decoder_t* decoder, char const* /* name */, bool* value)
{
    if (HLIB_ERROR_NONE != decoder->error) {
        return;
    }

    hlib_codec_decoder_binary_t* self = (hlib_codec_decoder_binary_t*)decoder;
    if (self->offset >= self->size) {
        decoder->error = HLIB_ERROR_PARSING;
        return;
    }

    *value = !!self->data[self->offset];
    ++self->offset;
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_decoder_binary_decode_int32(hlib_codec_decoder_t* decoder, char const* /* name */, int32_t* value)
{
    HLIB_DECODER_BINARY_DECODE_INT(int32_t, decoder, value);
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_decoder_binary_decode_int64(hlib_codec_decoder_t* decoder, char const* /* name */, int64_t* value)
{
    HLIB_DECODER_BINARY_DECODE_INT(int64_t, decoder, value);
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_decoder_binary_decode_float(hlib_codec_decoder_t* decoder, char const* /* name */, float* value)
{
    HLIB_DECODER_BINARY_DECODE_FLOAT(float, decoder, value);
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_decoder_binary_decode_double(hlib_codec_decoder_t* decoder, char const* /* name */, double* value)
{
    HLIB_DECODER_BINARY_DECODE_FLOAT(double, decoder, value);
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_decoder_binary_decode_string(hlib_codec_decoder_t* decoder, char const* /* name */, hlib_codec_string_t* value)
{
    int64_t length = 0;
    hlib_codec_decoder_binary_decode_int64(decoder, NULL, &length);
    if (HLIB_ERROR_NONE != decoder->error) {
        return;
    }

    hlib_codec_decoder_binary_t* self = (hlib_codec_decoder_binary_t*)decoder;
    if (self->offset + length > self->size) {
        decoder->error = HLIB_ERROR_PARSING;
        return;
    }

    value->length = length;
    value->data = ((char const*)self->data) + self->offset;

    self->offset += length;
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_decoder_binary_decode_binary(hlib_codec_decoder_t* decoder, char const* /* name */, hlib_codec_binary_t* value)
{
    int64_t size = 0;
    hlib_codec_decoder_binary_decode_int64(decoder, NULL, &size);
    if (HLIB_ERROR_NONE != decoder->error) {
        return;
    }

    hlib_codec_decoder_binary_t* self = (hlib_codec_decoder_binary_t*)decoder;
    if (self->offset + size > self->size) {
        decoder->error = HLIB_ERROR_PARSING;
        return;
    }

    value->size = size;
    value->data = ((char const*)self->data) + self->offset;

    self->offset += size;
}

HLIB_C_VISIBILITY_IMPL void hlib_codec_decoder_binary_close(hlib_codec_decoder_t* /* decoder */)
{
}

HLIB_C_VISIBILITY_IMPL hlib_codec_type_id_t hlib_codec_decoder_binary_more(hlib_codec_decoder_t* decoder)
{
    hlib_codec_decoder_binary_t* self = (hlib_codec_decoder_binary_t*)decoder;
    return self->offset < self->size;
}

HLIB_C_VISIBILITY_IMPL hlib_codec_type_id_t hlib_codec_decoder_binary_peek(hlib_codec_decoder_t* decoder)
{
    hlib_codec_decoder_binary_t lookahead = *(hlib_codec_decoder_binary_t*)decoder; 

    size_t size;
    hlib_codec_type_id_t id;

    hlib_codec_decoder_binary_open_array((hlib_codec_decoder_t*)&lookahead, NULL, &size);
    hlib_codec_decoder_binary_decode_int32((hlib_codec_decoder_t*)&lookahead, NULL, &id);
    if (HLIB_ERROR_NONE != lookahead.base.error
     || 2 != size) {
        return INT_MIN;
    }

    return id;
}

#endif // HLIB_C_CODEC_BINARY_IMPL_ONCE
#endif // HLIB_C_CODEC_BINARY_IMPL

#ifdef __cplusplus
}
#endif

