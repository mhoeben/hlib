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
#include <cmath>
#include <numeric>
#include <ratio>

namespace hlib
{
namespace math
{

// Knuth: approximately_equal gives whether the difference between a and b is
// smaller than the acceptable error (epsilon), determined by the larger of a or b.
template<typename T>
bool approximately_equal(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
{
    return std::abs(a - b) <= ( (std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}

// Knuth: essentially_equal gives whether the difference between a and b is
// smaller than the acceptable error (epsilon), determined by the smaller of a or b. 
template<typename T>
bool essentially_equal(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
{
    return std::abs(a - b) <= ( (std::abs(a) > std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}

template<typename T>
bool definitely_greate_than(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
{
    return (a - b) > ( (std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}

template<typename T>
bool definitely_less_than(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
{
    return (b - a) > ( (std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
}

template<typename N = int, typename D = N>
class Fraction final
{
public:
    typedef N Numerator;
    typedef D Denominator;

    N n{ 0 };
    D d{ 0 };

    constexpr Fraction() = default;

    constexpr Fraction(N numerator, D denominator = 1) noexcept
        : n{ numerator }
        , d{ denominator }
    {
    }

    bool operator !() const noexcept
    {
        return 0 == n;
    }

    bool operator == (Fraction const& that) const noexcept
    {
        return n * that.d == that.n * d;
    }

    bool operator != (Fraction const& that) const noexcept
    {
        return n * that.d != that.n * d;
    }

    bool operator <(Fraction const& that) const noexcept
    {
        return n * that.d < that.n * d;
    }

    bool operator <=(Fraction const& that) const noexcept
    {
        return n * that.d <= that.n * d;
    }

    bool operator >(Fraction const& that) const noexcept
    {
        return n * that.d > that.n * d;
    }

    bool operator >=(Fraction const& that) const noexcept
    {
        return n * that.d >= that.n * d;
    }

    template<typename T>
    typename std::enable_if<std::is_floating_point<T>::value, T>::type
    to() const noexcept
    {
        return static_cast<T>(n) / static_cast<T>(d);
    }

    template<typename T>
    typename std::enable_if<!std::is_same<bool, T>::value
                         && std::is_integral<T>::value, T>::type
    to() const
    {
        return static_cast<T>(std::round(to<double>()));
    }
};

namespace detail
{

void to_fraction(int64_t& num, int64_t& den, double value, double tolerance) noexcept;

} // namespace detail

template<typename N = int, typename D = N>
Fraction<N, D> to_fraction(double value, double tolerance = 1.0E-6) noexcept
{
    int64_t num, den;
    detail::to_fraction(num, den, value, tolerance);
    return Fraction<N, D>(static_cast<N>(num), static_cast<D>(den));
}

template<typename R = std::ratio<1, 1>, typename T = int>
struct RatioValue final
{
    typedef R Ratio;
    typedef T Type;

    T value{ 0 };

    constexpr RatioValue() = default;

    constexpr RatioValue(T a_value)
        : value{ a_value }
    {
    }

    Type const& operator *() const noexcept
    {
        return value;
    }

    Type& operator *()
    {
        return value;
    }

    bool operator !() const noexcept
    {
        return 0 != value;
    }

    bool operator == (RatioValue const& that) const noexcept
    {
        return that.value == value;
    }

    bool operator != (RatioValue const& that) const noexcept
    {
        return that.value != value;
    }

    bool operator <(RatioValue const& that) const noexcept
    {
        return value < that.value;
    }

    bool operator <=(RatioValue const& that) const noexcept
    {
        return value <= that.value;
    }

    bool operator >(RatioValue const& that) const noexcept
    {
        return value >= that.value;
    }

    bool operator >=(RatioValue const& that) const noexcept
    {
        return value >= that.value;
    }

    RatioValue operator +() const noexcept
    {
        return RatioValue(+value);
    }

    RatioValue operator -() const noexcept
    {
        return RatioValue(-value);
    }

    RatioValue operator +(RatioValue const& that) const noexcept
    {
        return RatioValue(value + that.value);
    }

    RatioValue operator -(RatioValue const& that) const noexcept
    {
        return RatioValue(value - that.value);
    }

    RatioValue operator *(RatioValue const& that) const noexcept
    {
        return RatioValue(value * that.value);
    }

    RatioValue operator /(RatioValue const& that) const
    {
        return RatioValue(value / that.value);
    }

    RatioValue operator %(RatioValue const& that) const
    {
        return RatioValue(value % that.value);
    }

    RatioValue& operator ++() noexcept
    {
        return RatioValue(++value);
    }

    RatioValue operator ++(int) noexcept
    {
        return RatioValue(value++);
    }

    RatioValue& operator --() noexcept
    {
        return RatioValue(--value);
    }

    RatioValue operator --(int) noexcept
    {
        return RatioValue(value--);
    }

    RatioValue& operator =(RatioValue const& that) noexcept
    {
        value = that.value;
        return *this;
    }

    RatioValue& operator +=(RatioValue const& that) noexcept
    {
        value += that.value;
        return *this;
    }

    RatioValue& operator -=(RatioValue const& that) noexcept
    {
        value -= that.value;
        return *this;
    }

    RatioValue& operator *=(RatioValue const& that) noexcept
    {
        value *= that.value;
        return *this;
    }

    RatioValue& operator /=(RatioValue const& that)
    {
        value /= that.value;
        return *this;
    }

    RatioValue& operator %=(RatioValue const& that)
    {
        value %= that.value;
        return *this;
    }

    template<typename TRatio, typename TType = T>
    RatioValue<TRatio, TType> to() const
    {
        using Factor = std::ratio_divide<R, TRatio>;
        return RatioValue<TRatio, TType>(static_cast<TType>(value) * static_cast<TType>(Factor::num) / static_cast<TType>(Factor::den));
    }

    template<typename TRatio, typename TType = T>
    operator RatioValue<TRatio, TType>() const
    {
        return to<TRatio, TType>();
    }

};

} // namespace math
} // namespace hlib

