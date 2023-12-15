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
#include "hlib/event_loop.hpp"
#include "hlib/event_queue.hpp"
#include <array>
#include <thread>

using namespace hlib;

TEST_CASE("EventQueue", "[events]")
{
    auto event_loop = std::make_shared<EventLoop>();
    EventQueue event_queue(event_loop);

    std::thread thread([&event_loop]{
        event_loop->dispatch();
    });

    std::array<int, 4> events = { };
    int count = 0;

    auto set = [&](int event)
    {
        events[count++] = event;

        if (4 == count) {
            event_loop->interrupt();
        }
    };

    event_queue.push(std::bind(set, 0));
    event_queue.push(std::bind(set, 1));
    event_queue.push(std::bind(set, 2));
    event_queue.push(std::bind(set, 3));

    thread.join();

    REQUIRE(0 == events[0]);
    REQUIRE(1 == events[1]);
    REQUIRE(2 == events[2]);
    REQUIRE(3 == events[3]);
}

