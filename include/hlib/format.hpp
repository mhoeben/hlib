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

#include "hlib/buffer.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wswitch-default"

#include "../../third_party/fmt/include/fmt/format.h"

#pragma GCC diagnostic pop

namespace hlib
{

template<typename T>
class BasicFormatBuffer final
{
public:
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
    
public:
    BasicFormatBuffer(Buffer& buffer)
        : m_buffer(buffer)
    {
    }

    void push_back(T const& value)
    {
        m_buffer.append(&value, sizeof(T));
    }

private:
    Buffer& m_buffer;
};

using FormatBuffer = BasicFormatBuffer<char>;

template<typename MEMORY_BUFFER>
void append_to(MEMORY_BUFFER& buffer, std::string const& string)
{
    buffer.append(string.data(), string.data() + string.length());
}

template<typename MEMORY_BUFFER>
void append_to(MEMORY_BUFFER& buffer, char c)
{
    buffer.append(&c, (&c) + 1);
}

template<typename T>
void append_to(fmt::basic_memory_buffer<T>& buffer, char const* string)
{
    buffer.append(string, string + strlen(string));
}

template<typename T, typename... PARAMS>
void append_to(fmt::basic_memory_buffer<T>& buffer, PARAMS&&... params)
{
    fmt::format_to(std::back_inserter(buffer), std::forward<PARAMS>(params)...);
}

template<typename T, typename... PARAMS>
void append_to(Buffer& buffer, PARAMS&&... params)
{
    BasicFormatBuffer<T> format_buffer(buffer);
    fmt::format_to(std::back_inserter(format_buffer), std::forward<PARAMS>(params)...);
}

template<typename... PARAMS>
void append_to(Buffer& buffer, PARAMS&&... params)
{
    FormatBuffer format_buffer(buffer);
    fmt::format_to(std::back_inserter(format_buffer), std::forward<PARAMS>(params)...);
}

template<typename EXCEPTION, typename... PARAMS>
[[noreturn]] void throwf(char const* fmt, PARAMS&&... params)
{
    throw EXCEPTION(fmt::format(fmt, std::forward<PARAMS>(params)...));
}

} // namespace hlib

