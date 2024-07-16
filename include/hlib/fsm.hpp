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

#include <functional>
#include <unordered_map>

namespace hlib
{

template<typename State, typename Event>
class FSM final
{
    static_assert(std::is_enum<State>::value, "State must be an enum type");
    static_assert(std::is_enum<Event>::value, "Event must be an enum type");

    static_assert(sizeof(typename std::underlying_type<State>::type) <= 4,
                  "The underlying type of State must not be larger than 32 bits");
    static_assert(sizeof(typename std::underlying_type<Event>::type) <= 4,
                  "The underlying type of Event must not be larger than 32 bits");

public:
    typedef std::function<void(State from, Event event, State to)> Callback;

    struct Transition
    {
        State from;
        Event event;
        State to;
        Callback callback;

        Transition(State a_from, Event a_event, State a_to, Callback a_callback)
            : from{ a_from }
            , event{ a_event }
            , to{ a_to }
            , callback(std::move(a_callback))
        {
        }

        Transition(State a_from, Event a_event, State a_to)
            : Transition(a_from, a_event, a_to, {})
        {
        }
    };

    FSM() = default;

    FSM(State initial, std::vector<Transition> const& transitions)
        : m_initial{ initial }
        , m_state{ m_initial }
    {
        for (auto const& transition : transitions) {
            m_transitions.emplace(combine(transition.from, transition.event), std::make_pair(transition.to, transition.callback));
        }
    }

    State state() const noexcept
    {
        return m_state;
    }

    void reset() noexcept
    {
        m_state = m_initial;
    }

    bool apply(Event event) noexcept
    {
        std::uint64_t const key = combine(m_state, event);

        auto it = m_transitions.find(key);
        if (m_transitions.end() == it) {
            return false;
        }

        State from = m_state;
        State to = it->second.first;

        if (nullptr != it->second.second) {
            it->second.second(from, event, to);
        }

        m_state = to;
        return true;
    }

private:
    State m_initial = State();
    State m_state = State();
    std::unordered_map<std::uint64_t, std::pair<State, Callback>> m_transitions;

    static constexpr std::uint64_t combine(State state, Event event) noexcept
    {
        return (static_cast<std::uint64_t>(state))
             | (static_cast<std::uint64_t>(event) << 32);
    }
};

} // namespace hlib

