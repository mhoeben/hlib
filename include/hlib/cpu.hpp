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
#pragma once

#include "hlib/base.hpp"
#include "hlib/result.hpp"
#include <vector>

namespace hlib
{

Result<int> cpu_get_ticks_per_second() noexcept;
Result<int> cpu_get_count() noexcept;
Result<int> cpu_get_frequency(int cpu) noexcept;
Result<std::size_t> cpu_get_cache_size(int cpu, int cache_index) noexcept;

class CPUMonitor final
{
    HLIB_NOT_COPYABLE(CPUMonitor);
    HLIB_NOT_MOVABLE(CPUMonitor);

public:
    struct Statistics
    {
        std::int64_t user;
        std::int64_t nice;
        std::int64_t system;
        std::int64_t idle;
        std::int64_t iowait;
        std::int64_t irq;
        std::int64_t softirq;
        std::int64_t steal;
        std::int64_t guest_nice;
        std::int64_t total;
    };

public:
    CPUMonitor() = default;

    int count() const noexcept;

    Statistics const& previous(int cpu) const noexcept;
    Statistics const& current(int cpu) const noexcept;

    std::int64_t user(int cpu = -1) const noexcept;
    std::int64_t nice(int cpu = -1) const noexcept;
    std::int64_t system(int cpu = -1) const noexcept;
    std::int64_t idle(int cpu = -1) const noexcept;
    std::int64_t iowait(int cpu = -1) const noexcept;
    std::int64_t irq(int cpu = -1) const noexcept;
    std::int64_t softirq(int cpu = -1) const noexcept;
    std::int64_t steal(int cpu = -1) const noexcept;
    std::int64_t guestNice(int cpu = -1) const noexcept;
    std::int64_t total(int cpu = -1) const noexcept;
    std::int64_t busy(int cpu = -1) const noexcept;

    Result<> initialize() noexcept;
    Result<> update() noexcept;

private:
    int m_count{};

    Statistics m_total_previous{};
    Statistics m_total_current{};

    std::vector<Statistics> m_cpu_previous;
    std::vector<Statistics> m_cpu_current;

    Statistics parse(std::string const& line) const noexcept;
};

} // namespace hlib

