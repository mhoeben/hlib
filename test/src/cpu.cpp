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
#include "test.hpp"
#include "hlib/cpu.hpp"
#include <unistd.h>

using namespace hlib;

TEST_CASE("CPU", "[cpu]")
{
    REQUIRE(cpu_get_count().value() > 0);   
    REQUIRE(cpu_get_frequency(0).value() > 0);   
    REQUIRE(cpu_get_cache_size(0, 0).value() > 0);   
}

TEST_CASE("CPUMonitor", "[cpu]")
{
    CPUMonitor cpu_monitor;
    REQUIRE(true == cpu_monitor.initialize().success());
    REQUIRE(cpu_monitor.nr() > 0);
    usleep(100);
    REQUIRE(true == cpu_monitor.update().success());
    REQUIRE(cpu_monitor.total() > 0);
}
