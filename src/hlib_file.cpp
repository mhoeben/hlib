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
#include "hlib/string.hpp"
#include <fcntl.h>
#include <fstream>
#include <system_error>
#include <unistd.h>
#include <unordered_map>

using namespace hlib;
using namespace hlib::file;

//
// Public
//
bool file::is_creatable(std::filesystem::path const& filepath) noexcept
{
    try
    {
        // Get the parent directory of the file path.
        std::filesystem::path const parent = filepath.parent_path();

        // Check the permissions of the parent directory.
        std::filesystem::file_status const status = std::filesystem::status(parent);
        std::filesystem::perms const permissions = status.permissions();

        // Check if write permission is granted for the parent directory
        return std::filesystem::perms::none != (std::filesystem::perms::owner_write & permissions);
    }
    catch (...) {
        return false;
    }
}

bool file::is_readable(std::filesystem::path const& filepath) noexcept
{
    std::error_code error_code;

    std::filesystem::file_status const status = std::filesystem::status(filepath, error_code);
    if (error_code) {
        return false;
    }

    std::filesystem::perms const permissions = status.permissions();
    return std::filesystem::perms::none != (std::filesystem::perms::owner_read & permissions)
        || std::filesystem::perms::none != (std::filesystem::perms::group_read & permissions)
        || std::filesystem::perms::none != (std::filesystem::perms::others_read & permissions);
}

bool file::is_writable(std::filesystem::path const& filepath) noexcept
{
    std::error_code error_code;

    std::filesystem::file_status const status = std::filesystem::status(filepath, error_code);
    if (error_code) {
        return false;
    }

    std::filesystem::perms const permissions = status.permissions();
    return std::filesystem::perms::none != (std::filesystem::perms::owner_write & permissions)
        || std::filesystem::perms::none != (std::filesystem::perms::group_write & permissions)
        || std::filesystem::perms::none != (std::filesystem::perms::others_write & permissions);
}

std::istream& file::read(std::istream& stream, Buffer& buffer, std::size_t size, std::error_code& error_code) noexcept
{
    char* ptr = reserve_as<char>(buffer, buffer.size() + size, std::nothrow);
    if (ptr == nullptr) {
        error_code = std::make_error_code(static_cast<std::errc>(errno));
        return stream;
    }

    stream.read(ptr + buffer.size(), size);
    if (true == stream.fail()) {
        error_code = std::make_error_code(static_cast<std::errc>(errno));
        return stream;
    }

    hverify(nullptr != buffer.resize(buffer.size() + stream.gcount(), std::nothrow));
    return stream;
}

std::istream& file::read(std::istream& stream, Buffer& buffer, std::size_t size)
{
    std::error_code error_code;

    read(stream, buffer, size, error_code);
    if (error_code) {
        throw std::system_error(error_code, "read() failed");
    }

    return stream;
}

Buffer file::read(std::istream& stream, std::size_t partial_size, std::error_code& error_code) noexcept
{
    assert(partial_size > 0);
    partial_size += 0 == partial_size;

    Buffer buffer;

    do {
        read(stream, buffer, partial_size, error_code);
    }
    while (false == stream.fail() && false == stream.eof());

    return buffer;
}

Buffer file::read(std::istream& stream, std::size_t partial_size)
{
    std::error_code error_code;

    Buffer buffer = read(stream, partial_size, error_code);
    if (error_code) {
        throw std::system_error(error_code, "read() failed");
    }

    return buffer;
}

ssize_t file::read(FILE* file, Buffer& buffer, std::size_t size, std::error_code& error_code) noexcept
{
    uint8_t* ptr = reserve_as<uint8_t>(buffer, buffer.size() + size, std::nothrow);
    if (ptr == nullptr) {
        error_code = std::make_error_code(static_cast<std::errc>(errno));
        return -1;
    }

    ssize_t count = fread(ptr + buffer.size(), 1, size, file);
    if (count < 0) {
        error_code = std::make_error_code(static_cast<std::errc>(errno));
        return -1;
    }

    hverify(nullptr != buffer.resize(buffer.size() + count, std::nothrow));
    return count;
}

ssize_t file::read(FILE* file, Buffer& buffer, std::size_t size)
{
    std::error_code error_code;

    ssize_t count = read(file, buffer, size, error_code);
    if (error_code) {
        throw std::system_error(error_code, "read() failed");
    }

    return count;
}

Buffer file::read(FILE* file, std::size_t partial_size, std::error_code& error_code) noexcept
{
    assert(partial_size > 0);
    partial_size += 0 == partial_size;

    Buffer buffer;

    do {
        read(file, buffer, partial_size, error_code);
    }
    while (0 == ferror(file) && 0 == feof(file));

    return buffer;
}

