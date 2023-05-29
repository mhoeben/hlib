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
#include <string>
#include <sstream>

namespace hlib { namespace log
{

enum Level
{
    kFatal,
    kError,
    kWarning,
    kNotice,
    kInfo,
    kDebug,
    kTrace
};

std::string const& to_string(Level level);

struct Domain
{
    std::string name;
    Level level;

    Domain(std::string name, Level level = kNotice);
    Domain(std::string name, std::string const& env_name);
};

void log(Domain const& domain, Level level, std::string const& message);

#define HLOGF(domain, ...) do {                                              hlib::log::log(domain, hlib::log::kFatal,   fmt::format(__VA_ARGS__));   } while (false)
#define HLOGE(domain, ...) do { if (hlib::log::kError   <= (domain).level) { hlib::log::log(domain, hlib::log::kError,   fmt::format(__VA_ARGS__)); } } while (false)
#define HLOGW(domain, ...) do { if (hlib::log::kWarning <= (domain).level) { hlib::log::log(domain, hlib::log::kWarning, fmt::format(__VA_ARGS__)); } } while (false)
#define HLOGN(domain, ...) do { if (hlib::log::kNotice  <= (domain).level) { hlib::log::log(domain, hlib::log::kNotice,  fmt::format(__VA_ARGS__)); } } while (false)
#define HLOGI(domain, ...) do { if (hlib::log::kInfo    <= (domain).level) { hlib::log::log(domain, hlib::log::kInfo,    fmt::format(__VA_ARGS__)); } } while (false)
#ifndef NDEBUG
#define HLOGD(domain, ...) do { if (hlib::log::kDebug   <= (domain).level) { hlib::log::log(domain, hlib::log::kDebug,   fmt::format(__VA_ARGS__)); } } while (false)
#define HLOGT(domain, ...) do { if (hlib::log::kTrace   <= (domain).level) { hlib::log::log(domain, hlib::log::kTrace,   fmt::format(__VA_ARGS__)); } } while (false)
#else
#define HLOGD(domain, ...) do { } while (false)
#define HLOGT(domain, ...) do { } while (false)
#endif

#define HLOGFS(domain, ...) do {                                              std::stringstream s; s << __VA_ARGS__; hlib::log::log(domain, hlib::log::kFatal,   s.str());   } while (false)
#define HLOGES(domain, ...) do { if (hlib::log::kError   <= (domain).level) { std::stringstream s; s << __VA_ARGS__; hlib::log::log(domain, hlib::log::kError,   s.str()); } } while (false)
#define HLOGWS(domain, ...) do { if (hlib::log::kWarning <= (domain).level) { std::stringstream s; s << __VA_ARGS__; hlib::log::log(domain, hlib::log::kWarning, s.str()); } } while (false)
#define HLOGNS(domain, ...) do { if (hlib::log::kNotice  <= (domain).level) { std::stringstream s; s << __VA_ARGS__; hlib::log::log(domain, hlib::log::kNotice,  s.str()); } } while (false)
#define HLOGIS(domain, ...) do { if (hlib::log::kInfo    <= (domain).level) { std::stringstream s; s << __VA_ARGS__; hlib::log::log(domain, hlib::log::kInfo,    s.str()); } } while (false)
#ifndef NDEBUG
#define HLOGDS(domain, ...) do { if (hlib::log::kDebug   <= (domain).level) { std::stringstream s; s << __VA_ARGS__; hlib::log::log(domain, hlib::log::kDebug,   s.str()); } } while (false)
#define HLOGTS(domain, ...) do { if (hlib::log::kTrace   <= (domain).level) { std::stringstream s; s << __VA_ARGS__; hlib::log::log(domain, hlib::log::kTrace,   s.str()); } } while (false)
#else
#define HLOGD(domain, ...) do { } while (false)
#define HLOGT(domain, ...) do { } while (false)
#endif

}} // namespace hlib::log

