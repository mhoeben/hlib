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
void EventBus::subscribe(std::string name, std::string action, std::weak_ptr<EventQueue> queue, Callback callback)
{
    HLIB_LOCK_GUARD(lock, m_mutex);

    m_actions[action][name] = Subscription{ std::move(queue), std::move(callback) };
}

void EventBus::unsubscribe(std::string const& name, std::string const& action)
{
    HLIB_LOCK_GUARD(lock, m_mutex);

    // Find action.
    auto it = m_actions.find(action);
    if (m_actions.end() == it) {
        return;
    }

    // Find subscription by name.
    auto jt = it->second.find(name);
    if (it->second.end() == jt) {
        return;
    }

    // Erase named subscription.
    it->second.erase(jt);

    // Cleanup all actions without subscription.
    for (it = m_actions.begin(); it != m_actions.end(); /* in-loop */) {
        if (true == it->second.empty()) {
            it = m_actions.erase(it);
            continue;
        }
        ++ it;
    }
}

void EventBus::notify(std::string const& name, std::string const& action, std::any data)
{
    HLIB_LOCK_GUARD(lock, m_mutex);

    // Find action.
    auto it = m_actions.find(action);
    if (m_actions.end() == it) {
        return;
    }

    // Find subscription by name.
    auto jt = it->second.find(name);
    if (it->second.end() == jt) {
        return;
    }

    // Obtain a shared pointer to the event queue.
    std::shared_ptr<EventQueue> queue = jt->second.queue.lock();
    if (nullptr == queue) {
        // Remove subscription if the queue has gone away.
        it->second.erase(jt);
        return;
    }

    // Bind data to event queue callback.
    EventQueue::Callback callback = std::bind(jt->second.callback, std::move(data));

    // Push callback on event queue.
    queue->push(std::move(callback));
}

void EventBus::broadcast(std::string const& action, std::any data)
{
    HLIB_LOCK_GUARD(lock, m_mutex);

    // Find action.
    auto it = m_actions.find(action);
    if (m_actions.end() == it) {
        return;
    }

    // Broadcast action to all subscriptions.
    for (auto jt = it->second.begin(); jt != it->second.end(); /* in-loop */) {
        // Obtain a shared pointer to the event queue.
        std::shared_ptr<EventQueue> queue = jt->second.queue.lock();
        if (nullptr == queue) {
            // Remove subscription if the queue has gone away.
            jt = it->second.erase(jt);
            continue;
        }

        // Bind data to event queue callback.
        EventQueue::Callback callback = std::bind(jt->second.callback, data);

        // Push callback on event queue.
        queue->push(std::move(callback));

        ++jt;
    }
}

