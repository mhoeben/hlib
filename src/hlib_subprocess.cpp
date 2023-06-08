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

//
// Public
//
Subprocess::Subprocess(Subprocess&& that) noexcept
    : m_return_code{ that.m_return_code }
    , m_output{ std::move(that.m_output) }
    , m_error{ std::move(that.m_error) }
{
    that.m_return_code = 0;
}

Subprocess& Subprocess::operator =(Subprocess&& that) noexcept
{
    m_return_code = that.m_return_code;
    m_output = std::move(that.m_output);
    m_error = std::move(that.m_error);

    that.m_return_code = 0;
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

void Subprocess::run(std::string command, std::vector<std::string> args)
{
    (void)command;
    (void)args;

    std::vector<char const*> argv;
    std::array<int, 2> output_pipe{ -1, -1 };
    std::array<int, 2> error_pipe{ -1, -1 };
    pid_t pid{ -1 };

    auto cleanup = [&]()
    {
        if (-1 != output_pipe[0]) {
            close(output_pipe[0]);
        }
        if (-1 != output_pipe[1]) {
            close(output_pipe[1]);
        }
        if (-1 != error_pipe[0]) {
            close(error_pipe[0]);
        }
        if (-1 != error_pipe[1]) {
            close(error_pipe[1]);
        }

        hverify(pid == waitpid(pid, nullptr, 0));
    };

    auto read_pipe = [&](int fd, Buffer& buffer) -> bool
    {
        do {
            std::uint8_t* data = reinterpret_cast<std::uint8_t*>(buffer.reserve(buffer.size() + Config::subprocessOutputBatchSize()));

            ssize_t size = read(fd, data + buffer.size(), buffer.capacity() - buffer.size());
            switch (size) {
            case -1:
                return false;

            case 0:
                return true;

            default:
                buffer.resize(buffer.size() + size);
                break;
            }
        }
        while (true);

        return true;
    };

    try {
        m_output.clear();
        m_error.clear();

        if (-1 == pipe(output_pipe.data())) {
            throwf<std::runtime_error>("Failed to create output pipe for '{}' ({})", command, get_error_string(errno));
        }
        if (-1 == pipe(error_pipe.data())) {
            throwf<std::runtime_error>("Failed to create error pipe for '{}' ({})", command, get_error_string(errno));
        }
        
        switch (pid = fork()) {
        case -1:
            throwf<std::runtime_error>("Failed to fork for '{}' ({})", command, get_error_string(errno));

        case 0:
            close(output_pipe[0]);
            output_pipe[0] = -1;

            close(error_pipe[0]);
            error_pipe[0] = -1;

            hverify(-1 != dup2(output_pipe[1], STDOUT_FILENO));
            hverify(-1 != dup2(error_pipe[1], STDERR_FILENO));

            close(output_pipe[1]);
            output_pipe[1] = -1;

            close(error_pipe[1]);
            error_pipe[1] = -1;

            argv.reserve(args.size() + 1);

            for (auto const& arg : args) {
                argv.push_back(arg.c_str());
            }
            argv.push_back(nullptr);

            hverify(-1 == execvp(command.c_str(), const_cast<char * const *>(argv.data())));

            fmt::print(stderr, "execvp failed for '{}' ({})", command, get_error_string(errno));
            abort();
            break;

        default:
            close(output_pipe[1]);
            output_pipe[1] = -1;

            close(error_pipe[1]);
            error_pipe[1] = -1;

            if (false == read_pipe(output_pipe[0], m_output)) {
                throw std::runtime_error(fmt::format("Failed to read from output pipe for '{}' ({})", command, get_error_string(errno)));
            }

            if (false == read_pipe(error_pipe[0], m_error)) {
                throw std::runtime_error(fmt::format("Failed to read from error pipe for '{}' ({})", command, get_error_string(errno)));
            }
            break;
        }

    }
    catch (...) {
        cleanup();
        throw;
    }

    cleanup();
}

