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
#include "hlib/event_queue.hpp"
#include "hlib/lock.hpp"

using namespace hlib;

//
// Implementation
//
void EventQueue::onTimer()
{
    HLIB_UNIQUE_LOCK(lock, m_mutex);
    Callback callback = std::move(m_queue.front());
    m_queue.pop_front();
    lock.unlock();

    callback();

    lock.lock();
    if (true == m_queue.empty()) {
        m_timer.clear();
    }
}

//
// Public
//
EventQueue::EventQueue(std::weak_ptr<EventLoop> event_loop)
    : m_timer(std::move(event_loop), std::bind(&EventQueue::onTimer, this))
{
}

void EventQueue::push(Callback callback)
{
    HLIB_LOCK_GUARD(lock, m_mutex);
    m_queue.emplace_back(std::move(callback));
    m_timer.set({ 0, 1 }, { 0, 1 });
}

