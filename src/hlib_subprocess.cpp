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

void close_fd(int fd)
{
    if (-1 == fd) {
        return;
    }

    close(fd);
}

struct Pipe
{
    UniqueOwner<int, -1> produce;
    UniqueOwner<int, -1> consume;

    Pipe(bool enable)
        : produce(&close_fd)
        , consume(&close_fd)
    {
        if (false == enable) {
            return;
        }

        std::array<int, 2> fds{ -1, -1 };

        if (-1 == pipe(fds.data())) {
            throwf<std::runtime_error>("Failed to create pipe for ({})", get_error_string(errno));
        }

        produce.reset(fds[0]);
        consume.reset(fds[1]);
    }

    ~Pipe()
    {
        close();
    }

    template<std::size_t index>
    void close() noexcept
    {
        static_assert(index >= 0 && index <= 1);
        if constexpr(0 == index) {
            produce.reset();
        }
        else {
            consume.reset();
        }
    }

    void close() noexcept
    {
        close<1>();
        close<0>();
    }
};

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
    assert(-1 == m_input_fd.value());
    assert(-1 == m_output_fd.value());
    assert(-1 == m_error_fd.value());

    m_return_code = Pending;

    m_input_fd.reset();
    m_output_fd.reset();
    m_error_fd.reset();

    m_output.clear();
    m_error.clear();

    std::shared_ptr<EventLoop> event_loop = m_event_loop_private
        ? m_event_loop_private
        : m_event_loop_extern.lock();
    if (nullptr == event_loop) {
        throwf<std::logic_error>("External event loop not available");
    }

    Pipe input_pipe(!!(m_redirect & StdIn));
    Pipe output_pipe(!!(m_redirect & StdOut));
    Pipe error_pipe(!!(m_redirect & StdErr));

    switch (m_pid = fork()) {
    case -1:
        throwf<std::runtime_error>("Failed to fork for '{}' ({})", argv[0], get_error_string(errno));

    case 0:
        event_loop.reset();

        if (StdIn & m_redirect) {
            hverify(-1 != dup2(input_pipe.produce.value(), STDIN_FILENO));
        }
        if (StdOut & m_redirect) {
            hverify(-1 != dup2(output_pipe.consume.value(), STDOUT_FILENO));
        }
        if (StdErr & m_redirect) {
            hverify(-1 != dup2(error_pipe.consume.value(), STDERR_FILENO));
        }

        input_pipe.close();
        output_pipe.close();
        error_pipe.close();

        hverify(-1 == execvp(argv[0], const_cast<char * const *>(argv.data())));

        fmt::print(stderr, "execvp failed for '{}' ({})", argv[0], get_error_string(errno));
        abort();
        return Error;

    default:
        m_input_fd = std::move(input_pipe.consume);
        m_output_fd = std::move(output_pipe.produce);
        m_error_fd = std::move(error_pipe.produce);

        if (StdIn & m_redirect) {
            event_loop->add(m_input_fd.value(), EventLoop::Write, m_on_input);
        }
        if (StdOut & m_redirect) {
            event_loop->add(m_output_fd.value(), EventLoop::Read, m_on_output);
        }
        if (StdErr & m_redirect) {
            event_loop->add(m_error_fd.value(), EventLoop::Read, m_on_error);
        }

        input_pipe.close();
        output_pipe.close();
        error_pipe.close();

        event_loop.reset();

        if (nullptr == m_event_loop_private) {
            return Pending;
        }

        if (0 != m_redirect) {
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
    , m_input_fd(&close_fd)
    , m_output_fd(&close_fd)
    , m_error_fd(&close_fd)
    , m_on_input(std::bind(&Subprocess::onInput, this, std::placeholders::_1, std::placeholders::_2))
    , m_on_output(std::bind(&Subprocess::onOutput, this, std::placeholders::_1, std::placeholders::_2))
    , m_on_error(std::bind(&Subprocess::onError, this, std::placeholders::_1, std::placeholders::_2))
{
}

Subprocess::Subprocess(std::weak_ptr<EventLoop> event_loop, EventLoop::Callback on_input,
        EventLoop::Callback on_output, EventLoop::Callback on_error) noexcept
    : m_event_loop_extern(std::move(event_loop))
    , m_input_fd(&close_fd)
    , m_output_fd(&close_fd)
    , m_error_fd(&close_fd)
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

int Subprocess::returnCode() const
{
    return m_return_code;
}

Buffer const& Subprocess::output() const
{
    return m_output;
}

Buffer const& Subprocess::error() const
{
    return m_error;
}

void Subprocess::setCaptureMask(std::uint8_t mask)
{
    assert(0 == (m_redirect & (~(StdOut|StdErr))));
    m_redirect = mask;
}

int Subprocess::run(std::string const& command, std::vector<std::string> const& args)
{
    m_redirect &= ~StdIn;
    return run(to_argv(command, args));
}

int Subprocess::run(std::string const& command, std::vector<std::string> const& args, Buffer input)
{
    m_redirect |= StdIn;
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

    m_error_fd.reset();
    m_output_fd.reset();
    m_input_fd.reset();

    m_pid = -1;
    return m_return_code;
}
