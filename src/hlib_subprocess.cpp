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
#include "hlib/subprocess.hpp"
#include "hlib/config.hpp"
#include "hlib/error.hpp"
#include "hlib/file.hpp"
#include "hlib/format.hpp"
#include <array>
#include <cstdio>
#include <sys/wait.h>
#include <unistd.h>

using namespace hlib;

namespace
{

std::vector<char const*> to_argv(std::string const& command, std::vector<std::string> const& args)
{
    std::vector<char const*> argv;
    argv.reserve(args.size() + 2);

    // man execvp: The char *const argv[] argument is an array of pointers to
    // null-terminated strings that represent the argument list available to
    // the new program. The first argument, by convention, should point to the
    // filename associated with the file being executed. The array of pointers
    // must be terminated by a null pointer.
    argv.push_back(command.c_str());
    for (auto const& arg : args) {
        argv.push_back(arg.c_str());
    }
    argv.push_back(nullptr);

    return argv;
}

bool read_to_buffer(int fd, Buffer& buffer)
{
    std::uint8_t* ptr = static_cast<std::uint8_t*>(buffer.reserve(buffer.size() + Config::subprocessOutputBatchSize()));

    ssize_t size = ::read(fd, ptr, Config::subprocessOutputBatchSize());
    if (-1 == size) {
        return false;
    }

    buffer.resize(buffer.size() + size);
    return size > 0;
}

} // namespace

//
// Implementation
//
void Subprocess::onInput(int fd, std::uint32_t events)
{
    if (0 == (EventLoop::Write & events)) {
        m_event_loop_private->interrupt();
        return;
    }

    assert(false == m_input.empty());
    assert(m_input_offset < m_input.size());

    ssize_t size = ::write(fd,
        static_cast<std::uint8_t const*>(m_input.data()) + m_input_offset,
        m_input.size() - m_input_offset
    );
    if (-1 == size) {
        m_event_loop_private->interrupt();
        return;
    }

    m_input_offset += size;
    if (m_input.size() == m_input_offset) {
        m_input_offset = 0;
        m_input.clear();
        m_event_loop_private->remove(fd);
        m_input_fd.reset();
    }
}

void Subprocess::onOutput(int fd, std::uint32_t events)
{
    if (0 == (EventLoop::Read & events)
     || false == read_to_buffer(fd, m_output)) {
        m_event_loop_private->interrupt();
    }
}

void Subprocess::onError(int fd, std::uint32_t events)
{
    if (0 == (EventLoop::Read & events)
     || false == read_to_buffer(fd, m_error)) {
        m_event_loop_private->interrupt();
    }
}

