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
#include "hlib/scope_guard.hpp"
#include "hlib/string.hpp"
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <system_error>
#include <unistd.h>
#include <unordered_map>
#include <pwd.h>
#include <unistd.h>

using namespace hlib;

//
// Public
//
std::filesystem::path file::get_home_directory(std::error_code& error_code) noexcept
{
    uid_t uid = getuid();
    struct passwd* pw = getpwuid(uid);
    if (nullptr == pw) {
        error_code = std::make_error_code(static_cast<std::errc>(errno));
        return std::filesystem::path();
    }

    return pw->pw_dir;
}

std::filesystem::path file::get_home_directory()
{
    std::error_code error_code;

    std::filesystem::path path = get_home_directory(error_code);
    if (error_code) {
        throw std::system_error(error_code, "get_home_directory() failed");
    }

    return path;
}

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

//
// void const*
//
Result<std::size_t> file::read(std::istream& stream, void* data, std::size_t size, std::nothrow_t) noexcept
{
    stream.read(static_cast<char*>(data), size);
    if (true == stream.fail()) {
        return make_system_error(errno);
    }

    return stream.gcount();
}

std::istream& file::read(std::istream& stream, void* data, std::size_t& size)
{
    size = success_or_throw<>(read(stream, data, size, std::nothrow));
    return stream;
}

Result<std::size_t> file::read(FILE* file, void* data, std::size_t size, std::nothrow_t) noexcept
{
    std::size_t count = fread(data, 1, size, file);
    if (0 != ferror(file)) {
        return make_system_error(errno);
    }

    return count;
}

std::size_t file::read(FILE* file, void* data, std::size_t size)
{
    return success_or_throw<std::size_t>(read(file, data, size, std::nothrow));
}

Result<size_t> file::read(int fd, void* data, std::size_t size, std::nothrow_t) noexcept
{
    ssize_t count = ::read(fd, data, size);
    if (count < 0) {
        return make_system_error(errno);
    }

    return count;
}

std::size_t file::read(int fd, void* data, std::size_t size)
{
    return success_or_throw<std::size_t>(read(fd, data, size, std::nothrow));
}

//
// Buffer
//
Result<> file::read(std::istream& stream, Buffer& buffer, std::size_t size, std::nothrow_t) noexcept
{
    char* ptr = static_cast<char*>(buffer.extend(size, std::nothrow));
    if (ptr == nullptr) {
        return make_system_error(errno);
    }

    stream.read(ptr + buffer.size(), size);
    if (true == stream.fail()) {
        return make_system_error(errno);
    }

    if (nullptr == buffer.resize(buffer.size() + stream.gcount(), std::nothrow)) {
        return make_system_error(errno);
    }

    return {};
}

std::istream& file::read(std::istream& stream, Buffer& buffer, std::size_t size)
{
    success_or_throw<>(read(stream, buffer, size, std::nothrow));
    return stream;
}

Result<Buffer> file::read(std::istream& stream, std::size_t batch_size, std::nothrow_t) noexcept
{
    assert(batch_size > 0);
    batch_size += 0 == batch_size;

    Result<> result;
    Buffer buffer;

    do {
        result = read(stream, buffer, batch_size, std::nothrow);
        if (true == result.failure()) {
            return result.error();
        }
    }
    while (false == stream.fail() && false == stream.eof());

    return buffer;
}

Buffer file::read(std::istream& stream, std::size_t batch_size)
{
    return success_or_throw<Buffer>(read(stream, batch_size, std::nothrow));
}

Result<std::size_t> file::read(FILE* file, Buffer& buffer, std::size_t size, std::nothrow_t) noexcept
{
    uint8_t* ptr = static_cast<uint8_t*>(buffer.extend(size, std::nothrow));
    if (ptr == nullptr) {
        return make_system_error(errno);
    }

    std::size_t count = fread(ptr + buffer.size(), 1, size, file);
    if (nullptr == buffer.resize(buffer.size() + count, std::nothrow)) {
        return make_system_error(errno);
    }

    if (0 != ferror(file)) {
        return make_system_error(errno);
    }

    return count;
}

std::size_t file::read(FILE* file, Buffer& buffer, std::size_t size)
{
    return success_or_throw<std::size_t>(read(file, buffer, size, std::nothrow));
}

Result<Buffer> file::read(FILE* file, std::size_t batch_size, std::nothrow_t) noexcept
{
    assert(batch_size > 0);
    batch_size += 0 == batch_size;

    Result<size_t> result;
    Buffer buffer;

    do {
        result = read(file, buffer, batch_size, std::nothrow);
        if (true == result.failure()) {
            return result.error();
        }
    }
    while (0 == feof(file));

    return buffer;
}

Buffer file::read(FILE* file, std::size_t batch_size)
{
    return success_or_throw<Buffer>(read(file, batch_size, std::nothrow));
}

