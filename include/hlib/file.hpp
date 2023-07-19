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
#include "hlib/config.hpp"
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <string>

namespace hlib
{
namespace file
{

bool is_creatable(std::filesystem::path const& filepath) noexcept;
bool is_readable(std::filesystem::path const& filepath) noexcept;
bool is_writable(std::filesystem::path const& filepath) noexcept;

std::istream& read(std::istream& stream, Buffer& buffer, std::size_t size, std::error_code& error_code) noexcept;
std::istream& read(std::istream& stream, Buffer& buffer, std::size_t size);

Buffer read(std::istream& stream, std::size_t partial_size, std::error_code& error_code) noexcept;
Buffer read(std::istream& stream, std::size_t partial_size);

ssize_t read(FILE* file, Buffer& buffer, std::size_t size, std::error_code& error_code) noexcept;
ssize_t read(FILE* file, Buffer& buffer, std::size_t size);

Buffer read(FILE* file, std::size_t partial_size, std::error_code& error_code) noexcept;
Buffer read(FILE* file, std::size_t partial_size);

Buffer read(std::string const& filepath, std::error_code& error_code) noexcept;
Buffer read(std::string const& filepath);

std::ostream& write(std::ostream& stream, Buffer const& buffer, std::size_t& offset, std::size_t size, std::error_code& error_code) noexcept;
std::ostream& write(std::ostream& stream, Buffer const& buffer, std::size_t& offset, std::size_t size);

void write(std::ostream& stream, Buffer const& buffer, std::error_code& error_code) noexcept;
void write(std::ostream& stream, Buffer const& buffer);

ssize_t write(FILE* file, Buffer const& buffer, std::size_t& offset, std::size_t size, std::error_code& error_code) noexcept;
ssize_t write(FILE* file, Buffer const& buffer, std::size_t& offset, std::size_t size);

void write(FILE* file, Buffer const& buffer, std::error_code& error_code) noexcept;
void write(FILE* file, Buffer const& buffer);

void write(std::string const& filepath, Buffer const& buffer, std::error_code& error_code) noexcept;
void write(std::string const& filepath, Buffer const& buffer);

std::string get_mime_type_from_extension(std::string const& extension, std::string const& default_mime_type);
std::string get_mime_type_from_file(std::string const& pathname, std::string const& default_mime_type);

} // namespace file
} // namespace hlib
