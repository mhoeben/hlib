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
    REQUIRE(0 == process.returnCode());
    REQUIRE(nullptr == process.output());
    REQUIRE(nullptr == process.error());
}

TEST_CASE("Subprocess Echo", "[subprocess]")
{
    auto process = std::make_unique<Subprocess>();
    process->run("echo", { "Hello",  "World!" });
    REQUIRE(0 == process->returnCode());
    REQUIRE(nullptr != process->output());
    REQUIRE(nullptr != process->error());
    REQUIRE("Hello World!\n" == to_string(*process->output()));
    REQUIRE(true == process->error()->empty());
    REQUIRE(Subprocess::Exited == process->state());
}

TEST_CASE("Subprocess Stdin to Stdout", "[subprocess]")
{
    auto process = std::make_unique<Subprocess>();
    process->run("cat", { "-" }, "Hello World!");
    REQUIRE(0 == process->returnCode());
    REQUIRE(nullptr != process->output());
    REQUIRE(nullptr != process->error());
    REQUIRE("Hello World!" == to_string(*process->output()));
    REQUIRE(true == process->error()->empty());
    REQUIRE(Subprocess::Exited == process->state());
}

TEST_CASE("Subprocess EventLoop No Data", "[subprocess]")
{
    auto event_loop = std::make_shared<EventLoop>();
    auto process = std::make_unique<Subprocess>(event_loop);

    REQUIRE_NOTHROW(process->run("cat", { "-" }));
    REQUIRE_NOTHROW(process->close());
    REQUIRE_NOTHROW(process->wait());
}

TEST_CASE("Subprocess EventLoop With Data", "[subprocess]")
{
    auto event_loop = std::make_shared<EventLoop>();
    auto process = std::make_unique<Subprocess>(event_loop);

    Buffer result;

    auto on_written = [&](std::shared_ptr<Source> const& /* source */)
    {
        process->close();
    };
    auto on_read = [&](std::shared_ptr<Sink> const& sink)
    {
        result = std::move(std::static_pointer_cast<SinkAdapter<Buffer>>(sink)->get());
        event_loop->interrupt();
    };

    REQUIRE_NOTHROW(process->run("cat", { "-" }));
    REQUIRE_NOTHROW(process->write(make_shared_source_buffer("Hello world!"), on_written));
    REQUIRE_NOTHROW(process->read(make_shared_sink_buffer(), on_read));
    REQUIRE_NOTHROW(event_loop->dispatch());
    REQUIRE_NOTHROW(process->wait());
    REQUIRE("Hello world!" == to_string(result));
}