Result<size_t> file::read(int fd, Buffer& buffer, std::size_t size, std::nothrow_t) noexcept
{
    uint8_t* ptr = static_cast<uint8_t*>(buffer.extend(size, std::nothrow));
    if (ptr == nullptr) {
        return make_system_error(errno);
    }

    ssize_t count = ::read(fd, ptr + buffer.size(), size);
    if (count < 0) {
        return make_system_error(errno);
    }

    if (nullptr == buffer.resize(buffer.size() + count, std::nothrow)) {
        return make_system_error(errno);
    }

    return count;
}

std::size_t file::read(int fd, Buffer& buffer, std::size_t size)
{
    return success_or_throw<std::size_t>(read(fd, buffer, size, std::nothrow));
}

Result<Buffer> file::read(int fd, std::size_t batch_size, std::nothrow_t) noexcept
{
    assert(batch_size > 0);
    batch_size += 0 == batch_size;

    Result<size_t> result;
    Buffer buffer;

    do {
        result = read(fd, buffer, batch_size, std::nothrow);
        if (true == result.failure()) {
            return result.error();
        }
    }
    while (result.value() > 0);

    return buffer;
}

Buffer file::read(int fd, std::size_t batch_size)
{
    return success_or_throw<Buffer>(read(fd, batch_size, std::nothrow));
}

Result<Buffer> file::read(std::string const& filepath, std::nothrow_t) noexcept
{
    std::error_code error_code;

    std::size_t size = std::filesystem::file_size(filepath, error_code);
    if (error_code) {
        return std::system_error(error_code);
    }

    Buffer buffer;

    if (0 == size) {
        return buffer;
    }

    std::ifstream stream(filepath);

    Result<> result = read(stream, buffer, size, std::nothrow);
    if (true == result.failure()) {
        return result.error();
    }

    return buffer;
}

Buffer file::read(std::string const& filepath)
{
    return success_or_throw<Buffer>(read(filepath, std::nothrow));
}

//
// void const*
//
Result<std::size_t> file::write(std::ostream& stream, void const* data, std::size_t size, std::nothrow_t) noexcept
{
    stream.write(static_cast<char const*>(data), size);
    if (true == stream.fail()) {
        return make_system_error(errno);
    }

    return {};
}

std::ostream& file::write(std::ostream& stream, void const* data, std::size_t& size)
{
    size = success_or_throw(write(stream, data, size, std::nothrow));
    return stream;
}

Result<std::size_t> file::write(FILE* file, void const* data, std::size_t size, std::nothrow_t) noexcept
{
    std::size_t count = fwrite(data, 1, size, file);
    if (0 != ferror(file)) {
        return make_system_error(errno);
    }

    return count;
}

std::size_t file::write(FILE* file, void const* data, std::size_t size)
{
    return success_or_throw<std::size_t>(write(file, data, size, std::nothrow));
}

Result<std::size_t> file::write(int fd, void const* data, std::size_t size, std::nothrow_t) noexcept
{
    ssize_t count = write(fd, data, size);
    if (-1 == count) {
        return make_system_error(errno);
    }

    return count;
}

std::size_t file::write(int fd, void const* data, std::size_t size)
{
    return success_or_throw<std::size_t>(write(fd, data, size, std::nothrow));
}

//
// Buffer
//
Result<> file::write(std::ostream& stream, Buffer const& buffer, std::size_t& offset, std::size_t size, std::nothrow_t) noexcept
{
    size = std::min(size, buffer.size() - offset);

    std::streampos const pos = stream.tellp();

    stream.write(static_cast<char const*>(buffer.data()) + offset, size);
    if (true == stream.fail()) {
        return make_system_error(errno);
    }

    offset += stream.tellp() - pos;
    return {};
}

std::ostream& file::write(std::ostream& stream, Buffer const& buffer, std::size_t& offset, std::size_t size)
{
    success_or_throw(write(stream, buffer, offset, size, std::nothrow));
    return stream;
}

Result<> file::write(std::ostream& stream, Buffer const& buffer, std::nothrow_t) noexcept
{
    std::size_t offset = 0;
    return write(stream, buffer, offset, buffer.size(), std::nothrow);
}

void file::write(std::ostream& stream, Buffer const& buffer)
{
    std::size_t offset = 0;
    write(stream, buffer, offset, buffer.size());
}

Result<std::size_t> file::write(FILE* file, Buffer const& buffer, std::size_t& offset, std::size_t size, std::nothrow_t) noexcept
{
    size = std::min(size, buffer.size() - offset);

    std::size_t count = fwrite(static_cast<char const*>(buffer.data()) + offset, 1, size, file);
    offset += count;

    if (0 != ferror(file)) {
        return make_system_error(errno);
    }

    return count;
}

std::size_t file::write(FILE* file, Buffer const& buffer, std::size_t& offset, std::size_t size)
{
    return success_or_throw<std::size_t>(write(file, buffer, offset, size, std::nothrow));
}

Result<std::size_t> file::write(FILE* file, Buffer const& buffer, std::nothrow_t) noexcept
{
    std::size_t offset = 0;

    return write(file, buffer, offset, buffer.size(), std::nothrow);
}

