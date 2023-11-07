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
#include "hlib/log.hpp"
#include "hlib/config.hpp"
#include "hlib/format.hpp"
#include "hlib/lock.hpp"
#include "hlib/utility.hpp"
#include <array>
#include <iostream>
#include <mutex>
#include <unordered_set>

using namespace hlib;

//
// Implementation
//
namespace
{

std::mutex mutex;
std::unordered_set<log::Domain*> domains;

constexpr std::size_t levels = static_cast<std::size_t>(log::Trace) + 1;

std::array<std::string, levels> const level_strings =
{
    "FATL",
    "ERRO",
    "WARN",
    "NOTI",
    "INFO",
    "DEBG",
    "TRAC"
};

void print(log::Domain const& domain, log::Level level, std::string const& message)
{
    if (nullptr == log::file) {
        return;
    }
    fmt::print(log::file, "{:<12}[{}]: {}\n", domain.name, to_string(level), message);
}

} // namespace

//
// Public
//
log::Domain::Domain(std::string a_name, Level a_level)
    : name(std::move(a_name))
    , level(a_level)
{
    HLIB_LOCK_GUARD(lock, mutex);
    domains.insert(this);
}

log::Domain::Domain(std::string a_name, std::string const& a_env_name)
    : name(std::move(a_name))
{
    level = static_cast<Level>(get_env<std::int32_t>(
        a_env_name,
        Config::defaultLogLevel()
    ));

    HLIB_LOCK_GUARD(lock, mutex);
    domains.insert(this);
}

log::Domain::~Domain()
{
    HLIB_LOCK_GUARD(lock, mutex);
    domains.erase(this);
}

log::Callback log::callback = std::bind(&print, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
FILE* log::file = stdout;

//
// Public
//
void hlib::log::set_level(std::string const& name, log::Level level) noexcept
{
    HLIB_LOCK_GUARD(lock, mutex);

    for (log::Domain* domain : domains) {
        if (name == domain->name) {
            domain->level = level;
        }
    }
}

std::string const& hlib::to_string(log::Level level) noexcept
{
    assert(level >= log::Fatal && level <= log::Trace);
    return level_strings[level];
}

