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
#include "hlib/time.hpp"

using namespace hlib;

TEST_CASE("Duration", "[time]")
{
    time::Duration d;

    d = time::Duration(time::Sec(1));
    REQUIRE(1 == d.tv_sec);
    REQUIRE(0 == d.tv_nsec);

    d = time::Duration(time::MSec(1234));
    REQUIRE(1 == d.tv_sec);
    REQUIRE(234000000 == d.tv_nsec);

    d = time::Duration(math::RatioValue<std::kilo>(1234));
    REQUIRE(1234000 == d.tv_sec);
    REQUIRE(0 == d.tv_nsec);
}

TEST_CASE("Clock UTC Strings", "[time]")
{
    time::Clock const now = time::now_utc();
    std::string iso8601;

    iso8601 = to_string_utc_date_and_time(now);
    REQUIRE(iso8601 == to_string_utc_date_and_time(time::to_clock(iso8601)));

    iso8601 = to_string_utc(now);
    REQUIRE(iso8601 == to_string_utc(time::to_clock(iso8601)));

    iso8601 = to_string_utc(now, true);
    REQUIRE(iso8601 == to_string_utc(time::to_clock(iso8601), true));

    iso8601 = to_string_local(now);
    REQUIRE(iso8601 == to_string_local(time::to_clock(iso8601)));
}

