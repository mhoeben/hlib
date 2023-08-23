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
#include "hlib/file.hpp"
#include "hlib/subprocess.hpp"
#include <unistd.h>

using namespace hlib;

TEST_CASE("Subprocess", "[subprocess]")
{
    Subprocess process;
    REQUIRE(Subprocess::Idle == process.state());
    REQUIRE(-1 == process.pid());
    REQUIRE(Subprocess::Pending == process.returnCode());
    REQUIRE(true == process.output().empty());
    REQUIRE(true == process.error().empty());
}

TEST_CASE("Subprocess Echo", "[subprocess]")
{
    Subprocess process("echo", { "Hello World!" });
    REQUIRE(0 == process.returnCode());
    REQUIRE("Hello World!\n" == to_string(process.output()));
    REQUIRE(true == process.error().empty());
    REQUIRE(Subprocess::Exited == process.state());

    process.run("echo", { "Good", "morning" });
    REQUIRE(0 == process.returnCode());
    REQUIRE("Good morning\n" == to_string(process.output()));
    REQUIRE(true == process.error().empty());
    REQUIRE(Subprocess::Exited == process.state());
}

TEST_CASE("Subprocess StdIn to StdOut", "[subprocess]")
{
    Subprocess process("cat", { "-" }, "Hello World!");
    REQUIRE(0 == process.returnCode());
    REQUIRE("Hello World!" == to_string(process.output()));
    REQUIRE(true == process.error().empty());
    REQUIRE(Subprocess::Exited == process.state());

    process.run("cat", { "-" }, "Good morning");
    REQUIRE(0 == process.returnCode());
    REQUIRE("Good morning" == to_string(process.output()));
    REQUIRE(true == process.error().empty());
    REQUIRE(Subprocess::Exited == process.state());

}

TEST_CASE("Subprocess Stdin to Null", "[subprocess]")
{
    Subprocess process("cat", { "-" }, "Touching the void", { "/dev/null", O_WRONLY }, {});
    REQUIRE(0 == process.returnCode());
    REQUIRE(true == process.output().empty());
    REQUIRE(true == process.error().empty());
    REQUIRE(Subprocess::Exited == process.state());
}

TEST_CASE("Subprocess EventLoop", "[subprocess]")
{
    auto event_loop = std::make_shared<EventLoop>();

    auto on_output = [&event_loop](int fd, std::uint32_t events)
    {
        if (0 == (EventLoop::Read & events)) {
            return;
        }

        char buffer[20];
        REQUIRE(13 == ::read(fd, buffer, sizeof(buffer)));

        event_loop->interrupt();
    };

    file::Pipe output_pipe;
    output_pipe.open();

    event_loop->add(
        *output_pipe.get<0>(),
        EventLoop::Read,
        std::bind(on_output, std::placeholders::_1, std::placeholders::_2)
    );

    Subprocess process(event_loop);
    REQUIRE(Subprocess::Idle == process.state());

    process.setOutput(std::move(output_pipe.get<1>()));
    process.run("echo", { "Hello World!" });

    REQUIRE(Subprocess::Running == process.state());

    event_loop->dispatch();

    process.wait();

    REQUIRE(Subprocess::Exited == process.state());
}

TEST_CASE("Subprocess EventLoop Buffer", "[subprocess]")
{
    auto event_loop = std::make_shared<EventLoop>();

    Subprocess process(event_loop);
    process.setOutput(Subprocess::Stream(std::make_shared<Buffer>(), std::bind([&event_loop] {
        event_loop->interrupt();
    })));
    process.run("echo", { "Hello World!" });

    event_loop->dispatch();

    process.wait();
}

