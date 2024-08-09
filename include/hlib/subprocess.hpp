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

#include "hlib/base.hpp"
#include "hlib/buffer.hpp"
#include "hlib/fdio.hpp"
#include "hlib/memory.hpp"
#include "hlib/result.hpp"
#include "hlib/sink.hpp"
#include "hlib/source.hpp"
#include <csignal>
#include <fcntl.h>
#include <set>
#include <string>
#include <vector>
#include <unistd.h>

namespace hlib
{

class EventLoop;

class Subprocess final
{
    HLIB_NOT_COPYABLE(Subprocess);

public:
    enum State
    {
        Idle,
        Running,
        Exited,
        Failed
    };

public:
    // Subprocess can be run with an internal event loop or external event loop.
    //
    // When running with an internal event loop, one can pass input to stdin
    // and get the subprocess's output using output() and error(). Progressive
    // read(), write() and close() options are not available. In this mode run()
    // returns only when the subprocess finishes and the caller does not have to
    // call wait() explicitely.
    //
    // When running from an external event loop, the run() method returns immediately.
    // Input shall be passed using the write() method and output/error data obtained
    // through read() methods. When all input has been submitted, close() shall be
    // called. input() and output() methods are not available in this mode. Once
    // input has closed, the user shall call wait() for the process to finish.
    //
    // kill() methods can be used in both modes.
    //
    Subprocess();
    Subprocess(std::string const& command, std::vector<std::string> const& args);
    Subprocess(std::string const& command, std::vector<std::string> const& args, Buffer&& input);
    Subprocess(std::weak_ptr<EventLoop> event_loop);

    State state() const noexcept;
    int pid() const noexcept;
    int returnCode() const noexcept;
    Buffer* output() const noexcept;
    Buffer* error() const noexcept;

    void setCloseFDs(bool enable, std::set<int> exceptions);

    Result<int> run(std::string const& command, std::vector<std::string> const& args, std::nothrow_t) noexcept;
    Result<int> run(std::string const& command, std::vector<std::string> const& args, Buffer&& input, std::nothrow_t) noexcept;

    int run(std::string const& command, std::vector<std::string> const& args);
    int run(std::string const& command, std::vector<std::string> const& args, Buffer&& input);

    void write(std::shared_ptr<Source> source, FileDescriptorIO::OnWritten callback);
    void write(std::shared_ptr<Source> source);
    void read(std::shared_ptr<Sink> sink, FileDescriptorIO::OnRead callback, bool read_stderr = false);
    void close();

    Result<int> wait(std::nothrow_t) noexcept;
    int wait();

    Result<> kill(int signal, std::nothrow_t) noexcept;
    Result<> kill(std::nothrow_t) noexcept;
    void kill(int signal = SIGKILL);

private:
    std::weak_ptr<EventLoop> m_event_loop_extern;
    std::shared_ptr<EventLoop> m_event_loop_intern;

    State m_state{ State::Idle };
    int m_pid{ -1 };
    int m_return_code{ 0 };

    std::shared_ptr<Source> m_stdin_source;
    std::shared_ptr<Sink> m_stdout_sink;
    std::shared_ptr<Sink> m_stderr_sink;

    std::unique_ptr<FileDescriptorIO> m_stdin;
    std::unique_ptr<FileDescriptorIO> m_stdout;
    std::unique_ptr<FileDescriptorIO> m_stderr;

    bool m_close_fds{ true };
    std::set<int> m_close_fds_exceptions{ STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO };

    void onWritten(std::shared_ptr<Source> const& source);
    void onClose(int error);
    Result<int> run(std::vector<char const*> argv);
};

} // namespace hlib

