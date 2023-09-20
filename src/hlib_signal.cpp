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
#include "hlib/signal.hpp"
#include "hlib/lock.hpp"
#include <condition_variable>
#include <mutex>
#include <unordered_map>

using namespace hlib;

namespace
{

struct ActiveSignalHandler
{
    struct sigaction old_action{};
    OnSignal callback;

    ActiveSignalHandler(OnSignal&& on_signal)
        : callback(std::move(on_signal))
    {
    }
};

std::unordered_map<Signal, ActiveSignalHandler> signal_handlers;

std::mutex wait_mutex;
std::condition_variable wait_cv;
volatile sig_atomic_t wait_signal{ 0 };

void wait_on_signal(int /* signal */) noexcept
{
    HLIB_LOCK_GUARD(lock, wait_mutex);
    wait_signal = 1;
    wait_cv.notify_one();
}

} // namespace

bool hlib::set_signal_handler(Signal signal, OnSignal callback) noexcept
{
    auto handler = [](int sig) noexcept
    {
        auto it = signal_handlers.find(static_cast<Signal>(sig));
        assert(signal_handlers.end() != it);

        it->second.callback(static_cast<Signal>(sig));
    };

    // Clear previous installed signal handler.
    clear_signal_handler(signal);

    // Emplace signal hander.
    auto result = signal_handlers.emplace(signal, std::move(callback));
    assert(true == result.second);
    auto& active_signal_handler = result.first->second;

    // Install signal action.
    struct sigaction new_action{};
    new_action.sa_handler = handler;
    sigemptyset(&new_action.sa_mask);
    if (-1 == sigaction(static_cast<int>(signal), &new_action, &active_signal_handler.old_action)) {
        signal_handlers.erase(signal);
        return false;
    }

    return true;
}

bool hlib::clear_signal_handler(Signal signal) noexcept
{
    // Find signal handler, if any.
    auto it = signal_handlers.find(signal);
    if (signal_handlers.end() == it) {
        return false;
    }

    // Defuse signal action.
    if (-1 != sigaction(static_cast<int>(signal), &it->second.old_action, nullptr)) {
        return false;
    }

    // Remove signal handler.
    signal_handlers.erase(it);
    return true;
}

bool hlib::wait_for_signal(int signal) noexcept
{
    struct sigaction new_action{};
    struct sigaction old_action{};
    new_action.sa_handler = wait_on_signal;
    sigemptyset(&new_action.sa_mask);
    if (-1 == sigaction(signal, &new_action, &old_action)) {
        return false;
    }

    HLIB_UNIQUE_LOCK(lock, wait_mutex);
    wait_cv.wait(lock, [] { return 0 != wait_signal; });

    if (-1 != sigaction(signal, &old_action, nullptr)) {
        return false;
    }

    return true;
}

