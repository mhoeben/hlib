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

#include <mutex>

//
// Some useful macros to prevent locking errors like:
//
// std::lock_guard<std::mutex> (m_mutex);
//
// The macros also reduce verbosity:
//
// HLIB_LOCK_GUARD(lock, m_mutex);
//
#define HLIB_LOCK_GUARD(variable_name, mutex) \
    std::lock_guard<decltype(mutex)> variable_name(mutex)

#define HLIB_UNIQUE_LOCK(variable_name, mutex) \
    std::unique_lock<decltype(mutex)> variable_name(mutex)

#define HLIB_UNIQUE_LOCK_DEFERRED(variable_name, mutex) \
    std::unique_lock<decltype(mutex)> variable_name(mutex, std::defer_lock)
