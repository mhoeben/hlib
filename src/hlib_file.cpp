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
#include "hlib/file.hpp"
#include "hlib/error.hpp"
#include "hlib/format.hpp"
#include "hlib/scope_guard.hpp"
#include <sys/stat.h>

using namespace hlib;

Buffer file::read(std::istream& stream, std::size_t chunk_size)
{
    assert(chunk_size > 0);
    chunk_size += 0 == chunk_size;

    Buffer buffer;

    do {
        stream.read(
            static_cast<char*>(buffer.reserve(buffer.size() + chunk_size)),
            chunk_size
        );
        if (true == stream.fail()) {
            throwf<std::runtime_error>("Failed to read ({})", get_error_string(errno));
        }

        buffer.resize(buffer.size() + stream.gcount());
        stream.clear();
    }
    while (false == stream.eof());

    return buffer;
}

Buffer file::read(FILE* file, std::size_t chunk_size)
{
    assert(chunk_size > 0);
    chunk_size += 0 == chunk_size;

    Buffer buffer;

    do {
        std::uint8_t* ptr = static_cast<std::uint8_t*>(buffer.reserve(buffer.size() + chunk_size));
        ssize_t bytes = fread(ptr + buffer.size(), 1, chunk_size, file);
        if (0 == bytes) {
            if (0 != ferror(file)) {
                throwf<std::runtime_error>("Failed to read ({})", get_error_string(errno));
            }
            break;
        }

        buffer.resize(buffer.size() + bytes);
    }
    while (true);

    return buffer;
}

Buffer file::read(std::string const& pathname)
{
    FILE* file = fopen(pathname.c_str(), "r");
    if (nullptr == file) {
        throwf<std::runtime_error>("Failed to open '{}' for reading ({})", pathname, get_error_string(errno));
    }

    ScopeGuard cleanup([&file] {
        assert(nullptr != file);
        fclose(file);
    });

    struct stat st;
    if (-1 == fstat(fileno(file), &st)) {
        // Failed to stat file: progressively read file.
        return file::read(file);
    }

    Buffer buffer;
    ssize_t bytes = fread(buffer.resize(st.st_size), 1, st.st_size, file);
    if (bytes < static_cast<ssize_t>(st.st_size)) {
        throwf<std::runtime_error>("Failed to read ({})", get_error_string(errno));
    }

    return buffer;
}

void file::write(std::ostream& stream, Buffer const& buffer)
{
    if (true == stream.write(static_cast<char const*>(buffer.data()), buffer.size()).fail()) {
        throwf<std::runtime_error>("Failed to write {} bytes ({})", buffer.size(), get_error_string(errno));
    }
}

void file::write(FILE* file, Buffer const& buffer)
{
    if (1 != fwrite(buffer.data(), buffer.size(), 1, file)) {
        throwf<std::runtime_error>("Failed to write {} bytes ({})", buffer.size(), get_error_string(errno));
    }
}

void file::write(std::string const& pathname, Buffer const& buffer, bool append)
{
    FILE* file = fopen(pathname.c_str(), append ? "a":"w");
    if (nullptr == file) {
        throwf<std::runtime_error>("Failed to open '{}' for {} ({})", pathname, append ? "appending":"writing", get_error_string(errno));
    }

    ScopeGuard cleanup([&file] {
        assert(nullptr != file);
        fclose(file);
    });

    file::write(file, buffer);
}

