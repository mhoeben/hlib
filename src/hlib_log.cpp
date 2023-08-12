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
#include "hlib/utility.hpp"
#include <array>

using namespace hlib;

//
// Implementation
//
namespace
{

constexpr std::size_t kLevels = static_cast<std::size_t>(log::Trace) + 1;

std::array<std::string, kLevels> const kLevelStrings =
{
    "FATL",
    "ERRO",
    "WARN",
    "NOTI",
    "INFO",
    "DEBG",
    "TRAC"
};

} // namespace

//
// Public
//
log::Domain::Domain(std::string a_name, Level a_level) noexcept
    : name(std::move(a_name))
    , level(a_level)
{
}

log::Domain::Domain(std::string a_name, std::string const& a_env_name) noexcept
    : name(std::move(a_name))
{
    level = static_cast<Level>(get_env<std::int32_t>(
        a_env_name,
        Config::defaultLogLevel()
    ));
}

void log::log(Domain const& domain, Level level, std::string const& message)
{
    fmt::print("{:<12}[{}]: {}\n", domain.name, to_string(level), message);
}

//
// Public (Stringify)
//
std::string const& hlib::to_string(log::Level level) noexcept
{
    assert(level >= log::Fatal && level <= log::Trace);
    return kLevelStrings[level];
}
