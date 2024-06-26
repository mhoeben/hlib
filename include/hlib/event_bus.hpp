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

#include "hlib/base.hpp"
#include "hlib/event_queue.hpp"
#include <any>
#include <list>
#include <mutex>
#include <string>
#include <unordered_map>

namespace hlib
{

//
// The following is a useful convention:
//
// * Components subscribe with their name to events they export, for example:
//
//      subscribe("ipc", "send", callback);
//
// * Other components can address those exported events as follows:
//
//      notify("ipc", "send", data);
//
// * Components can also emit events to any component interested by broadcasting events.
//   In this case the action is prefixed by the origin component's name:
//
//      broadcast("ipc", "ipc-receive", data);
//
// * Interested components anonymously subscribe to these events with their own name:
//
//     subscribe("component", "ipc-receive", callback);
//
class EventBus final
{
    HLIB_NOT_COPYABLE(EventBus);
    HLIB_NOT_MOVABLE(EventBus);
 
public:
    typedef std::function<void(std::any data)> Callback;

    struct Subscription
    {
        std::weak_ptr<EventQueue> queue;
        Callback callback;
    };

public:
    EventBus() = default;

    Subscription get(std::string const& name, std::string const& action) const;

    bool subscribe(std::string name, std::string action, Subscription subscription);
    bool subscribe(std::string name, std::string action, std::weak_ptr<EventQueue> queue, Callback callback);
    void unsubscribe(std::string const& name, std::string const& action);

    void notify(std::string const& name, std::string const& action, std::any data = {});
    void broadcast(std::string const& action, std::any data = {});

private:
    mutable std::mutex m_mutex;

    std::unordered_map<
        std::string, // action,
        std::unordered_map<
            std::string, // name,
            Subscription
        >
    > m_actions;
};

} // namespace hlib