Buffer file::read(FILE* file, std::size_t partial_size)
{
    std::error_code error_code;

    Buffer buffer = read(file, partial_size, error_code);
    if (error_code) {
        throw std::system_error(error_code, "read() failed");
    }

    return buffer;
}

ssize_t file::read(int fd, Buffer& buffer, std::size_t size, std::error_code& error_code) noexcept
{
    uint8_t* ptr = reserve_as<uint8_t>(buffer, buffer.size() + size, std::nothrow);
    if (ptr == nullptr) {
        error_code = std::make_error_code(static_cast<std::errc>(errno));
        return -1;
    }

    ssize_t count = ::read(fd, ptr + buffer.size(), size);
    if (count < 0) {
        error_code = std::make_error_code(static_cast<std::errc>(errno));
        return -1;
    }

    hverify(nullptr != buffer.resize(buffer.size() + count, std::nothrow));
    return count;
}

ssize_t file::read(int fd, Buffer& buffer, std::size_t size)
{
    std::error_code error_code;

    ssize_t count = read(fd, buffer, size, error_code);
    if (error_code) {
        throw std::system_error(error_code, "read() failed");
    }

    return count;
}

Buffer file::read(int fd, std::size_t partial_size, std::error_code& error_code) noexcept
{
    assert(partial_size > 0);
    partial_size += 0 == partial_size;

    Buffer buffer;

    do {
        if (0 == read(fd, buffer, partial_size, error_code)) {
            return buffer;
        }

        if (-1 == fcntl(fd, F_GETFL)) {
            error_code = std::make_error_code(static_cast<std::errc>(errno));
            return buffer;
        }
    }
    while (true);
}

Buffer file::read(int fd, std::size_t partial_size)
{
    std::error_code error_code;

    Buffer buffer = read(fd, partial_size, error_code);
    if (error_code) {
        throw std::system_error(error_code, "read() failed");
    }

    return buffer;
}

Buffer file::read(std::string const& filepath, std::error_code& error_code) noexcept
{
    Buffer buffer;

    std::size_t size = std::filesystem::file_size(filepath, error_code);
    if (error_code) {
        return buffer;
    }

    if (0 == size) {
        return buffer;
    }

    std::ifstream stream(filepath);
    read(stream, buffer, size, error_code);
    return buffer;
}

Buffer file::read(std::string const& filepath)
{
    std::error_code error_code;

    Buffer buffer = read(filepath, error_code);
    if (error_code) {
        throw std::system_error(error_code, "read() failed");
    }

    return buffer;
}

std::ostream& file::write(std::ostream& stream, Buffer const& buffer, std::size_t& offset, std::size_t size, std::error_code& error_code) noexcept
{
    if (offset >= buffer.size()) {
        error_code = std::make_error_code(static_cast<std::errc>(EINVAL));
        return stream;
    }

    size = std::min(size, buffer.size() - offset);

    std::streampos pos = stream.tellp();

    stream.write(static_cast<char const*>(buffer.data()) + offset, size);
    if (true == stream.fail()) {
        error_code = std::make_error_code(static_cast<std::errc>(errno));
        return stream;
    }

    offset += stream.tellp() - pos;
    return stream;
}

std::ostream& file::write(std::ostream& stream, Buffer const& buffer, std::size_t& offset, std::size_t size)
{
    std::error_code error_code;

    write(stream, buffer, offset, size, error_code);
    if (error_code) {
        throw std::system_error(error_code, "Failed to write from buffer to stream");
    }

    return stream;
}

void file::write(std::ostream& stream, Buffer const& buffer, std::error_code& error_code) noexcept
{
    std::size_t offset = 0;
    write(stream, buffer, offset, buffer.size(), error_code);
}

void file::write(std::ostream& stream, Buffer const& buffer)
{
    std::size_t offset = 0;
    write(stream, buffer, offset, buffer.size());
}

ssize_t file::write(FILE* file, Buffer const& buffer, std::size_t& offset, std::size_t size, std::error_code& error_code) noexcept
{
    if (offset >= buffer.size()) {
        error_code = std::make_error_code(static_cast<std::errc>(EINVAL));
        return -1;
    }

    size = std::min(size, buffer.size() - offset);

    ssize_t count = fwrite(static_cast<char const*>(buffer.data()) + offset, 1, size, file);
    if (count < 0) {
        error_code = std::make_error_code(static_cast<std::errc>(errno));
        return -1;
    }

    offset += count;
    return count;
}

ssize_t file::write(FILE* file, Buffer const& buffer, std::size_t& offset, std::size_t size)
{
    std::error_code error_code;

    ssize_t count = write(file, buffer, offset, size, error_code);
    if (error_code) {
        throw std::system_error(error_code, "Failed to write from buffer to file");
    }

    return count;
}

