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
#include "hlib/container.hpp"
#include "hlib/error.hpp"
#include "hlib/file.hpp"
#include "hlib/memory.hpp"
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

} // namespace

//
// Implementation (Subprocess)
//
void Subprocess::onWritten(std::shared_ptr<Source> const& /* source */)
{
    m_stdin->close();
}

void Subprocess::onClose(int /* error */)
{
    if (nullptr == m_event_loop_intern) {
        return;
    }

    m_event_loop_intern->interrupt();
}

Result<int> Subprocess::run(std::vector<char const*> argv)
{
    using namespace std::placeholders;

    assert(-1 == m_pid);

    m_state = Failed;
    m_return_code = 0;

    // Create pipes for stdin, stdout and stderr.
    file::Pipe stdin_pipe(true);
    file::Pipe stdout_pipe(true);
    file::Pipe stderr_pipe(true);

    // Fork.
    switch (m_pid = fork()) {
    case -1:
        return make_system_error(errno, "fork() failed");

    case 0:
        // Change stdin, stdout and stderr to use the pipes.
        HVERIFY(-1 != dup2(stdin_pipe.get<0>().get(), STDIN_FILENO));
        HVERIFY(-1 != dup2(stdout_pipe.get<1>().get(), STDOUT_FILENO));
        HVERIFY(-1 != dup2(stderr_pipe.get<1>().get(), STDERR_FILENO));

        // Close pipes.
        stdin_pipe.close();
        stdout_pipe.close();
        stderr_pipe.close();

        // Close file descriptors?
        if (true == m_close_fds) {
            int const max_fd = sysconf(_SC_OPEN_MAX);

            for (int fd = 0; fd < max_fd; ++fd) {
                // Don't close file descriptors on the exception list.
                if (true == container::contains(m_close_fds_exceptions, fd)) {
                    continue;
                }
                ::close(fd);
            }
        }

        // Execute process.
        HVERIFY(0 == execvp(argv[0], const_cast<char * const *>(argv.data())));

        fprintf(stderr, "execvp() failed (%s)", get_error_string().c_str());
        abort();
        return -1;

    default:
        // Transfer relevant pipe sides to local file descriptors.
        m_stdin->open(std::move(stdin_pipe.get<1>()));      // Parent is writer.
        m_stdout->open(std::move(stdout_pipe.get<0>()));    // Parent is reader.
        m_stderr->open(std::move(stderr_pipe.get<0>()));    // Parent is reader.

        // Close pipes.
        stdin_pipe.close();
        stdout_pipe.close();
        stderr_pipe.close();

        // Add source and sinks.
        if (nullptr != m_stdin_source) {
            m_stdin->write(m_stdin_source, std::bind(&Subprocess::onWritten, this, _1));
        }
        if (nullptr != m_stdout_sink) {
            m_stdout->read(m_stdout_sink);
        }
        if (nullptr != m_stderr_sink) {
            m_stderr->read(m_stderr_sink);
        }

        m_state = Running;

        if (nullptr == m_event_loop_intern) {
            return 0;
        }

        m_event_loop_intern->flush();
        m_event_loop_intern->dispatch();
        return wait();
    }
}

//
// Public (Subprocess)
//
Subprocess::Subprocess()
    : m_event_loop_intern(std::make_shared<EventLoop>())
    , m_stdin(std::make_unique<FileDescriptorIO>(m_event_loop_intern))
    , m_stdout(std::make_unique<FileDescriptorIO>(m_event_loop_intern))
    , m_stderr(std::make_unique<FileDescriptorIO>(m_event_loop_intern))
{
    using namespace std::placeholders;

    m_stdin->setCloseCallback(std::bind(&Subprocess::onClose, this, _1));
    m_stdout->setCloseCallback(std::bind(&Subprocess::onClose, this, _1));
    m_stderr->setCloseCallback(std::bind(&Subprocess::onClose, this, _1));
}

Subprocess::Subprocess(std::string const& command, std::vector<std::string> const& args)
    : Subprocess()
{
    run(command, args);
}

Subprocess::Subprocess(std::string const& command, std::vector<std::string> const& args, Buffer&& input)
    : Subprocess()
{
    run(command, args, std::move(input));
}

Subprocess::Subprocess(std::weak_ptr<EventLoop> event_loop)
    : m_event_loop_extern(std::move(event_loop))
    , m_stdin(std::make_unique<FileDescriptorIO>(m_event_loop_extern))
    , m_stdout(std::make_unique<FileDescriptorIO>(m_event_loop_extern))
    , m_stderr(std::make_unique<FileDescriptorIO>(m_event_loop_extern))
{
    using namespace std::placeholders;

    m_stdin->setCloseCallback(std::bind(&Subprocess::onClose, this, _1));
    m_stdout->setCloseCallback(std::bind(&Subprocess::onClose, this, _1));
    m_stderr->setCloseCallback(std::bind(&Subprocess::onClose, this, _1));
}

