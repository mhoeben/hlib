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

#include "hlib/file.hpp"
#include <functional>
#include <condition_variable>
#include <cstdio>
#include <list>
#include <memory>
#include <string>
#include <thread>

namespace hlib
{
namespace log
{

enum Level
{
    Fatal,
    Error,
    Warning,
    Notice,
    Info,
    Debug,
    Trace
};

struct Domain final
{
    std::string const name;
    Level level;

    std::string const env_name;

    Domain(std::string a_name, Level a_level = Level::Notice);
    Domain(std::string a_name, std::string a_env_name);
    ~Domain();
};

class Writer
{
    HLIB_NOT_COPYABLE(Writer);
    HLIB_NOT_MOVABLE(Writer);

public:
    static Writer& get() noexcept;
    static void set(std::shared_ptr<Writer> writer);
    static void set(file::Handle file, bool threaded = false);

public:
    Writer(file::Handle file = file::Handle(), bool threaded = false);
    virtual ~Writer();

    void write(std::string string);
    virtual void write(Domain const& domain, Level level, std::string const& string);

private:
    static std::shared_ptr<Writer> m_writer;

    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::thread m_thread;

    bool m_exit{ false };
    std::list<std::string> m_queue;
    file::Handle m_file;

    void print(std::string const& string);
    void worker();
};

inline void write(Domain const& domain, Level level, std::string const& string)
{
    Writer::get().write(domain, level, string);
}

#define HLOG(a_domain, a_level, ...)  \
    do { if (a_level <= (a_domain).level) { \
        hlib::log::write(a_domain, a_level, fmt::format(__VA_ARGS__)); \
    } } while (false)

#define HLOGF(domain, ...) HLOG(domain, hlib::log::Fatal,   __VA_ARGS__)
#define HLOGE(domain, ...) HLOG(domain, hlib::log::Error,   __VA_ARGS__)
#define HLOGW(domain, ...) HLOG(domain, hlib::log::Warning, __VA_ARGS__)
#define HLOGN(domain, ...) HLOG(domain, hlib::log::Notice,  __VA_ARGS__)
#define HLOGI(domain, ...) HLOG(domain, hlib::log::Info,    __VA_ARGS__)
#define HLOGD(domain, ...) HLOG(domain, hlib::log::Debug,   __VA_ARGS__)
#ifndef NDEBUG
#define HLOGT(domain, ...) HLOG(domain, hlib::log::Trace,   __VA_ARGS__)
#else
#define HLOGT(domain, ...) do { } while (false)
#endif

#define HLOGF_THROW(domain, exception, ...) \
    do { \
        std::string message = fmt::format(__VA_ARGS__); \
        hlib::log::write(domain, hlib::log::Fatal, message); \
        throw exception(fmt::format("{}[FATL]: {}", domain.name, message)); \
    } while (false)

#define HLOGE_THROW(domain, exception, ...) \
    do { \
        std::string message = fmt::format(__VA_ARGS__); \
        hlib::log::write(domain, hlib::log::Error, message); \
        throw exception(fmt::format("{}[ERRO]: {}", domain.name, message)); \
    } while (false)

void set_level_by_name(std::string const& name, log::Level level);
void set_level_by_env_name(std::string const& env_name, log::Level level);

} // namespace log

std::string const& to_string(log::Level level) noexcept;

} // namespace hlib

