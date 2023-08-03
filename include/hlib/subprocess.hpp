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
#pragma once

#include "hlib/buffer.hpp"
#include "hlib/event_loop.hpp"
#include "hlib/memory.hpp"
#include <string>
#include <vector>

namespace hlib
{

class Subprocess final
{
    HLIB_NOT_COPYABLE(Subprocess);

public:
    static constexpr int Pending{ -2 };
    static constexpr int Error{ -1 };
    static constexpr int Ok{ 0 };

    static constexpr std::uint8_t StdOut{ 0x01 };
    static constexpr std::uint8_t StdErr{ 0x02 };

public:
    Subprocess();
    Subprocess(std::string const& command, std::vector<std::string> const& args);
    Subprocess(std::string const& command, std::vector<std::string> const& args, Buffer input);
    Subprocess(std::weak_ptr<EventLoop> event_loop, EventLoop::Callback on_input,
        EventLoop::Callback on_output, EventLoop::Callback on_error) noexcept;
    Subprocess(Subprocess&& that) noexcept;

    Subprocess& operator =(Subprocess&& that) noexcept;

    int pid() const;
    int returnCode() const;
    Buffer const& output() const;
    Buffer const& error() const;

    void setCaptureMask(std::uint8_t mask);

    int run(std::string const& command, std::vector<std::string> const& args);
    int run(std::string const& command, std::vector<std::string> const& args, Buffer input);

    int wait();

private:
    std::shared_ptr<EventLoop> m_event_loop_private;
    std::weak_ptr<EventLoop> m_event_loop_extern;

    static constexpr std::uint8_t StdIn{ 0x04 };
    std::uint8_t m_redirect{ StdOut|StdErr };

    int m_pid{ -1 };
    int m_return_code{ Pending };

    UniqueOwner<int, -1> m_input_fd;
    UniqueOwner<int, -1> m_output_fd;
    UniqueOwner<int, -1> m_error_fd;

    std::size_t m_input_offset{ 0 };
    Buffer m_input;
    Buffer m_output;
    Buffer m_error;

    EventLoop::Callback m_on_input;
    EventLoop::Callback m_on_output;
    EventLoop::Callback m_on_error;

    void onInput(int fd, std::uint32_t events);
    void onOutput(int fd, std::uint32_t events);
    void onError(int fd, std::uint32_t events);

    int run(std::vector<char const*> argv);
};

} // namespace hlib

