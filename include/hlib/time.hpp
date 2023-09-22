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
#include <type_traits>

namespace hlib
{
namespace time
{

struct Duration
{
    std::timespec timespec;

    Duration() noexcept;
    explicit Duration(std::timespec const& ts) noexcept;
    Duration(std::time_t secs, long nsecs) noexcept;
    Duration(double secs) noexcept;

    virtual ~Duration() = default;

    operator double() const noexcept;

    Duration operator +(Duration const& rhs) const noexcept;
    Duration operator -(Duration const& rhs) const noexcept;

    Duration& operator +=(Duration const& rhs) noexcept;
    Duration& operator -=(Duration const& rhs) noexcept;

    Duration operator *(double rhs) const noexcept; 
    Duration operator /(double rhs) const;

    Duration& operator *=(double rhs) noexcept;
    Duration& operator /=(double rhs);

    template<typename T>
    typename std::enable_if<std::is_same<double, T>::value, T>::type
    to()
    {
        return static_cast<double>(timespec.tv_sec)
             + static_cast<double>(timespec.tv_nsec) / static_cast<double>(1000000000L);
    }

    template<typename T>
    typename std::enable_if<!std::is_same<double, T>::value, T>::type
    to()
    {
        math::RatioValue<std::nano, int64_t> nsec(std::int64_t(timespec.tv_sec) * 1000000000LL + timespec.tv_nsec);
        return nsec.to<typename T::Ratio, typename T::Type>();
    }
};

typedef math::RatioValue<math::One, int64_t> Sec;
typedef math::RatioValue<std::milli, int64_t> MSec;
typedef math::RatioValue<std::micro, int64_t> USec;
typedef math::RatioValue<std::nano, int64_t> NSec;

struct Clock : std::timespec
{
    Clock() noexcept;
    Clock(clockid_t clock_id) noexcept;
    explicit Clock(std::timespec const& ts) noexcept;

    Duration operator - (Clock const& rhs) const noexcept;

    Clock operator + (Duration const& rhs) const noexcept;
    Clock operator - (Duration const& rhs) const noexcept;

    Clock& operator += (Duration const& rhs) noexcept;
    Clock& operator -= (Duration const& rhs) noexcept;

    bool operator !() const noexcept;
    bool operator ==(std::timespec const& rhs) const noexcept;
    bool operator !=(std::timespec const& rhs) const noexcept;

    bool operator <(std::timespec const& rhs) const noexcept;
    bool operator >(std::timespec const& rhs) const noexcept;

    bool operator <=(std::timespec const& rhs) const noexcept;
    bool operator >=(std::timespec const& rhs) const noexcept;
};

Clock now(clockid_t clock_id = CLOCK_MONOTONIC);

} // namespace time
} // namespace hlib

