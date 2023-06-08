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

#include <string>

enum class Language
{
    Invalid,

    CPP11,
    C99,
    Typescript
};

Language to_language(std::string const& string);

enum class Side
{
    Invalid,

    Left,
    Right,
    Both
};

Side to_side(std::string const& string);

enum class Type
{
    Invalid,

    Bool,
    Int32,
    Int64,
    Float32,
    Float64,
    String,
    Blob,

    BoolArray,
    Int32Array,
    Int64Array,
    Float32Array,
    Float64Array,
    StringArray,
    BlobArray
};

Type to_type(std::string const& string);

bool is_primitive(Type type);
bool is_pointer(Type type);
bool is_vector(Type type);
Type to_underlying_type(Type type);
