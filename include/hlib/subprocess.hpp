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
#include <fcntl.h>
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

    class Stream
    {
        friend class Subprocess;

        HLIB_NOT_COPYABLE(Stream);

    public:
        Stream() noexcept;
        Stream(int fd) noexcept;
        Stream(UniqueOwner<int, -1>&& fd) noexcept;
        Stream(std::string const& filename);
        Stream(std::string const& filename, int flags, mode_t mode = S_IRUSR|S_IWUSR);
        Stream(std::shared_ptr<Buffer> buffer) noexcept;
        Stream(Stream&& that) noexcept;

        Stream& operator =(Stream&& that) noexcept;

    private:
        UniqueOwner<int, -1> m_fd;
        std::shared_ptr<Buffer> m_buffer;
    };

public:
    Subprocess();
    Subprocess(std::weak_ptr<EventLoop> event_loop, EventLoop::Callback on_input,
        EventLoop::Callback on_output, EventLoop::Callback on_error) noexcept;
    Subprocess(std::string const& command, std::vector<std::string> const& args);
    Subprocess(std::string const& command, std::vector<std::string> const& args, Stream input);
    Subprocess(std::string const& command, std::vector<std::string> const& args, std::string const& input);
    Subprocess(std::string const& command, std::vector<std::string> const& args, Stream input, Stream output, Stream error);
    Subprocess(std::string const& command, std::vector<std::string> const& args, std::string const& input, Stream output, Stream error);
    Subprocess(Subprocess&& that) noexcept;

    Subprocess& operator =(Subprocess&& that) noexcept;

    int pid() const noexcept;
    int returnCode() const noexcept;
    Buffer const& output() const noexcept;
    Buffer const& error() const noexcept;

    void setInput(Stream input) noexcept;
    void setOutput(Stream output) noexcept;
    void setError(Stream error) noexcept;

    int run(std::string const& command, std::vector<std::string> const& args);
    int run(std::string const& command, std::vector<std::string> const& args, Stream input);
    int run(std::string const& command, std::vector<std::string> const& args, std::string const& input);
    int run(std::string const& command, std::vector<std::string> const& args, Stream input, Stream output, Stream error);
    int run(std::string const& command, std::vector<std::string> const& args, std::string const& input, Stream output, Stream error);

    int wait();

private:
    std::shared_ptr<EventLoop> m_event_loop_private;
    std::weak_ptr<EventLoop> m_event_loop_extern;

    int m_pid{ -1 };
    int m_return_code{ Pending };

    std::size_t m_input_offset{ 0 };
    Stream m_input;
    Stream m_output;
    Stream m_error;

    std::shared_ptr<Buffer> m_output_buffer;
    std::shared_ptr<Buffer> m_error_buffer;

    EventLoop::Callback m_on_input;
    EventLoop::Callback m_on_output;
    EventLoop::Callback m_on_error;

    void onInput(int fd, std::uint32_t events);
    void onOutput(int fd, std::uint32_t events);
    void onError(int fd, std::uint32_t events);

    int run(std::vector<char const*> argv);
};

} // namespace hlib

