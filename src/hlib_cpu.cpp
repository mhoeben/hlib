//
// MIT License
//
// Copyright (c) 2019 Maarten Hoeben
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
#include "hlib/cpu.hpp"
#include "hlib_format.hpp"
#include "hlib/string.hpp"
#include <fstream>
#include <sstream>
#include <unistd.h>

using namespace hlib;

//
// Implementation
//
namespace
{

Result<std::string> read(std::string const& filepath, bool trim = false) noexcept
{
    try {
        std::ifstream file(filepath);
        std::stringstream buffer;
        buffer << file.rdbuf();

        if (true == trim) {
            return hlib::trim(buffer.str());
        }

        return buffer.str();
    }
    catch (std::exception const& e) {
        return Error(e);
    }
}

} // namespace

//
// Public
//
Result<int> hlib::cpu_get_count() noexcept
{
    int result = sysconf(_SC_NPROCESSORS_CONF);
    if (-1 == result) {
        return Error(make_system_error(errno));
    }

    return static_cast<int>(result);
}

Result<int> hlib::cpu_get_frequency(int nr) noexcept
{
    assert(nr < sysconf(_SC_NPROCESSORS_CONF));

    Result<std::string> file = read(format("/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", nr), true);
    if (true == file.failure()) {
        return file.error();
    }

    std::optional<int> frequency = string_to<int>(file.value());
    assert(std::nullopt != frequency);

    return frequency.value();
}

Result<std::size_t> hlib::cpu_get_cache_size(int nr, int cache_index) noexcept
{
    assert(nr < sysconf(_SC_NPROCESSORS_CONF));

    Result<std::string> file = read(format("/sys/devices/system/cpu/cpu%d/cache/index%d/size", nr, cache_index), true);
    if (true == file.failure()) {
        return file.error();
    }

    std::string string = file.value();
    assert(false == string.empty());

    std::size_t factor;

    switch (string.back()) {
    case 'K': factor = 1024; break;
    case 'M': factor = 1024*1024; break;
    default:
        break;
    }

    string = trim_right(string, "KM");

    std::optional<std::size_t> frequency = string_to<std::size_t>(string);
    assert(std::nullopt != frequency);

    return frequency.value() * factor;
}

//
// Implementation (CPUMonitor)
//
CPUMonitor::Statistics CPUMonitor::parse(std::string const& line) const noexcept
{
    std::vector<std::string> columns = split(line, ' ', true);
    std::int64_t total = 0;

    auto parse_number = [&columns, &total](std::size_t index) -> std::int64_t {
        std::optional<std::int64_t> result = string_to<std::int64_t>(columns[index]);
        assert(std::nullopt != result);
        total += result.value();
        return total;
    };

    Statistics statistics{
        parse_number(1),
        parse_number(2),
        parse_number(3),
        parse_number(4),
        parse_number(5),
        parse_number(6),
        parse_number(7),
        parse_number(8),
        parse_number(9),
        0
    };
    statistics.total = total;

    return statistics;
}

//
// Public (CPUMonitor)
//
int CPUMonitor::nr() const noexcept
{
    return m_nr;
}

CPUMonitor::Statistics const& CPUMonitor::previous(int nr) const noexcept
{
    assert(nr < m_nr);
    return nr < 0 ? m_total_previous : m_cpu_previous[nr];
}

CPUMonitor::Statistics const& CPUMonitor::current(int nr) const noexcept
{
    assert(nr < m_nr);
    return nr < 0 ? m_total_current : m_cpu_current[nr];
}

std::int64_t CPUMonitor::user(int nr) const noexcept
{
    assert(nr < m_nr);
    return nr < 0
        ? m_total_current.user - m_total_previous.user
        : m_cpu_current[nr].user - m_cpu_current[nr].user;
}

std::int64_t CPUMonitor::nice(int nr) const noexcept
{
    assert(nr < m_nr);
    return nr < 0
        ? m_total_current.nice - m_total_previous.nice
        : m_cpu_current[nr].nice - m_cpu_current[nr].nice;
}

std::int64_t CPUMonitor::system(int nr) const noexcept
{
    assert(nr < m_nr);
    return nr < 0
        ? m_total_current.system - m_total_previous.system
        : m_cpu_current[nr].system - m_cpu_current[nr].system;
}

std::int64_t CPUMonitor::idle(int nr) const noexcept
{
    assert(nr < m_nr);
    return nr < 0
        ? m_total_current.idle - m_total_previous.idle
        : m_cpu_current[nr].idle - m_cpu_current[nr].idle;
}

std::int64_t CPUMonitor::iowait(int nr) const noexcept
{
    assert(nr < m_nr);
    return nr < 0
        ? m_total_current.iowait - m_total_previous.iowait
        : m_cpu_current[nr].iowait - m_cpu_current[nr].iowait;
}

std::int64_t CPUMonitor::irq(int nr) const noexcept
{
    assert(nr < m_nr);
    return nr < 0
        ? m_total_current.irq - m_total_previous.irq
        : m_cpu_current[nr].irq - m_cpu_current[nr].irq;
}

std::int64_t CPUMonitor::softirq(int nr) const noexcept
{
    assert(nr < m_nr);
    return nr < 0
        ? m_total_current.softirq - m_total_previous.softirq
        : m_cpu_current[nr].softirq - m_cpu_current[nr].softirq;
}

std::int64_t CPUMonitor::steal(int nr) const noexcept
{
    assert(nr < m_nr);
    return nr < 0
        ? m_total_current.steal - m_total_previous.steal
        : m_cpu_current[nr].steal - m_cpu_current[nr].steal;
}

std::int64_t CPUMonitor::guestNice(int nr) const noexcept
{
    assert(nr < m_nr);
    return nr < 0
        ? m_total_current.guest_nice - m_total_previous.guest_nice
        : m_cpu_current[nr].guest_nice - m_cpu_current[nr].guest_nice;
}

std::int64_t CPUMonitor::total(int nr) const noexcept
{
    assert(nr < m_nr);
    return nr < 0
        ? m_total_current.total - m_total_previous.total
        : m_cpu_current[nr].total - m_cpu_current[nr].total;
}

Result<> CPUMonitor::initialize() noexcept
{
    Result<int> nr = cpu_get_count();
    if (true == nr.failure()) {
        return nr.error();
    }
    m_nr = static_cast<std::int64_t>(nr.value());

    m_cpu_previous.resize(m_nr, {});
    m_cpu_current.resize(m_nr, {});

    return update();
}

Result<> CPUMonitor::update() noexcept
{
    Result<std::string> file = read("/proc/stat");
    if (true == file.failure()) {
        return file.error();
    }

    std::vector<std::string> lines = split(file.value(), '\n');
    assert(lines.size() >= static_cast<std::size_t>(m_nr + 1));

    m_total_previous = m_total_current;
    m_total_current = parse(lines[0]);

    for (int cpu = 0; cpu < m_nr; ++cpu) {
        m_cpu_previous[cpu] = m_cpu_current[cpu];
        m_cpu_current[cpu] = parse(lines[cpu]);
    }

    return {};
}

