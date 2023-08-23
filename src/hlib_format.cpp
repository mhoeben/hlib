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
#include "hlib/format.hpp"
#include <cctype>

using namespace hlib;

void hlib::hexdump(FILE* output, void const* data, std::size_t size, std::size_t columns, bool ascii)
{
    const std::uint8_t* ptr = reinterpret_cast<const uint8_t*>(data);
    std::size_t row = 0;
    std::size_t index;
    std::size_t column;

    fmt::memory_buffer hex_buffer;
    fmt::memory_buffer ascii_buffer;

    auto print_row = [&]
    {
        fmt::print(output, "{:8x} |{:<{}}", row, fmt::to_string(hex_buffer), columns * 3);
        if (true == ascii) {
            fmt::print(output, "| {}\n", fmt::to_string(ascii_buffer));
        }
        else {
            fmt::print(output, "\n");
        }
        row += columns;
    };

    for (index = 0, column = 0; index < size; ++index, ++column) {
        if (column == columns) {
            print_row();

            column = 0;
            hex_buffer.clear();
            ascii_buffer.clear();
        }

        fmt::format_to(std::back_inserter(hex_buffer), " {:02x}", ptr[index]);
        fmt::format_to(std::back_inserter(ascii_buffer), "{:c}", std::isprint(ptr[index]) ? ptr[index] : '.');
    }

    if (0 != column) {
        print_row();
    }
}

void hlib::hexdump(void const* data, size_t size, std::size_t columns, bool ascii)
{
    hexdump(stdout, data, size, columns, ascii);
}

void hlib::hexdump(FILE* output, Buffer const& buffer, std::size_t columns, bool ascii)
{
    hexdump(output, buffer.data(), buffer.size(), columns, ascii);
}

void hlib::hexdump(Buffer const& buffer, std::size_t columns, bool ascii)
{
    hexdump(stdout, buffer, columns, ascii);
}

