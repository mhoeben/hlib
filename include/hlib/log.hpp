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
#include <functional>
#include <cstdio>
#include <string>

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

struct Domain
{
    std::string name;
    Level level;

    Domain(std::string a_name, Level a_level = Level::Notice) noexcept;
    Domain(std::string a_name, std::string const& a_env_name) noexcept;
};

typedef std::function<void(Domain const& domain, Level level, std::string const& message)> Callback;
extern Callback callback;

extern FILE* file;

#define HLOGF(domain, ...) do {                                             hlib::log::callback(domain, hlib::log::Fatal,   fmt::format(__VA_ARGS__));   } while (false)
#define HLOGE(domain, ...) do { if (hlib::log::Error   <= (domain).level) { hlib::log::callback(domain, hlib::log::Error,   fmt::format(__VA_ARGS__)); } } while (false)
#define HLOGW(domain, ...) do { if (hlib::log::Warning <= (domain).level) { hlib::log::callback(domain, hlib::log::Warning, fmt::format(__VA_ARGS__)); } } while (false)
#define HLOGN(domain, ...) do { if (hlib::log::Notice  <= (domain).level) { hlib::log::callback(domain, hlib::log::Notice,  fmt::format(__VA_ARGS__)); } } while (false)
#define HLOGI(domain, ...) do { if (hlib::log::Info    <= (domain).level) { hlib::log::callback(domain, hlib::log::Info,    fmt::format(__VA_ARGS__)); } } while (false)
#ifndef NDEBUG
#define HLOGD(domain, ...) do { if (hlib::log::Debug   <= (domain).level) { hlib::log::callback(domain, hlib::log::Debug,   fmt::format(__VA_ARGS__)); } } while (false)
#define HLOGT(domain, ...) do { if (hlib::log::Trace   <= (domain).level) { hlib::log::callback(domain, hlib::log::Trace,   fmt::format(__VA_ARGS__)); } } while (false)
#else
#define HLOGD(domain, ...) do { } while (false)
#define HLOGT(domain, ...) do { } while (false)
#endif

#define HLOGF_THROW(domain, exception, ...) \
    do { \
        std::string message = fmt::format(__VA_ARGS__); \
        hlib::log::callback(domain, hlib::log::Fatal, message); \
        throw exception(fmt::format("{}[FATL]: {}", domain, message)); \
    } while (false)

#define HLOGE_THROW(domain, exception, ...) \
    do { \
        std::string message = fmt::format(__VA_ARGS__); \
        hlib::log::callback(domain, hlib::log::Error, message); \
        throw exception(fmt::format("{}[ERRO]: {}", domain, message)); \
    } while (false)

} // namespace log

std::string const& to_string(log::Level level) noexcept;

} // namespace hlib

