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
#include "hlib/format.hpp"
#include "hlib/error.hpp"

using namespace hlib;

namespace
{

constexpr long nsec_per_sec{ 1000000000L };

std::timespec timespec_add(std::timespec const& lhs, std::timespec const& that) noexcept
{
    std::timespec result
    {
        lhs.tv_sec + that.tv_sec,
        lhs.tv_nsec + that.tv_nsec
    };

    if (result.tv_nsec >= nsec_per_sec) {
        result.tv_sec += 1;
        result.tv_nsec -= nsec_per_sec;
    }

    return result;
}

std::timespec timespec_subtract(std::timespec const& lhs, std::timespec const& that) noexcept
{
    std::timespec result
    {
        lhs.tv_sec - that.tv_sec,
        lhs.tv_nsec - that.tv_nsec
    };

    if (result.tv_nsec < 0) {
        result.tv_sec -= 1;
        result.tv_nsec += nsec_per_sec;
    }

    return result;
}

std::timespec timespec_multiply(const std::timespec& lhs, double that) noexcept
{
    std::timespec result;

    double const nsec = lhs.tv_nsec * that;
    double const carry = std::floor(nsec / nsec_per_sec);

    result.tv_sec = lhs.tv_sec * static_cast<std::time_t>(that) + static_cast<std::time_t>(carry);
    result.tv_nsec = static_cast<long>(nsec - carry * nsec_per_sec);

    return result;
}

std::timespec timespec_divide(const timespec& lhs, double that)
{
    std::timespec result;

    double total_secs = lhs.tv_sec + lhs.tv_nsec / nsec_per_sec;
    double divided_secs = total_secs / that;

    result.tv_sec = static_cast<std::time_t>(divided_secs);
    result.tv_nsec = static_cast<long>((divided_secs - result.tv_sec) * nsec_per_sec);

    return result;
}

} // namespace

//
// Public (Duration)
//
time::Duration::Duration() noexcept
    : std::timespec{ 0, 0 }
{
}

time::Duration::Duration(std::timespec const& ts) noexcept
    : std::timespec{ ts }
{
}

time::Duration::Duration(std::time_t secs, long nsecs) noexcept
    : std::timespec{ secs, nsecs }
{
}

time::Duration::Duration(double secs) noexcept
{
    assert(secs >= 0.0);

    tv_sec = static_cast<time_t>(std::floor(secs));
    tv_nsec = static_cast<long>(std::fmod(secs, 1.0) * nsec_per_sec);
}

time::Duration time::Duration::operator +(time::Duration const& that) const noexcept
{
    return time::Duration(timespec_add(*this, that));
}

time::Duration time::Duration::operator -(time::Duration const& that) const noexcept
{
    return time::Duration(timespec_subtract(*this, that));
}

time::Duration& time::Duration::operator +=(time::Duration const& that) noexcept
{
    static_cast<timespec&>(*this) = timespec_add(*this, that);
    return *this;
}

time::Duration& time::Duration::operator -=(time::Duration const& that) noexcept
{
    static_cast<timespec&>(*this) = timespec_subtract(*this, that);
    return *this;
}

time::Duration time::Duration::operator *(double that) const noexcept
{
    return time::Duration(timespec_multiply(*this, that));
}

time::Duration time::Duration::operator /(double that) const
{
    return time::Duration(timespec_divide(*this, that));
}

time::Duration& time::Duration::operator *=(double that) noexcept
{
    static_cast<timespec&>(*this) = timespec_multiply(*this, that);
    return *this;
}

time::Duration& time::Duration::operator /=(double that)
{
    static_cast<timespec&>(*this) = timespec_divide(*this, that);
    return *this;
}

bool time::Duration::operator !() const noexcept
{
    return 0 == tv_sec && 0 == tv_nsec;
}

bool time::Duration::operator ==(Duration const& that) const noexcept
{
    return tv_sec == that.tv_sec && tv_nsec == that.tv_nsec;
}

