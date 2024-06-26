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
#include "hlib/latch.hpp"
#include "hlib/lock.hpp"

using namespace hlib;

//
// Public
//
Latch::Latch(std::ptrdiff_t expected)
    : m_count{ expected }
{
}

void Latch::countDown(std::ptrdiff_t n)
{
    HLIB_UNIQUE_LOCK(lock, m_mutex);

    if (m_count <= 0) {
        return;
    }

    assert(n <= m_count);
    m_count -= n;

    if (m_count > 0) {
        return;
    }

    lock.unlock();
    m_condition_variable.notify_all();
}

bool Latch::tryWait() const
{
    HLIB_UNIQUE_LOCK(lock, m_mutex);
    return 0 == m_count;
}

void Latch::wait()
{
    HLIB_UNIQUE_LOCK(lock, m_mutex);
    m_condition_variable.wait(lock, [this] { return 0 == m_count; });
}


void Latch::arriveAndWait(std::ptrdiff_t n)
{
    countDown(n);
    wait();
}