int Subprocess::run(std::vector<char const*> argv)
{
    assert(-1 == m_pid);

    m_return_code = Pending;

    m_output.clear();
    m_error.clear();

    std::shared_ptr<EventLoop> event_loop = m_event_loop_private
        ? m_event_loop_private
        : m_event_loop_extern.lock();
    if (nullptr == event_loop) {
        throwf<std::logic_error>("External event loop not available");
    }


    file::Pipe input_pipe;
    switch (m_input_fd.value()) {
    case Buffered:  input_pipe.open(); break;
    case StdIn:     break;
    default:        input_pipe.set<0>(std::move(m_input_fd)); break;
    }

    file::Pipe output_pipe;
    switch (m_output_fd.value()) {
    case Buffered:  output_pipe.open(); break;
    case StdOut:    break;
    default:        output_pipe.set<1>(std::move(m_output_fd)); break;
    }

    file::Pipe error_pipe;
    switch (m_error_fd.value()) {
    case Buffered:  error_pipe.open(); break;
    case StdErr:    break;
    default:        error_pipe.set<1>(std::move(m_error_fd)); break;
    }

    bool redirect = false;

    switch (m_pid = fork()) {
    case -1:
        throwf<std::runtime_error>("Failed to fork for '{}' ({})", argv[0], get_error_string(errno));

    case 0:
        event_loop.reset();

        if (input_pipe.get<0>().value() >= 0) {
            hverify(-1 != dup2(input_pipe.get<0>().value(), STDIN_FILENO));
        }
        if (output_pipe.get<1>().value() >= 0) {
            hverify(-1 != dup2(output_pipe.get<1>().value(), STDOUT_FILENO));
        }
        if (error_pipe.get<1>().value() >= 0) {
            hverify(-1 != dup2(error_pipe.get<1>().value(), STDERR_FILENO));
        }

        input_pipe.close();
        output_pipe.close();
        error_pipe.close();

        hverify(-1 == execvp(argv[0], const_cast<char * const *>(argv.data())));

        fmt::print(stderr, "execvp failed for '{}' ({})", argv[0], get_error_string(errno));
        abort();
        return Error;

    default:
        if (Buffered == m_input_fd.value()) {
            m_input_fd = std::move(input_pipe.get<1>());
            event_loop->add(m_input_fd.value(), EventLoop::Write, m_on_input);

            redirect = true;
        }
        else {
            m_input_fd.reset();
        }

        if (Buffered == m_output_fd.value()) {
            m_output_fd = std::move(output_pipe.get<0>());
            event_loop->add(m_output_fd.value(), EventLoop::Read, m_on_output);

            redirect = true;
        }
        else {
            m_output_fd.reset();
        }

        if (Buffered == m_error_fd.value()) {
            m_error_fd = std::move(error_pipe.get<0>());
            event_loop->add(m_error_fd.value(), EventLoop::Read, m_on_error);

            redirect = true;
        }
        else {
            m_error_fd.reset();
        }

        input_pipe.close();
        output_pipe.close();
        error_pipe.close();

        event_loop.reset();

        if (nullptr == m_event_loop_private) {
            return Pending;
        }

        if (true == redirect) {
            m_event_loop_private->flush();
            m_event_loop_private->dispatch();
        }

        return wait();
    }
}

//
// Public
//
Subprocess::Subprocess()
    : m_event_loop_private(std::make_shared<EventLoop>())
    , m_input_fd(Config::subprocessStdIn(), &file::close_fd)
    , m_output_fd(Config::subprocessStdOut(), &file::close_fd)
    , m_error_fd(Config::subprocessStdErr(), &file::close_fd)
    , m_on_input(std::bind(&Subprocess::onInput, this, std::placeholders::_1, std::placeholders::_2))
    , m_on_output(std::bind(&Subprocess::onOutput, this, std::placeholders::_1, std::placeholders::_2))
    , m_on_error(std::bind(&Subprocess::onError, this, std::placeholders::_1, std::placeholders::_2))
{
}

Subprocess::Subprocess(std::weak_ptr<EventLoop> event_loop, EventLoop::Callback on_input,
        EventLoop::Callback on_output, EventLoop::Callback on_error) noexcept
    : m_event_loop_extern(std::move(event_loop))
    , m_input_fd(Config::subprocessStdIn(), &file::close_fd)
    , m_output_fd(Config::subprocessStdOut(), &file::close_fd)
    , m_error_fd(Config::subprocessStdErr(), &file::close_fd)
    , m_on_input(std::move(on_input))
    , m_on_output(std::move(on_output))
    , m_on_error(std::move(on_error))
{
}

Subprocess::Subprocess(std::string const& command, std::vector<std::string> const& args)
    : Subprocess()
{ 
    run(command, args);
}

Subprocess::Subprocess(std::string const& command, std::vector<std::string> const& args, Buffer input)
    : Subprocess()
{ 
    run(command, args, std::move(input));
}

Subprocess::Subprocess(Subprocess&& that) noexcept
    : m_event_loop_private(std::move(that.m_event_loop_private))
    , m_event_loop_extern(std::move(that.m_event_loop_extern))
    , m_return_code{ that.m_return_code }
    , m_input_fd(std::move(that.m_input_fd))
    , m_output_fd(std::move(that.m_output_fd))
    , m_error_fd(std::move(that.m_error_fd))
    , m_input(std::move(that.m_input))
    , m_output(std::move(that.m_output))
    , m_error(std::move(that.m_error))
    , m_on_input(std::move(that.m_on_input))
    , m_on_output(std::move(that.m_on_output))
    , m_on_error(std::move(that.m_on_error))
{
    that.m_return_code = Pending;
}

