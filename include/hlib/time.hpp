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

#include "hlib/math.hpp"
#include <ctime>
#include <cmath>
#include <string>
#include <type_traits>

namespace hlib
{
namespace time
{

typedef math::RatioValue<math::One, int64_t> Sec;
typedef math::RatioValue<std::milli, int64_t> MSec;
typedef math::RatioValue<std::micro, int64_t> USec;
typedef math::RatioValue<std::nano, int64_t> NSec;

struct Clock;

class Duration final : public std::timespec
{
    Duration(Clock const& that) = delete;

public:
    Duration() noexcept;
    explicit Duration(std::timespec const& ts) noexcept;
    Duration(std::time_t secs, long nsecs) noexcept;
    Duration(double secs) noexcept;

    template<typename R, typename T = int64_t>
    Duration(math::RatioValue<R, T> const& value) noexcept
    {
        T const& ticks = value.value();
        tv_sec = (ticks * R::num) / R::den;

        constexpr int64_t factor = (1000000000LL * R::num) / R::den;
        tv_nsec = (ticks * factor) % 1000000000ULL;
    }

    virtual ~Duration() = default;

    Duration operator +(Duration const& that) const noexcept;
    Duration operator -(Duration const& that) const noexcept;

    Duration& operator +=(Duration const& that) noexcept;
    Duration& operator -=(Duration const& that) noexcept;

    Duration operator *(double that) const noexcept; 
    Duration operator /(double that) const;

    Duration& operator *=(double that) noexcept;
    Duration& operator /=(double that);

    bool operator !() const noexcept;
    bool operator ==(Duration const& that) const noexcept;
    bool operator !=(Duration const& that) const noexcept;

    bool operator <(Duration const& that) const noexcept;
    bool operator >(Duration const& that) const noexcept;

    bool operator <=(Duration const& that) const noexcept;
    bool operator >=(Duration const& that) const noexcept;

    template<typename T>
    typename std::enable_if<std::is_same<double, T>::value, T>::type
    to() const noexcept
    {
        return static_cast<double>(tv_sec)
             + static_cast<double>(tv_nsec) / static_cast<double>(1000000000L);
    }

    template<typename T>
    typename std::enable_if<!std::is_same<double, T>::value, T>::type
    to() const noexcept
    {
        math::RatioValue<std::nano, int64_t> nsec(std::int64_t(tv_sec) * 1000000000LL + tv_nsec);
        return nsec.to<typename T::Ratio, typename T::Type>();
    }
};

class Clock final : public std::timespec
{
    Clock(Duration const&) noexcept = delete;

public:
    Clock() noexcept;
    explicit Clock(std::timespec const& ts) noexcept;
    explicit Clock(std::time_t sec, long nsec) noexcept;
    Clock(clockid_t clock_id, std::nothrow_t) noexcept;
    Clock(clockid_t clock_id);

    Duration operator - (Clock const& that) const noexcept;

    Clock operator + (Duration const& that) const noexcept;
    Clock operator - (Duration const& that) const noexcept;

    Clock& operator += (Duration const& that) noexcept;
    Clock& operator -= (Duration const& that) noexcept;

    bool operator !() const noexcept;
    bool operator ==(std::timespec const& that) const noexcept;
    bool operator !=(std::timespec const& that) const noexcept;

    bool operator <(std::timespec const& that) const noexcept;
    bool operator >(std::timespec const& that) const noexcept;

    bool operator <=(std::timespec const& that) const noexcept;
    bool operator >=(std::timespec const& that) const noexcept;
};

Clock now(clockid_t clock_id = CLOCK_MONOTONIC);
Clock now_utc(clockid_t clock_id = CLOCK_REALTIME);

} // namespace time

std::string to_string(time::Duration const& duration, bool milliseconds = false);

std::string to_string_utc(time::Clock const& clock, bool milliseconds = false);
std::string to_string_local(time::Clock const& clock, bool milliseconds = false);

} // namespace hlib

