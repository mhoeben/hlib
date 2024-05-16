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
#include "hlib/event_bus.hpp"
#include "hlib/event_loop.hpp"
#include "hlib/event_queue.hpp"
#include <array>
#include <thread>

using namespace hlib;

TEST_CASE("EventBus", "[events]")
{
    auto event_loop = std::make_shared<EventLoop>();
    auto event_queue_0 = std::make_shared<EventQueue>(event_loop);
    auto event_queue_1 = std::make_shared<EventQueue>(event_loop);
    EventBus event_bus;

    std::thread thread([&event_loop]{
        event_loop->dispatch();
    });

    auto on_exit = [&event_loop](std::any const&)
    { 
        event_loop->interrupt();
    };

    std::string data_0;
    auto on_event_0 = [&data_0](std::any const& data)
    {
        data_0 = std::any_cast<std::string const&>(data);
    };

    int data_1;
    auto on_event_1 = [&data_1](std::any const& data)
    {
        data_1 = std::any_cast<int>(data);
    };

    event_bus.subscribe("0", "test", event_queue_0, on_event_0);
    event_bus.subscribe("1", "test", event_queue_1, on_event_1);
    event_bus.subscribe("0", "exit", event_queue_0, on_exit);

    event_bus.notify("0", "test", std::string("Hello world!"));
    event_bus.notify("1", "test", 42);
    event_bus.broadcast("exit", {});

    thread.join();

    REQUIRE("Hello world!" == data_0);
    REQUIRE(42 == data_1);
}