void file::write(FILE* file, Buffer const& buffer, std::error_code& error_code) noexcept
{
    std::size_t offset = 0;
    write(file, buffer, offset, buffer.size(), error_code);
}

void file::write(FILE* file, Buffer const& buffer)
{
    std::size_t offset = 0; 
    write(file, buffer, offset, buffer.size());
}

void file::write(std::string const& filepath, Buffer const& buffer, std::error_code& error_code) noexcept
{
    std::ofstream stream(filepath);
    if (!stream) {
        error_code = std::make_error_code(static_cast<std::errc>(errno));
        return;
    }

    write(stream, buffer, error_code);
}

void file::write(std::string const& filepath, Buffer const& buffer)
{
    std::error_code error_code;

    write(filepath, buffer, error_code);
    if (error_code) {
        throw std::system_error(error_code, "Failed to write from buffer to file");
    }
}

std::string file::get_mime_type_from_extension(std::string const& extension, std::string const& default_mime_type)
{
    static std::unordered_map<std::string, std::string> const table =
    {
        { "txt",  "text/plain" },
        { "htm",  "text/html" },
        { "html", "text/html" },
        { "js",   "text/javascript" },
        { "xml",  "text/xml" },
        { "json", "application/json" },
        { "pdf",  "application/pdf" },
        { "png",  "image/png" },
        { "jpg",  "image/jpeg" },
        { "jpeg", "image/jpeg" },
        { "webp", "image/webp" },
        { "gif",  "image/gif" },
        { "bmp",  "image/bmp" }
    };

    auto it = table.find(strip_left(extension, "."));
    return table.end() != it ? it->second : default_mime_type;
}

std::string file::get_mime_type_from_file(std::string const& pathname, std::string const& default_mime_type)
{
    std::filesystem::path path(pathname);

    if (false == std::filesystem::exists(path)) {
        throw std::runtime_error("File not found");
    }

    if (std::filesystem::file_type::regular != std::filesystem::status(path).type()) {
        throw std::runtime_error("File not a regular file");
    }

    std::size_t size = std::filesystem::file_size(path);
    std::array<char, 8> header;
    std::ifstream file(pathname, std::ios::binary);
    file.read(header.data(), 8);

    if (file.gcount() < 8) {
        return get_mime_type_from_extension(path.extension(), default_mime_type);
    }

    auto matches = [&](char const* signature, std::size_t length) {
        return !memcmp(header.data(), signature, length);
    };

    /* Look at data to make an educated guess. */
    if (true == matches("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8)) {
        return "image/png";
    }
    if (true == matches("RIFF", 4) || true == matches("WEBP", 4)) {
        return "image/webp";
    }
    if (true == matches("\xFF\xD8\xFF", 3)) {
        return "image/jpeg";
    }
    if (true == matches("GIF87a", 6) || true == matches("GIF89a", 6)) {
        return "image/gif";
    }
    if (true == matches("BM", 2)) {
        return "image/bmp";
    }
    if (size >= 8 && 0 == memcmp(header.data() + 4, "\x66\x74\x79\x70", 4)) {
        return "video/mp4";
    }

    return get_mime_type_from_extension(path.extension(), default_mime_type);
}

bool file::fd_set_nonblocking(int fd, bool enable) noexcept
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (-1 == flags) {
        return false;
    }

    if (true == enable) {
        flags |= O_NONBLOCK;    
    }
    else {
        flags &= ~O_NONBLOCK;
    }

    if (-1 == fcntl(fd, F_SETFL, flags)) {
        return false;
    }

    return true;
}

void file::fd_close(int fd) noexcept
{
    if (fd < 0) {
        return;
    }

    close(fd);
}

//
// Public (Pipe)
//
Pipe::Pipe() noexcept
    : m_fds { &fd_close, &fd_close }
{
}

Pipe::Pipe(bool open, std::nothrow_t) noexcept
    : Pipe()
{
    if (true == open) {
        Pipe::open(std::nothrow);
    }
}

Pipe::Pipe(bool open)
    : Pipe()
{
    if (true == open) {
        Pipe::open();
    }
}

Pipe::~Pipe()
{
    close();
}

int Pipe::operator[](std::size_t index) const noexcept
{
    assert(index <= 1);
    return m_fds[index].value();
}

bool Pipe::open(std::nothrow_t) noexcept
{
    std::array<int, 2> fds{ -1, -1 };

    close();

    if (-1 == ::pipe(fds.data())) {
        return false;
    }

    m_fds[0].reset(fds[0]);
    m_fds[1].reset(fds[1]);
    return true;
}

void Pipe::open()
{
    if (false == open(std::nothrow)) {
        throwf<std::runtime_error>("pipe() failed ({})", get_error_string());
    }
}

void Pipe::close() noexcept
{
    m_fds[1].reset();
    m_fds[0].reset();
}