Subprocess& Subprocess::operator =(Subprocess&& that) noexcept
{
    m_event_loop_private = std::move(that.m_event_loop_private);
    m_event_loop_extern = std::move(that.m_event_loop_extern);
    m_return_code = that.m_return_code;
    m_input_fd = std::move(that.m_input_fd);
    m_output_fd = std::move(that.m_output_fd);
    m_error_fd = std::move(that.m_error_fd);
    m_input = std::move(that.m_input);
    m_output = std::move(that.m_output);
    m_error = std::move(that.m_error);
    m_on_input = std::move(that.m_on_input);
    m_on_output = std::move(that.m_on_output);
    m_on_error = std::move(that.m_on_error);

    that.m_return_code = Pending;
    return *this;
}

int Subprocess::pid() const noexcept
{
    return m_pid;
}

int Subprocess::returnCode() const noexcept
{
    return m_return_code;
}

Buffer const& Subprocess::output() const noexcept
{
    return m_output;
}

Buffer const& Subprocess::error() const noexcept
{
    return m_error;
}

void Subprocess::setInput(int fd) noexcept
{
    m_input_fd.reset(fd);
}

void Subprocess::setInput(std::string const& filename, int flags)
{
    assert(0 == ((O_CREAT|O_WRONLY) & flags));

    int fd = open(filename.c_str(), flags);
    if (-1 == fd) {
        throwf<std::runtime_error>("Failed to open '{}' for subprocess input ({})", filename, get_error_string(errno));
    }

    setInput(fd);
}

void Subprocess::setOutput(int fd) noexcept
{
    m_output_fd.reset(fd);
}

void Subprocess::setOutput(std::string const& filename, int flags, mode_t mode)
{
    assert(0 == ((O_RDONLY) & flags));

    int fd = open(filename.c_str(), flags, mode);
    if (-1 == fd) {
        throwf<std::runtime_error>("Failed to open '{}' for subprocess output ({})", filename, get_error_string(errno));
    }

    setOutput(fd);
}

void Subprocess::setError(int fd) noexcept
{
    m_error_fd.reset(fd);
}

void Subprocess::setError(std::string const& filename, int flags, mode_t mode)
{
    assert(0 == ((O_RDONLY) & flags));

    int fd = open(filename.c_str(), flags, mode);
    if (-1 == fd) {
        throwf<std::runtime_error>("Failed to open '{}' for subprocess error ({})", filename, get_error_string(errno));
    }

    setError(fd);
}

int Subprocess::run(std::string const& command, std::vector<std::string> const& args)
{
    return run(to_argv(command, args));
}

int Subprocess::run(std::string const& command, std::vector<std::string> const& args, Buffer input)
{
    m_input_fd.reset(Buffered);
    m_input = std::move(input);
    return run(to_argv(command, args));
}

int Subprocess::wait()
{
    int status;

    if (m_pid != waitpid(m_pid, &status, 0)) {
        throwf<std::runtime_error>("Failed to wait for pid {} to exit ({})", m_pid, get_error_string(errno));
    }

    if (WIFEXITED(status)) {
        m_return_code = WEXITSTATUS(status);
    }

    std::shared_ptr<EventLoop> event_loop = m_event_loop_private
        ? m_event_loop_private
        : m_event_loop_extern.lock();
    if (nullptr != event_loop) {
        if (-1 != m_error_fd.value()) {
            event_loop->remove(m_error_fd.value());
        }
        if (-1 != m_output_fd.value()) {
            event_loop->remove(m_output_fd.value());
        }
        if (-1 != m_input_fd.value()) {
            event_loop->remove(m_input_fd.value());
        }
    }

    m_error_fd.reset(Config::subprocessStdErr());
    m_output_fd.reset(Config::subprocessStdOut());
    m_input_fd.reset(Config::subprocessStdIn());

    m_pid = -1;
    return m_return_code;
}
