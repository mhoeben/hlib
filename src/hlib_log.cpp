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
#include <unordered_map>

using namespace hlib;

//
// Implementation
//
namespace
{

std::mutex mutex;
std::unordered_set<log::Domain*> domains;
std::unordered_map<std::string, log::Level> overrides;

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

void register_domain(log::Domain& domain)
{
    HLIB_LOCK_GUARD(lock, mutex);

    domains.insert(&domain);

    auto it = overrides.find(domain.name);
    if (overrides.end() != it) {
        domain.level = it->second;
    }
}

void deregister_domain(log::Domain& domain)
{
    HLIB_LOCK_GUARD(lock, mutex);

    domains.erase(&domain);
}

} // namespace

//
// Public (Domain)
//
log::Domain::Domain(std::string a_name, Level a_level)
    : name(std::move(a_name))
    , level(a_level)
{
    register_domain(*this);
}

log::Domain::Domain(std::string a_name, std::string a_env_name)
    : name(std::move(a_name))
    , env_name(std::move(a_env_name))
{
    level = static_cast<Level>(get_env<std::int32_t>(
        env_name,
        Config::defaultLogLevel()
    ));

    register_domain(*this);
}

log::Domain::~Domain()
{
    deregister_domain(*this);
}

//
// Implementation (Writer)
//
std::shared_ptr<log::Writer> log::Writer::m_writer = std::make_shared<log::Writer>();

void log::Writer::print(std::string const& string)
{
    FILE* file = m_file;

    // Output to stdout when no file was configured.
    if (nullptr == file) {
        file = stdout;
    }

    fwrite(string.data(), string.length(), 1, file);
    fflush(file);
}

void log::Writer::worker()
{
    HLIB_UNIQUE_LOCK(lock, m_mutex);

    do {
        // Wait for condition variable.
        m_condition.wait(lock);

        // Empty queue in one operation.
        std::list<std::string> strings = std::move(m_queue);

        // Unlock while writing queued strings.
        lock.unlock();

        // Print all strings all strings.
        for (auto const& string : strings) {
            print(string);
        }

        // Re-acquire lock for wait.
        lock.lock();
    }
    while (false == m_exit);
}

//
// Public (Writer)
//
log::Writer& log::Writer::get() noexcept
{
    assert(nullptr != m_writer);
    return *m_writer;
}

void log::Writer::set(std::shared_ptr<Writer> writer)
{
    m_writer = std::move(writer);
}

void log::Writer::set(file::Handle file, bool threaded)
{
    set(std::make_shared<Writer>(std::move(file), threaded));
}

log::Writer::Writer(file::Handle file, bool threaded)
    : m_file(std::move(file))
{
    if (false == threaded) {
        return;
    }

    m_thread = std::thread(std::bind(&Writer::worker, this));
}

log::Writer::~Writer()
{
    if (true == m_thread.joinable()) {
        // Set exit flag.
        {
            HLIB_LOCK_GUARD(lock, m_mutex);
            m_exit = true;
        }

        // Notify worker.
        m_condition.notify_one();

        // Wait for thread to stop.
        m_thread.join();
    }
}

void log::Writer::write(std::string string)
{
    // Threaded operation?
    if (true == m_thread.joinable()) {
        HLIB_UNIQUE_LOCK(lock, m_mutex);
        {
            m_queue.emplace_back(std::move(string));
        }
        lock.unlock();

        // Notify worker.
        m_condition.notify_one();
    }
    else {
        // Output string on current thread.
        print(string);
    }
}

void log::Writer::write(Domain const& domain, Level level, std::string const& string)
{
    write(fmt::format("{:<12}[{}]: {}\n", domain.name, to_string(level), string));
}

//
// Public
//
void hlib::log::set_level_by_name(std::string const& name, log::Level level)
{
    HLIB_LOCK_GUARD(lock, mutex);

    // Register the overridden log level.
    overrides[name] = level;

    // Update domains specified by name.
    for (log::Domain* domain : domains) {
        if (name == domain->name) {
            domain->level = level;
        }
    }
}

void hlib::log::set_level_by_env_name(std::string const& env_name, log::Level level)
{
    HLIB_LOCK_GUARD(lock, mutex);

    // Set the environment variable.
    if (-1 == setenv(env_name.c_str(), std::to_string(static_cast<int>(level)).c_str(), 1)) {
        throw std::runtime_error("setenv() failed");
    }

    // Update domains specified by environment name.
    for (log::Domain* domain : domains) {
        if (env_name == domain->env_name) {
            domain->level = level;
        }
    }
}

std::string const& hlib::to_string(log::Level level) noexcept
{
    assert(level >= log::Fatal && level <= log::Trace);
    return level_strings[level];
}

