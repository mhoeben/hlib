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

#include "hlib/base.hpp"
#include "hlib/buffer.hpp"
#include "hlib/memory.hpp"
#include "hlib/result.hpp"
#include <array>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <string>

namespace hlib
{
namespace file
{

std::filesystem::path get_home_directory(std::error_code& error_code) noexcept;
std::filesystem::path get_home_directory();

bool is_creatable(std::filesystem::path const& filepath) noexcept;
bool is_readable(std::filesystem::path const& filepath) noexcept;
bool is_writable(std::filesystem::path const& filepath) noexcept;

Result<>      read(std::istream& stream, Buffer& buffer, std::size_t size, std::nothrow_t) noexcept;
std::istream& read(std::istream& stream, Buffer& buffer, std::size_t size);

Result<Buffer> read(std::istream& stream, std::size_t batch_size, std::nothrow_t) noexcept;
       Buffer  read(std::istream& stream, std::size_t batch_size);

Result<std::size_t> read(FILE* file, Buffer& buffer, std::size_t size, std::nothrow_t) noexcept;
       std::size_t  read(FILE* file, Buffer& buffer, std::size_t size);

Result<Buffer> read(FILE* file, std::size_t batch_size, std::nothrow_t) noexcept;
       Buffer  read(FILE* file, std::size_t batch_size);

Result<std::size_t> read(int fd, Buffer& buffer, std::size_t size, std::nothrow_t) noexcept;
       std::size_t  read(int fd, Buffer& buffer, std::size_t size);

Result<Buffer> read(int fd, std::size_t batch_size, std::nothrow_t) noexcept;
       Buffer  read(int fd, std::size_t batch_size);

Result<Buffer> read(std::string const& filepath, std::nothrow_t) noexcept;
       Buffer  read(std::string const& filepath);

Result<>      write(std::ostream& stream, Buffer const& buffer, std::size_t& offset, std::size_t size, std::nothrow_t) noexcept;
std::ostream& write(std::ostream& stream, Buffer const& buffer, std::size_t& offset, std::size_t size);

Result<> write(std::ostream& stream, Buffer const& buffer, std::nothrow_t) noexcept;
void     write(std::ostream& stream, Buffer const& buffer);

Result<std::size_t> write(FILE* file, Buffer const& buffer, std::size_t& offset, std::size_t size, std::nothrow_t) noexcept;
       std::size_t  write(FILE* file, Buffer const& buffer, std::size_t& offset, std::size_t size);

Result<std::size_t> write(FILE* file, Buffer const& buffer, std::nothrow_t) noexcept;
       std::size_t  write(FILE* file, Buffer const& buffer);

Result<std::size_t> write(int fd, Buffer const& buffer, std::size_t& offset, std::size_t size, std::nothrow_t) noexcept;
       std::size_t  write(int fd, Buffer const& buffer, std::size_t& offset, std::size_t size);

Result<std::size_t> write(int fd, Buffer const& buffer, std::nothrow_t) noexcept;
       std::size_t  write(int fd, Buffer const& buffer);

Result<> write(std::string const& filepath, Buffer const& buffer, std::nothrow_t) noexcept;
void     write(std::string const& filepath, Buffer const& buffer);

std::string get_mime_type_from_extension(std::string const& extension, std::string const& default_mime_type);
std::string get_mime_type_from_file(std::string const& pathname, std::string const& default_mime_type);

Result<> fd_set_non_blocking(int fd, bool enable, std::nothrow_t) noexcept;
void fd_set_non_blocking(int fd, bool enable);

void fd_close(int fd) noexcept;

class Handle final
{
    HLIB_NOT_COPYABLE(Handle);

public:
    Handle() noexcept;
    Handle(std::string const& filepath, std::string const& mode, std::error_code& error_code) noexcept;
    Handle(std::string const& filepath, std::string const& mode);
    Handle(Handle&& that) noexcept;

    Handle& operator =(Handle&& that) noexcept;

    operator FILE*() const noexcept;

    Result<> open(std::string const& filepath, std::string const& mode, std::nothrow_t) noexcept;
    void open(std::string const& filepath, std::string const& mode);
    void close() noexcept;
    FILE* release() noexcept;

private:
    UniqueHandle<FILE*, nullptr> m_handle;
};

class Pipe final
{
    HLIB_NOT_COPYABLE(Pipe);
    HLIB_NOT_MOVABLE(Pipe);

public:
    Pipe() noexcept;
    Pipe(std::error_code& error_code) noexcept;
    Pipe(bool open);
    ~Pipe();

    int operator[](std::size_t index) const noexcept;

    template<std::size_t index>
    UniqueHandle<int, -1> const& get() const noexcept
    {
        static_assert(index <= 1);
        return m_fds[index];
    }

    template<std::size_t index>
    UniqueHandle<int, -1>& get() noexcept
    {
        static_assert(index <= 1);
        return m_fds[index];
    }

    template<std::size_t index>
    void set(UniqueHandle<int, -1>&& fd) noexcept
    {
        static_assert(index <= -1);
        m_fds[index] = std::move(fd);
    }

    Result<> open(std::nothrow_t) noexcept;
    void open();

    template<std::size_t index>
    void close() noexcept
    {
        m_fds[index].reset();
    }

    void close() noexcept;

private:
    std::array<UniqueHandle<int, -1>, 2> m_fds;
};

} // namespace file
} // namespace hlib