bool time::Duration::operator !=(Duration const& that) const noexcept
{
    return !(*this == that);
}

bool time::Duration::operator <(Duration const& that) const noexcept
{
    return tv_sec < that.tv_sec
        || (tv_sec == that.tv_sec && tv_nsec < that.tv_nsec);
}

bool time::Duration::operator >(Duration const& that) const noexcept
{
    return tv_sec > that.tv_sec
        || (tv_sec == that.tv_sec && tv_nsec > that.tv_nsec);
}

bool time::Duration::operator <=(Duration const& that) const noexcept
{
    return !(*this > that);
}

bool time::Duration::operator >=(Duration const& that) const noexcept
{
    return !(*this < that);
}

//
// Public (Clock)
//
time::Clock::Clock() noexcept
{
    tv_sec = 0;
    tv_nsec = 0;
}

time::Clock::Clock(std::timespec const& ts) noexcept
{
    tv_sec = ts.tv_sec;
    tv_nsec = ts.tv_nsec;
}

time::Clock::Clock(std::time_t sec, long nsec) noexcept
{
    tv_sec = sec;
    tv_nsec = nsec;
}

time::Clock::Clock(clockid_t clock_id, std::nothrow_t) noexcept
{
    hverify(0 == clock_gettime(clock_id, static_cast<std::timespec*>(this)));
}

time::Clock::Clock(clockid_t clock_id)
{
    if (-1 == clock_gettime(clock_id, static_cast<std::timespec*>(this))) {
        throwf<std::runtime_error>("clock_gettime failed ({})", get_error_string());
    }
}

time::Duration time::Clock::operator -(time::Clock const& that) const noexcept
{
    return time::Duration(timespec_subtract(*this, that));
}

time::Clock time::Clock::operator +(time::Duration const& that) const noexcept
{
    return time::Clock(timespec_add(*this, that));
}

time::Clock time::Clock::operator - (time::Duration const& that) const noexcept
{
    return time::Clock(timespec_subtract(*this, that));
}

time::Clock& time::Clock::operator +=(time::Duration const& that) noexcept
{
    Clock result(timespec_add(*this, that));
    tv_sec = result.tv_sec;
    tv_nsec = result.tv_nsec;
    return *this;
}

time::Clock& time::Clock::operator -=(time::Duration const& that) noexcept
{
    Clock result(timespec_subtract(*this, that));
    tv_sec = result.tv_sec;
    tv_nsec = result.tv_nsec;
    return *this;
}

bool time::Clock::operator !() const noexcept
{
    return 0 == tv_sec && 0 == tv_nsec;
}

bool time::Clock::operator ==(std::timespec const& that) const noexcept
{
    return tv_sec == that.tv_sec && tv_nsec == that.tv_nsec;
}

bool time::Clock::operator !=(std::timespec const& that) const noexcept
{
    return !(*this == that);
}

bool time::Clock::operator <(std::timespec const& that) const noexcept
{
    return tv_sec < that.tv_sec
        || (tv_sec == that.tv_sec && tv_nsec < that.tv_nsec);
}

bool time::Clock::operator >(std::timespec const& that) const noexcept
{
    return tv_sec > that.tv_sec
        || (tv_sec == that.tv_sec && tv_nsec > that.tv_nsec);
}

bool time::Clock::operator <=(std::timespec const& that) const noexcept
{
    return !(*this > that);
}

bool time::Clock::operator >=(std::timespec const& that) const noexcept
{
    return !(*this < that);
}

//
// Public
//
time::Clock hlib::time::now(clockid_t clock_id)
{
    return Clock(clock_id);
}

std::string hlib::time::to_string(Duration duration)
{
    struct timespec const& ts = duration;

    return fmt::format("{:2d}:{:02d}:{:02d}.{:03d}",
        (ts.tv_sec / 3600),
        (ts.tv_sec / 60) % 60,
        (ts.tv_sec / 1) % 60,
        (ts.tv_nsec / 1000000)
    );
}

