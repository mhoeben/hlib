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
#include "hlib/error.hpp"
#include "hlib_format.hpp"
#include "hlib/string.hpp"

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

std::string localtime_offset_string(time::Clock const& clock)
{
    struct tm time_info;
    std::int32_t offset;
    char sign;

    // Get local time reentrant.
    localtime_r(&clock.tv_sec, &time_info);

    // Determine sign and absolute offset.
    if (time_info.tm_gmtoff >= 0) {
        sign = '+';
        offset = time_info.tm_gmtoff / 60;
    }
    else {
        sign = '-';
        offset = -(time_info.tm_gmtoff / 60);
    }

    return format("%c%02d:%02d", sign, offset / 60, offset % 60);
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
    HVERIFY(0 == clock_gettime(clock_id, static_cast<std::timespec*>(this)));
}

time::Clock::Clock(clockid_t clock_id)
{
    if (-1 == clock_gettime(clock_id, static_cast<std::timespec*>(this))) {
        throw make_system_error(errno, "clock_gettime() failed");
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
time::Clock time::now(clockid_t clock_id)
{
    return Clock(clock_id);
}

time::Clock time::now_utc(clockid_t clock_id)
{
    assert(
        CLOCK_REALTIME == clock_id ||
        CLOCK_REALTIME_ALARM == clock_id ||
        CLOCK_REALTIME_COARSE == clock_id
    );
    return Clock(clock_id);
}

time::Clock const time::infinity(0x7fffffff, 0);

time::Clock time::to_clock(std::string const& iso8601)
{
    std::tm tm_time{};
    std::timespec timespec;

    auto parse = [&](char const* format)
    {
        char* r = strptime(iso8601.c_str(), format, &tm_time);
        if (nullptr == r || 0 != *r) {
            throw std::logic_error("Invalid ISO8601 string");
        }
    };

    // Date only?
    if (false == contains(iso8601, 'T')) {
        parse("%Y-%m-%d");
        timespec.tv_sec = timegm(&tm_time);
        timespec.tv_nsec = 0;
    }
    // Without millisecond part?
    else if (false == contains(iso8601, '.')) {
        parse("%Y-%m-%dT%H:%M:%S%z");
        timespec.tv_sec = timegm(&tm_time);
        timespec.tv_nsec = 0;
    }
    else {
        char* r = strptime(iso8601.c_str(), "%Y-%m-%dT%H:%M:%S", &tm_time);
        if (nullptr == r || '.' != *r) {
            throw std::logic_error("Invalid ISO8601 string");
        }

        int milliseconds;
        if (1 != sscanf(r, ".%03d", &milliseconds)) {
            throw std::logic_error("Invalid ISO8601 string");
        }

        r = strptime(r + 4, "%z", &tm_time);
        if (nullptr == r || 0 != *r) {
            throw std::logic_error("Invalid ISO8601 string");
        }

        timespec.tv_sec = timegm(&tm_time);
        timespec.tv_nsec = milliseconds * 1000000;
    }

    return Clock(timespec);
}

std::string hlib::to_string(time::Duration const& duration, bool milliseconds)
{
    std::string string = format("%02ld:%02ld:%02ld",
        (duration.tv_sec / 3600),
        (duration.tv_sec / 60) % 60,
        (duration.tv_sec / 1) % 60
    );

    if (true == milliseconds) {
        format_to(string, ".%03ld", duration.tv_nsec / 1000000);
    }

    return string;
}

std::string hlib::to_string_utc_date(time::Clock const& clock)
{
    char string[12]; // ISO 8601 format: "YYYY-MM-DDZ\0"
    strftime(string, sizeof(string), "%Y-%m-%dZ", gmtime(&(clock.tv_sec)));
    return string;
}

std::string hlib::to_string_utc_date_and_time(time::Clock const& clock)
{
    char string[21]; // ISO 8601 format: "YYYY-MM-DDTHH:MM:SSZ\0"
    strftime(string, sizeof(string), "%Y-%m-%dT%H:%M:%SZ", gmtime(&(clock.tv_sec)));
    return string;
}

std::string hlib::to_string_utc_time(time::Clock const& clock)
{
    char string[10]; // ISO 8601 format: "HH:MM:SSZ\0"
    strftime(string, sizeof(string), "%H:%M:%SZ", gmtime(&(clock.tv_sec)));
    return string;
}

std::string hlib::to_string_utc_time_milliseconds(time::Clock const& clock)
{
    char string[14]; // ISO 8601 format: "HH:MM:SS[.MMM]Z\0"
    constexpr std::size_t millisecond_offset = 8;

    strftime(string, sizeof(string), "%H:%M:%S", gmtime(&(clock.tv_sec)));

    // Satisfy the compiler's buffer overflow checker by limiting the argument to a range of 0 - 999.
    const std::uint32_t ms = (clock.tv_nsec / 1000000) % 1000;
    snprintf(string + millisecond_offset, sizeof(string) - millisecond_offset, ".%03dZ", ms);
    return string;
}

std::string hlib::to_string_utc(time::Clock const& clock, bool milliseconds)
{
    char string[25]; // ISO 8601 format: "YYYY-MM-DDTHH:MM:SS[.MMM]Z\0"
    constexpr std::size_t millisecond_offset = 19;

    strftime(string, sizeof(string), "%Y-%m-%dT%H:%M:%SZ", gmtime(&(clock.tv_sec)));

    if (true == milliseconds) {
        // Satisfy the compiler's buffer overflow checker by limiting the argument to a range of 0 - 999.
        const std::uint32_t ms = (clock.tv_nsec / 1000000) % 1000;
        snprintf(string + millisecond_offset, sizeof(string) - millisecond_offset, ".%03dZ", ms);
    }

    return string;
}

std::string hlib::to_string_local_date(time::Clock const& clock)
{
    char string[11]; // ISO 8601 format: "YYYY-MM-DD\0"
    strftime(string, sizeof(string), "%Y-%m-%d", localtime(&(clock.tv_sec)));
    return string + localtime_offset_string(clock);
}

std::string hlib::to_string_local_date_and_time(time::Clock const& clock)
{
    char string[20]; // ISO 8601 format: "YYYY-MM-DDTHH:MM:SS\0"
    strftime(string, sizeof(string), "%Y-%m-%dT%H:%M:%S", localtime(&(clock.tv_sec)));
    return string + localtime_offset_string(clock);
}

std::string hlib::to_string_local_time(time::Clock const& clock)
{
    char string[9]; // ISO 8601 format: "HH:MM:SS\0"
    strftime(string, sizeof(string), "%H:%M:%S", localtime(&(clock.tv_sec)));
    return string + localtime_offset_string(clock);
}

std::string hlib::to_string_local_time_milliseconds(time::Clock const& clock)
{
    char string[14]; // ISO 8601 format: "HH:MM:SS[.MMM]\0"
    constexpr std::size_t millisecond_offset = 8;

    strftime(string, sizeof(string), "%H:%M:%S", gmtime(&(clock.tv_sec)));

    // Satisfy the compiler's buffer overflow checker by limiting the argument to a range of 0 - 999.
    const std::uint32_t ms = (clock.tv_nsec / 1000000) % 1000;
    snprintf(string + millisecond_offset, sizeof(string) - millisecond_offset, ".%03d", ms);
    return string + localtime_offset_string(clock);
}

std::string hlib::to_string_local(time::Clock const& clock, bool milliseconds)
{
    char string[24]; // ISO 8601 format: "YYYY-MM-DDTHH:MM:SS[.MMM]\0"
    constexpr std::size_t millisecond_offset = 19;

    strftime(string, sizeof(string), "%Y-%m-%dT%H:%M:%S", gmtime(&(clock.tv_sec)));

    if (true == milliseconds) {
        // Satisfy the compiler's buffer overflow checker by limiting the argument to a range of 0 - 999.
        const std::uint32_t ms = (clock.tv_nsec / 1000000) % 1000;
        snprintf(string + millisecond_offset, sizeof(string) - millisecond_offset, ".%03d", ms);
    }

    return string + localtime_offset_string(clock);
}

