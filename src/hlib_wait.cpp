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
#include "hlib/wait.hpp"
#include <condition_variable>
#include <mutex>
#include <signal.h>

namespace
{

std::mutex wait_mutex;
std::condition_variable wait_cv;
volatile sig_atomic_t wait_signal{ 0 };

void wait_on_signal(int /* signal */)
{
    std::lock_guard<std::mutex> lock(wait_mutex);
    wait_signal = 1;
    wait_cv.notify_one();
}

} // namespace

void hlib::wait_for_signal(int signal)
{
    struct sigaction new_action{};
    struct sigaction old_action{};
    new_action.sa_handler = wait_on_signal;
    sigemptyset(&new_action.sa_mask);
    hverify(0 == sigaction(signal, &new_action, &old_action));

    std::unique_lock<std::mutex> lock(wait_mutex);
    wait_cv.wait(lock, [] { return 0 != wait_signal; });

    hverify(0 == sigaction(signal, &old_action, nullptr));
}

