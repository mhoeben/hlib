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
Result<int> hlib::cpu_get_ticks_per_second() noexcept
{
    int result = sysconf(_SC_CLK_TCK);
    if (-1 == result) {
        return Error(make_system_error(errno));
    }

    return static_cast<int>(result);
}

Result<int> hlib::cpu_get_count() noexcept
{
    int result = sysconf(_SC_NPROCESSORS_CONF);
    if (-1 == result) {
        return Error(make_system_error(errno));
    }

    return static_cast<int>(result);
}

Result<int> hlib::cpu_get_frequency(int cpu) noexcept
{
    assert(cpu < sysconf(_SC_NPROCESSORS_CONF));

    Result<std::string> file = read(format("/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", cpu), true);
    if (true == file.failure()) {
        return file.error();
    }

    std::optional<int> frequency = string_to<int>(file.value());
    assert(std::nullopt != frequency);

    return frequency.value();
}

Result<std::size_t> hlib::cpu_get_cache_size(int cpu, int cache_index) noexcept
{
    assert(cpu < sysconf(_SC_NPROCESSORS_CONF));

    Result<std::string> file = read(format("/sys/devices/system/cpu/cpu%d/cache/index%d/size", cpu, cache_index), true);
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
        return result.value();
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
int CPUMonitor::count() const noexcept
{
    return m_count;
}

CPUMonitor::Statistics const& CPUMonitor::previous(int cpu) const noexcept
{
    assert(cpu < m_count);
    return cpu < 0 ? m_total_previous : m_cpu_previous[cpu];
}

CPUMonitor::Statistics const& CPUMonitor::current(int cpu) const noexcept
{
    assert(cpu < m_count);
    return cpu < 0 ? m_total_current : m_cpu_current[cpu];
}

std::int64_t CPUMonitor::user(int cpu) const noexcept
{
    assert(cpu < m_count);
    return cpu < 0
        ? m_total_current.user - m_total_previous.user
        : m_cpu_current[cpu].user - m_cpu_previous[cpu].user;
}

std::int64_t CPUMonitor::nice(int cpu) const noexcept
{
    assert(cpu < m_count);
    return cpu < 0
        ? m_total_current.nice - m_total_previous.nice
        : m_cpu_current[cpu].nice - m_cpu_previous[cpu].nice;
}

std::int64_t CPUMonitor::system(int cpu) const noexcept
{
    assert(cpu < m_count);
    return cpu < 0
        ? m_total_current.system - m_total_previous.system
        : m_cpu_current[cpu].system - m_cpu_previous[cpu].system;
}

std::int64_t CPUMonitor::idle(int cpu) const noexcept
{
    assert(cpu < m_count);
    return cpu < 0
        ? m_total_current.idle - m_total_previous.idle
        : m_cpu_current[cpu].idle - m_cpu_previous[cpu].idle;
}

std::int64_t CPUMonitor::iowait(int cpu) const noexcept
{
    assert(cpu < m_count);
    return cpu < 0
        ? m_total_current.iowait - m_total_previous.iowait
        : m_cpu_current[cpu].iowait - m_cpu_previous[cpu].iowait;
}

std::int64_t CPUMonitor::irq(int cpu) const noexcept
{
    assert(cpu < m_count);
    return cpu < 0
        ? m_total_current.irq - m_total_previous.irq
        : m_cpu_current[cpu].irq - m_cpu_previous[cpu].irq;
}

std::int64_t CPUMonitor::softirq(int cpu) const noexcept
{
    assert(cpu < m_count);
    return cpu < 0
        ? m_total_current.softirq - m_total_previous.softirq
        : m_cpu_current[cpu].softirq - m_cpu_previous[cpu].softirq;
}

std::int64_t CPUMonitor::steal(int cpu) const noexcept
{
    assert(cpu < m_count);
    return cpu < 0
        ? m_total_current.steal - m_total_previous.steal
        : m_cpu_current[cpu].steal - m_cpu_previous[cpu].steal;
}

std::int64_t CPUMonitor::guestNice(int cpu) const noexcept
{
    assert(cpu < m_count);
    return cpu < 0
        ? m_total_current.guest_nice - m_total_previous.guest_nice
        : m_cpu_current[cpu].guest_nice - m_cpu_previous[cpu].guest_nice;
}

std::int64_t CPUMonitor::total(int cpu) const noexcept
{
    assert(cpu < m_count);
    return cpu < 0
        ? m_total_current.total - m_total_previous.total
        : m_cpu_current[cpu].total - m_cpu_previous[cpu].total;
}

std::int64_t CPUMonitor::busy(int cpu) const noexcept
{
    return total(cpu) - (idle(cpu) + iowait(cpu));
}

Result<> CPUMonitor::initialize() noexcept
{
    Result<int> cpu = cpu_get_count();
    if (true == cpu.failure()) {
        return cpu.error();
    }
    m_count = static_cast<std::int64_t>(cpu.value());

    m_cpu_previous.resize(m_count, {});
    m_cpu_current.resize(m_count, {});

    return update();
}

Result<> CPUMonitor::update() noexcept
{
    Result<std::string> file = read("/proc/stat");
    if (true == file.failure()) {
        return file.error();
    }

    std::vector<std::string> lines = split(file.value(), '\n');
    assert(lines.size() >= static_cast<std::size_t>(m_count + 1));

    m_total_previous = m_total_current;
    m_total_current = parse(lines[0]);

    for (int cpu = 0; cpu < m_count; ++cpu) {
        m_cpu_previous[cpu] = m_cpu_current[cpu];
        m_cpu_current[cpu] = parse(lines[cpu + 1]);
    }

    return {};
}