Subprocess::State Subprocess::state() const noexcept
{
    return m_state;
}

int Subprocess::pid() const noexcept
{
    return m_pid;
}

int Subprocess::returnCode() const noexcept
{
    return m_return_code;
}

Buffer* Subprocess::output() const noexcept
{
    if (nullptr == m_stdout_sink) {
        return nullptr;
    }

#ifdef HLIB_RTTI_ENABLED
    auto sink = std::dynamic_pointer_cast<SinkAdapter<Buffer>>(m_stdout_sink);
#else
    auto sink = std::static_pointer_cast<SinkAdapter<Buffer>>(m_stdout_sink);
#endif
    return &sink->get();
}

Buffer* Subprocess::error() const noexcept
{
    if (nullptr == m_stderr_sink) {
        return nullptr;
    }

#ifdef HLIB_RTTI_ENABLED
    auto sink = std::dynamic_pointer_cast<SinkAdapter<Buffer>>(m_stderr_sink);
#else
    auto sink = std::static_pointer_cast<SinkAdapter<Buffer>>(m_stderr_sink);
#endif
    return &sink->get();
}

void Subprocess::setCloseFDs(bool enable, std::set<int> exceptions)
{
    m_close_fds = enable;
    m_close_fds_exceptions = std::move(exceptions);
    m_close_fds_exceptions.insert(STDIN_FILENO);
    m_close_fds_exceptions.insert(STDOUT_FILENO);
    m_close_fds_exceptions.insert(STDERR_FILENO);
}

Result<int> Subprocess::run(std::string const& command, std::vector<std::string> const& args, std::nothrow_t) noexcept
{
    if (Idle != m_state) {
        return std::logic_error("Cannot reuse Subprocess instance");
    }

    // Use internal source and sinks when using internal event loop.
    if (nullptr != m_event_loop_intern) {
        m_stdin_source = make_shared_source_buffer();
        m_stdout_sink = make_shared_sink_buffer();
        m_stderr_sink = make_shared_sink_buffer();
    }

    return run(to_argv(command, args));
}

Result<int> Subprocess::run(std::string const& command, std::vector<std::string> const& args, Buffer&& buffer, std::nothrow_t) noexcept
{
    if (Idle != m_state) {
        return std::logic_error("Cannot reuse Subprocess instance");
    }
    if (nullptr == m_event_loop_intern) {
        return std::logic_error("Invalid run() call on external event loop");
    }

    m_stdin_source = make_shared_source<Buffer>(std::move(buffer));
    m_stdout_sink = make_shared_sink_buffer();
    m_stderr_sink = make_shared_sink_buffer();

    return run(to_argv(command, args));
}

int Subprocess::run(std::string const& command, std::vector<std::string> const& args)
{
    return success_or_throw(run(command, args, std::nothrow));
}

int Subprocess::run(std::string const& command, std::vector<std::string> const& args, Buffer&& buffer)
{
    return success_or_throw(run(command, args, std::move(buffer), std::nothrow));
}

void Subprocess::write(std::shared_ptr<Source> source, FileDescriptorIO::OnWritten callback)
{
    assert(Running == m_state);
    m_stdin->write(std::move(source), std::move(callback));
}

void Subprocess::write(std::shared_ptr<Source> source)
{
    assert(Running == m_state);
    m_stdin->write(std::move(source));
}

void Subprocess::read(std::shared_ptr<Sink> sink, FileDescriptorIO::OnRead callback, bool read_stderr)
{
    assert(Running == m_state);
    if (true == read_stderr) {
        m_stderr->read(std::move(sink), std::move(callback));
    }
    else {
        m_stdout->read(std::move(sink), std::move(callback));
    }
}

void Subprocess::close()
{
    m_stdin->close();
}

Result<int> Subprocess::wait(std::nothrow_t) noexcept
{
    assert(Running == m_state);

    int status;

    m_state = Failed;

    if (m_pid != waitpid(m_pid, &status, 0)) {
        return make_system_error(errno, "waitpid() failed");
    }

    if (WIFEXITED(status)) {
        m_return_code = WEXITSTATUS(status);
    }

    m_pid = -1;
    m_state = Exited;
    return m_return_code;
}

int Subprocess::wait()
{
    return success_or_throw(wait(std::nothrow));
}

Result<> Subprocess::kill(int signal, std::nothrow_t) noexcept
{
    if (m_pid <= 0) {
        errno = 0;
        return Result<>();
    }

    if (-1 == ::kill(m_pid, signal)) {
        return make_system_error(errno, "kill() failed");
    }

    return Result<>();
}

Result<> Subprocess::kill(std::nothrow_t) noexcept
{
    return kill(SIGKILL, std::nothrow);
}

void Subprocess::kill(int signal)
{
    success_or_throw(kill(signal, std::nothrow));
}