std::size_t file::write(FILE* file, Buffer const& buffer)
{
    std::size_t offset = 0; 

    return write(file, buffer, offset, buffer.size());
}

Result<std::size_t> write(int fd, Buffer const& buffer, std::size_t& offset, std::size_t size, std::nothrow_t) noexcept
{
    size = std::min(size, buffer.size() - offset);

    ssize_t count = write(fd, static_cast<char const*>(buffer.data()) + offset, size);
    if (-1 == count) {
        return make_system_error(errno);
    }

    offset += count;
    return count;
}

std::size_t write(int fd, Buffer const& buffer, std::size_t& offset, std::size_t size)
{
    return success_or_throw<std::size_t>(write(fd, buffer, offset, size, std::nothrow));
}

Result<std::size_t> write(int fd, Buffer const& buffer, std::nothrow_t) noexcept
{
    std::size_t offset = 0;

    return write(fd, buffer, offset, buffer.size());
}

std::size_t write(int fd, Buffer const& buffer)
{
    std::size_t offset = 0; 

    return write(fd, buffer, offset, buffer.size());
}

Result<> file::write(std::string const& filepath, Buffer const& buffer, std::nothrow_t) noexcept
{
    std::ofstream stream(filepath);
    if (!stream) {
        return make_system_error(errno);
    }

    return write(stream, buffer, std::nothrow);
}

void file::write(std::string const& filepath, Buffer const& buffer)
{
    success_or_throw<>(write(filepath, buffer, std::nothrow));
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

    auto it = table.find(trim_left(extension, "."));
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

Result<> file::fd_set_non_blocking(int fd, bool enable, std::nothrow_t) noexcept
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (-1 == flags) {
        return make_system_error(errno);
    }

    if (true == enable) {
        flags |= O_NONBLOCK;    
    }
    else {
        flags &= ~O_NONBLOCK;
    }

    if (-1 == fcntl(fd, F_SETFL, flags)) {
        return make_system_error(errno);
    }

    return {};
}

void file::fd_set_non_blocking(int fd, bool enable)
{
    success_or_throw(fd_set_non_blocking(fd, enable, std::nothrow));
}

void file::fd_close(int fd) noexcept
{
    if (fd < 0) {
        return;
    }

    close(fd);
}

//
// Public (File)
//
file::File::File() noexcept
    : m_handle(&fclose)
{
}

file::File::File(std::string const& filepath, std::string const& mode, std::error_code& error_code) noexcept
    : File()
{
    Result<> result = open(filepath, mode, std::nothrow);
    if (true == result.failure()) {
        error_code = result.error().code();
    }
}

file::File::File(std::string const& filepath, std::string const& mode)
    : File()
{
    open(filepath, mode);
}

file::File::File(File&& that) noexcept
    : m_handle(std::move(that.m_handle))
{
}

file::File& file::File::operator =(File&& that) noexcept
{
    m_handle = std::move(that.m_handle);
    return *this;
}

file::File::operator FILE*() const noexcept
{
    return m_handle.get();
}

Result<> file::File::open(std::string const& filepath, std::string const& mode, std::nothrow_t) noexcept
{
    close();

    m_handle.reset(fopen(filepath.c_str(), mode.c_str()));
    if (nullptr == m_handle.get()) {
        return make_system_error(errno);
    }

    return {};
}

void file::File::open(std::string const& filepath, std::string const& mode)
{
    std::error_code error_code;

    Result<> result = open(filepath, mode, std::nothrow);
    if (true == result.failure()) {
        throw result.error();
    }
}

void file::File::close() noexcept
{
    if (nullptr == m_handle.get()) {
        return;
    }

    m_handle.reset();
    return;
}

FILE* file::File::release() noexcept
{
    return m_handle.release();
}

//
// Public (Pipe)
//
file::Pipe::Pipe() noexcept
    : m_fds { Handle<int, -1>(&fd_close), Handle<int, -1>(&fd_close) }
{
}

file::Pipe::Pipe(std::error_code& error_code) noexcept
    : Pipe()
{
    Result<> result = Pipe::open(std::nothrow);
    if (true == result.failure()) {
        error_code = result.error().code();
    }
}

file::Pipe::Pipe(bool open)
    : Pipe()
{
    if (false == open) {
        return;
    }
    Pipe::open();
}

file::Pipe::~Pipe()
{
    close();
}

int file::Pipe::operator[](std::size_t index) const noexcept
{
    assert(index <= 1);
    return m_fds[index].get();
}

Result<> file::Pipe::open(std::nothrow_t) noexcept
{
    std::array<int, 2> fds{ -1, -1 };

    close();

    if (-1 == ::pipe(fds.data())) {
        return make_system_error(errno);
    }

    m_fds[0].reset(fds[0]);
    m_fds[1].reset(fds[1]);
    return {};
}

void file::Pipe::open()
{
    success_or_throw(open(std::nothrow));
}

void file::Pipe::close() noexcept
{
    m_fds[1].reset();
    m_fds[0].reset();
}

