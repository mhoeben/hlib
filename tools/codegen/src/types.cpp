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
#include "types.hpp"
#include <unordered_map>

Language to_language(std::string const& string)
{
    static std::unordered_map<std::string, Language> const map
    {
        { "cpp11",      Language::CPP11 },
        { "c99",        Language::C99 },
        { "typescript", Language::Typescript }
    };

    auto it = map.find(string);
    return map.end() == it ? Language::Invalid : it->second;
}

Side to_side(std::string const& string)
{
    static std::unordered_map<std::string, Side> const map =
    {
        { "left",   Side::Left },
        { "right",  Side::Right },
        { "both",   Side::Both }
    };

    auto it = map.find(string);
    return map.end() == it ? Side::Invalid : it->second;
}

Flow to_flow(std::string const& string)
{
    static std::unordered_map<std::string, Flow> const map =
    {
        { "left",   Flow::Left },
        { "right",  Flow::Right },
        { "both",   Flow::Both }
    };

    auto it = map.find(string);
    return map.end() == it ? Flow::Invalid : it->second;
}

Type to_type(std::string const& string)
{
    static std::unordered_map<std::string, Type> const map =
    {
        { "bool",       Type::Bool },
        { "int32",      Type::Int32 },
        { "int64",      Type::Int64 },
        { "float32",    Type::Float32 },
        { "float64",    Type::Float64 },
        { "string",     Type::String },
        { "binary",     Type::Binary },
        { "bool[]",     Type::BoolArray },
        { "int32[]",    Type::Int32Array },
        { "int64[]",    Type::Int64Array },
        { "float32[]",  Type::Float32Array },
        { "float64[]",  Type::Float64Array },
        { "string[]",   Type::StringArray },
        { "blob[]",     Type::BinaryArray }
    };

    auto it = map.find(string);
    return map.end() == it ? Type::Invalid : it->second;
}

bool is_primitive(Type type)
{
    return type <= Type::Binary;
}

bool is_pointer(Type type)
{
    return type >= Type::String;
}

bool is_vector(Type type)
{
    return type >= Type::BoolArray;
}

Type to_underlying_type(Type type)
{
    static std::unordered_map<Type, Type> const map =
    {
        { Type::Bool,           Type::Bool },
        { Type::Int32,          Type::Int32 },
        { Type::Int64,          Type::Int64 },
        { Type::Float32,        Type::Float32 },
        { Type::Float64,        Type::Float64 },
        { Type::String,         Type::String },
        { Type::Binary,         Type::Binary },
        { Type::BoolArray,      Type::Bool },
        { Type::Int32Array,     Type::Int32 },
        { Type::Int64Array,     Type::Int64 },
        { Type::Float32Array,   Type::Float32 },
        { Type::Float64Array,   Type::Float64 },
        { Type::StringArray,    Type::String },
        { Type::BinaryArray,    Type::Binary }
    };

    auto it = map.find(type);
    return map.end() == it ? Type::Invalid : it->second;
}

