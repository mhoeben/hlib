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
#include <csignal>
#include <functional>

namespace hlib
{

enum class Signal
{
    Abort = SIGABRT,
    Alarm = SIGALRM,
    BusError = SIGBUS,
    Child = SIGCHLD,
    Continue = SIGCONT,
    Hup = SIGHUP,
    IllegalInstruction = SIGILL,
    Interrupt = SIGINT,
    Kill = SIGKILL,
    Pipe = SIGPIPE,
    Poll = SIGPOLL,
    Quit = SIGQUIT,
    Stop = SIGSTOP,
    TStop = SIGTSTP,
    Terminate = SIGTERM,
    User1 = SIGUSR1,
    User2 = SIGUSR2
};

typedef std::function<void(Signal signal)> OnSignal;

bool set_signal_handler(Signal signal, OnSignal callback);
bool clear_signal_handler(Signal signal);

bool wait_for_signal(Signal signal);

bool raise(Signal signal) noexcept;

} // namespace hlib

