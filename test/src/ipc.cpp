//
// MIT License
//
// Copyright (c) 2024 Maarten Hoeben
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
#include "hlib/format.hpp"
#include "hlib/ipc.hpp"
#include "hlib/time.hpp"
#include <limits>
#include <thread>
#include <unistd.h>

using namespace hlib;

namespace
{

constexpr std::size_t samples = 1000;
constexpr char const* filepath = "/tmp/hlib_tests_ipc";

void test(std::string const& type, IPC& producer, IPC& consumer, std::size_t size)
{
    assert(sizeof(std::timespec) <= size);

    std::vector<time::USec> results(samples);

    std::thread thread([&] {
        std::vector<uint8_t> data(size);
        std::timespec* ts = reinterpret_cast<std::timespec*>(data.data());

        for (std::size_t sample = 0; sample < samples; ++sample) {
            REQUIRE(data.size() == throw_or_value<std::size_t>(consumer.consume(data.data(), data.size())));

            results[sample] = (time::now() - time::Clock(*ts)).to<time::USec>();
        }
    });

    std::vector<uint8_t> data(size);
    std::timespec* ts = reinterpret_cast<std::timespec*>(data.data());

    for (std::size_t sample = 0; sample < samples; ++sample) {
        REQUIRE(0 == clock_gettime(CLOCK_MONOTONIC, ts));
        REQUIRE(data.size() == throw_or_value<std::size_t>(producer.produce(data.data(), data.size())));
    }

    thread.join();

    time::USec min(std::numeric_limits<time::USec::Type>::max());
    time::USec max(std::numeric_limits<time::USec::Type>::min());
    time::USec cumulative(0);
    for (std::size_t sample = 0; sample < samples; ++sample) {
        if (results[sample] < min) {
            min = results[sample];
        }
        if (results[sample] > max) {
            max = results[sample];
        }
        cumulative += results[sample];
    }

    fmt::print("---- {} : {} ----\n", type, size);
    fmt::print("min: {}us\n", min.value());
    fmt::print("max: {}us\n", max.value());
    fmt::print("avg: {}us\n", cumulative.value() / samples);
};

} // namespace

TEST_CASE("IPC Named Pipe", "[ipc]")
{
    IPC::Config config(IPC::NamedPipe, filepath);

    unlink(filepath);
    IPC producer(config, true);
    IPC consumer(config, false);

    test("Named Pipe", producer, consumer, 50);
}

TEST_CASE("IPC Shared Memory", "[ipc]")
{
    IPC::Config config(IPC::SharedMemory, 1000000, filepath);

    IPC producer(config, true);
    IPC consumer(config, false);

    test("Shared Memory", producer, consumer, 500000);
}

