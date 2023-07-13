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

#include "hlib/base.hpp"
#include <ctime>
#include <cmath>
#include <type_traits>

namespace hlib
{

bool operator ! (std::timespec const& ts) noexcept;
bool operator == (std::timespec const& lhs, std::timespec const& rhs) noexcept;
bool operator != (std::timespec const& lhs, std::timespec const& rhs) noexcept;

bool operator < (std::timespec const& lhs, std::timespec const& rhs) noexcept;
bool operator > (std::timespec const& lhs, std::timespec const& rhs) noexcept;

bool operator <= (std::timespec const& lhs, std::timespec const& rhs) noexcept;
bool operator >= (std::timespec const& lhs, std::timespec const& rhs) noexcept;

struct Duration
{
    std::timespec timespec;

    Duration() noexcept;
    explicit Duration(std::timespec const& ts) noexcept;
    Duration(std::time_t secs, long nsecs) noexcept;
    Duration(double secs) noexcept;

    virtual ~Duration() = default;

    operator double () const noexcept;

    Duration operator + (Duration const& rhs) const noexcept;
    Duration operator - (Duration const& rhs) const noexcept;

    Duration& operator += (Duration const& rhs) noexcept;
    Duration& operator -= (Duration const& rhs) noexcept;

    Duration operator * (double rhs) const noexcept; 
    Duration operator / (double rhs) const;

    Duration& operator *= (double rhs) noexcept;
    Duration& operator /= (double rhs);
};

struct Seconds : Duration
{
    static constexpr double Ratio{ 1.0 };

    Seconds(double secs);
};

struct MilliSeconds : Duration
{
    static constexpr double Ratio{ 1000.0 };

    MilliSeconds(double msecs);
};

struct MicroSeconds : Duration
{
    static constexpr double Ratio{ 1000000.0 };

    MicroSeconds(double usecs);
};

struct NanoSeconds : Duration
{
    static constexpr double Ratio{ 1000000000.0 };

    NanoSeconds(double nsecs);
};

template<typename T>
typename std::enable_if<std::is_base_of<Duration, T>::value, double>::type
to(Duration const& duration)
{
    return static_cast<double>(duration) * T::Ratio;
}

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
};

Clock now(clockid_t clock_id = CLOCK_MONOTONIC);

} // namespace hlib

