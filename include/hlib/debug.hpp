//
// MIT License
//
// Copyright (c) 2024 Maarten Hoeben
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

#include "hlib/macro.hpp"
#include "hlib/math.hpp"
#include "hlib/time.hpp"
#include <sys/ptrace.h>
#include <unistd.h>

namespace hlib
{

#ifndef NDEBUG
#define HLIB_DEBUG_HERE         (__FILE__ ":" HLIB_STRINGIFY_NUMBER(__LINE__))
#define HLIB_DEBUG_HERE_STRING  std::string(HLIB_DEBUG_HERE)
#else
#define HLIB_DEBUG_HERE         ""
#define HLIB_DEBUG_HERE_STRING  std::string()
#endif

#if defined(NDEBUG)
#define HLIB_DEBUGGER() do { } while()
#elif defined(__GNUC__) || defined(__clang__)
#define HLIB_DEBUGGER() __builtin_debugtrap();
#elif defined(_MSC_VER)
#define HLIB_DEBUGGER() __debugbreak();
#elif defined(__i386__) || defined(__x86_64__)
#define HLIB_DEBUGGER() __asm__("int $3");
#elif defined(__aarch64__)
#define HLIB_DEBUGGER() __asm__("brk #0");
#elif defined(__arm__)
#define HLIB_DEBUFFER() __asm__("bkpt #0");
#elif
#error "Unsupported compiler or architecture"
#endif

bool debugger_is_attached() noexcept;
#define HLIB_DEBUGGER_IS_ATTACHED() debugger_is_attached()

void debugger_wait_for_attachment(time::Duration const& sleep_interval = time::Sec(1)) noexcept;
#define HLIB_DEBUGGER_WAIT_FOR_ATTACHM() debugger_wait_for_attachment()

} // namespace hlib
