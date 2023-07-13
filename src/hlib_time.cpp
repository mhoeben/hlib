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
#include "hlib/time.hpp"
#include "hlib/base.hpp"

using namespace hlib;

namespace {

constexpr long SecsPerSec{ 1L };
constexpr long MSecsPerSec{ 1000L };
constexpr long USecsPerSec{ 1000000L };
constexpr long NSecsPerSec{ 1000000000L };

std::timespec timespec_add(std::timespec const& lhs, std::timespec const& rhs) noexcept
{
    std::timespec result
    {
        lhs.tv_sec + rhs.tv_sec,
        lhs.tv_nsec + rhs.tv_nsec
    };

    if (result.tv_nsec >= NSecsPerSec) {
        result.tv_sec += 1;
        result.tv_nsec -= NSecsPerSec;
    }

    return result;
}

std::timespec timespec_subtract(std::timespec const& lhs, std::timespec const& rhs) noexcept
{
    std::timespec result
    {
        lhs.tv_sec - rhs.tv_sec,
        lhs.tv_nsec - rhs.tv_nsec
    };

    if (result.tv_nsec < 0) {
        result.tv_sec -= 1;
        result.tv_nsec += NSecsPerSec;
    }

    return result;
}

std::timespec timespec_multiply(const std::timespec& lhs, double rhs) noexcept
{
    std::timespec result;

    double const nsec = lhs.tv_nsec * rhs;
    double const carry = std::floor(nsec / NSecsPerSec);

    result.tv_sec = lhs.tv_sec * static_cast<std::time_t>(rhs) + static_cast<std::time_t>(carry);
    result.tv_nsec = static_cast<long>(nsec - carry * NSecsPerSec);

    return result;
}

std::timespec timespec_divide(const timespec& lhs, double rhs)
{
    std::timespec result;

    double total_secs = lhs.tv_sec + lhs.tv_nsec / NSecsPerSec;
    double divided_secs = total_secs / rhs;

    result.tv_sec = static_cast<std::time_t>(divided_secs);
    result.tv_nsec = static_cast<long>((divided_secs - result.tv_sec) * NSecsPerSec);

    return result;
}

} // namespace

//
// Public (std::timespec)
//
bool hlib::operator !(std::timespec const& ts) noexcept
{
    return 0 == ts.tv_sec && 0 == ts.tv_nsec;
}

bool hlib::operator ==(std::timespec const& lhs, std::timespec const& rhs) noexcept
{
    return lhs.tv_sec == rhs.tv_sec && lhs.tv_nsec == rhs.tv_nsec;
}

bool hlib::operator !=(std::timespec const& lhs, std::timespec const& rhs) noexcept
{
    return !(lhs == rhs);
}

bool hlib::operator < (std::timespec const& lhs, std::timespec const& rhs) noexcept
{
    return lhs.tv_sec < rhs.tv_sec
        || (lhs.tv_sec == rhs.tv_sec && lhs.tv_nsec < rhs.tv_nsec);
}

bool hlib::operator > (std::timespec const& lhs, std::timespec const& rhs) noexcept
{
    return lhs.tv_sec > rhs.tv_sec
        || (lhs.tv_sec == rhs.tv_sec && lhs.tv_nsec > rhs.tv_nsec);
}

bool hlib::operator <= (std::timespec const& lhs, std::timespec const& rhs) noexcept
{
    return !(lhs > rhs);
}

bool hlib::operator >= (std::timespec const& lhs, std::timespec const& rhs) noexcept
{
    return !(lhs < rhs);
}

//
// Public (Duration)
//
Duration::Duration() noexcept
    : timespec{ 0, 0 }
{
}

Duration::Duration(std::timespec const& ts) noexcept
    : timespec{ ts }
{
}

Duration::Duration(std::time_t secs, long nsecs) noexcept
    : timespec{ secs, nsecs }
{
}

Duration::Duration(double secs) noexcept
{
    assert(secs >= 0.0);

    timespec.tv_sec = static_cast<time_t>(std::floor(secs));
    timespec.tv_nsec = static_cast<long>(std::fmod(secs, 1.0) * NSecsPerSec);
}

Duration::operator double () const noexcept
{
    return static_cast<double>(timespec.tv_sec)
         + static_cast<double>(timespec.tv_nsec) / static_cast<double>(NSecsPerSec);
}

Duration Duration::operator + (Duration const& rhs) const noexcept
{
    return Duration(timespec_add(timespec, rhs.timespec));
}

Duration Duration::operator - (Duration const& rhs) const noexcept
{
    return Duration(timespec_subtract(timespec, rhs.timespec));
}

Duration& Duration::operator += (Duration const& rhs) noexcept
{
    timespec = timespec_add(timespec, rhs.timespec);
    return *this;
}

Duration& Duration::operator -= (Duration const& rhs) noexcept
{
    timespec = timespec_subtract(timespec, rhs.timespec);
    return *this;
}

Duration Duration::operator * (double rhs) const noexcept
{
    return Duration(timespec_multiply(timespec, rhs));
}

Duration Duration::operator / (double rhs) const
{
    return Duration(timespec_divide(timespec, rhs));
}

Duration& Duration::operator *= (double rhs) noexcept
{
    timespec = timespec_multiply(timespec, rhs);
    return *this;
}

Duration& Duration::operator /= (double rhs)
{
    timespec = timespec_divide(timespec, rhs);
    return *this;
}

//
// Public (Seconds)
//
Seconds::Seconds(double secs)
    : Duration{ secs }
{
}

//
// Public (MilliSeconds)
//
MilliSeconds::MilliSeconds(double msecs)
    : Duration{ msecs * Ratio }
{
}

//
// Public (MicroSeconds)
//
MicroSeconds::MicroSeconds(double usecs)
    : Duration{ usecs * Ratio }
{
}

//
// Public (NanoSeconds)
//
NanoSeconds::NanoSeconds(double nsecs)
    : Duration{ nsecs * Ratio }
{
}

//
// Public (Clock)
//
Clock::Clock() noexcept
{
    tv_sec = 0;
    tv_nsec = 0;
}

Clock::Clock(clockid_t clock_id) noexcept
{
    hverify(0 == clock_gettime(clock_id, static_cast<std::timespec*>(this)));
}

Clock::Clock(std::timespec const& ts) noexcept
{
    tv_sec = ts.tv_sec;
    tv_nsec = ts.tv_nsec;
}

Duration Clock::operator - (Clock const& rhs) const noexcept
{
    return Duration(timespec_subtract(*this, rhs));
}

Clock Clock::operator + (Duration const& rhs) const noexcept
{
    return Clock(timespec_add(*this, rhs.timespec));
}

Clock Clock::operator - (Duration const& rhs) const noexcept
{
    return Clock(timespec_subtract(*this, rhs.timespec));
}

Clock& Clock::operator += (Duration const& rhs) noexcept
{
    Clock result(timespec_add(*this, rhs.timespec));
    tv_sec = result.tv_sec;
    tv_nsec = result.tv_nsec;
    return *this;
}

Clock& Clock::operator -= (Duration const& rhs) noexcept
{
    Clock result(timespec_subtract(*this, rhs.timespec));
    tv_sec = result.tv_sec;
    tv_nsec = result.tv_nsec;
    return *this;
}

//
// Public
//
Clock hlib::now(clockid_t clock_id)
{
    return Clock(clock_id);
}

