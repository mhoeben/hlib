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
#include "test.hpp"
#include "hlib/fsm.hpp"

using namespace hlib;

TEST_CASE("FSM", "[fsm]")
{
    using namespace std::placeholders;

    enum State
    {
        Begin,
        Intermediate,
        End
    };

    enum Event
    {
        Next,
        Prev
    };

    State previous = Begin;
    State current = Begin;
    Event last_event;

    auto set = [&](State from, Event event, State to) noexcept {
        previous = from;
        current = to;
        last_event = event;
    };

    FSM<State, Event> fsm(current, {
        { Begin,        Next, Intermediate, set },
        { Intermediate, Next, End,          set },

        { End,          Prev, Intermediate, set },
        { Intermediate, Prev, Begin,        set }
    });

    REQUIRE(Begin == fsm.state());

    REQUIRE(true            == fsm.apply(Next));
    REQUIRE(Begin           == previous);
    REQUIRE(Intermediate    == current);
    REQUIRE(Next            == last_event);

    REQUIRE(true            == fsm.apply(Next));
    REQUIRE(Intermediate    == previous);
    REQUIRE(End             == current);
    REQUIRE(Next            == last_event);

    REQUIRE(false           == fsm.apply(Next));
    REQUIRE(Intermediate    == previous);
    REQUIRE(End             == current);
    REQUIRE(Next            == last_event);

    REQUIRE(true            == fsm.apply(Prev));
    REQUIRE(End             == previous);
    REQUIRE(Intermediate    == current);
    REQUIRE(Prev            == last_event);

    REQUIRE(true            == fsm.apply(Prev));
    REQUIRE(Intermediate    == previous);
    REQUIRE(Begin           == current);
    REQUIRE(Prev            == last_event);

    REQUIRE(false           == fsm.apply(Prev));
    REQUIRE(Intermediate    == previous);
    REQUIRE(Begin           == current);
    REQUIRE(Prev            == last_event);
}

