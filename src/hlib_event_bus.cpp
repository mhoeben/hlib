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
#include "hlib/event_bus.hpp"
#include "hlib/lock.hpp"

using namespace hlib;

//
// Public
//
void EventBus::subscribe(std::string tag, std::weak_ptr<EventQueue> queue, Callback callback)
{
    HLIB_LOCK_GUARD(lock, m_mutex);

    m_subscriptions.emplace(std::move(tag), Subscription{ std::move(queue), std::move(callback) });
}

void EventBus::unsubscribe(std::string const& tag)
{
    HLIB_LOCK_GUARD(lock, m_mutex);

    m_subscriptions.erase(tag);
}

void EventBus::raise(std::string const& tag, std::any data)
{
    HLIB_UNIQUE_LOCK(lock, m_mutex);

    // Find subscription of tag.
    auto it = m_subscriptions.find(tag);
    if (m_subscriptions.end() == it) {
        return;
    }

    // Obtain a shared pointer to the event queue.
    std::shared_ptr<EventQueue> queue = it->second.queue.lock();
    if (nullptr == queue) {
        // Remove subscription if the queue has gone away.
        m_subscriptions.erase(it);
        return;
    }

    // Bind data to event queue callback.
    EventQueue::Callback callback = std::bind(it->second.callback, std::move(data));

    lock.unlock();

    // Push callback.
    queue->push(std::move(callback));
}

