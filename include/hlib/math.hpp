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

template<typename T>
const T& clamp(T const& value, T const& min, T const& max)
{
    return std::max(std::min(value, max), min);
}

template<typename N = int, typename D = N>
class Fraction final
{
public:
    typedef N Num;
    typedef D Denom;

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
    to() const noexcept
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

template<typename T, typename N, typename D>
T fraction_to(Fraction<N, D> const& fraction) noexcept
{
    return fraction.template to<T>();
}

typedef std::ratio<1, 1> One;


template<typename T>
struct IsRatio : std::false_type {};

template<intmax_t Num, intmax_t Den>
struct IsRatio<std::ratio<Num, Den>> : std::true_type {};

template<typename R = One, typename T = int>
struct RatioValue final
{
    typedef R Ratio;
    typedef T Type;

    constexpr RatioValue() = default;

    explicit constexpr RatioValue(T value)
        : m_value{ value }
    {
    }

    Type const& operator *() const noexcept
    {
        return m_value;
    }

    Type& operator *()
    {
        return m_value;
    }

    bool operator !() const noexcept
    {
        return 0 != m_value;
    }

    bool operator == (RatioValue const& that) const noexcept
    {
        return that.m_value == m_value;
    }

    bool operator != (RatioValue const& that) const noexcept
    {
        return that.m_value != m_value;
    }

    bool operator <(RatioValue const& that) const noexcept
    {
        return m_value < that.m_value;
    }

    bool operator <=(RatioValue const& that) const noexcept
    {
        return m_value <= that.m_value;
    }

    bool operator >(RatioValue const& that) const noexcept
    {
        return m_value >= that.m_value;
    }

    bool operator >=(RatioValue const& that) const noexcept
    {
        return m_value >= that.m_value;
    }

    RatioValue operator +() const noexcept
    {
        return RatioValue(+m_value);
    }

    RatioValue operator -() const noexcept
    {
        return RatioValue(-m_value);
    }

    RatioValue operator +(RatioValue const& that) const noexcept
    {
        return RatioValue(m_value + that.m_value);
    }

    RatioValue operator -(RatioValue const& that) const noexcept
    {
        return RatioValue(m_value - that.m_value);
    }

    T operator /(RatioValue const& that) const noexcept
    {
        return m_value / that.m_value;
    }

    RatioValue operator %(RatioValue const& that) const noexcept
    {
        return RatioValue(m_value % that.m_value);
    }

    RatioValue& operator ++() noexcept
    {
        return RatioValue(++m_value);
    }

    RatioValue operator ++(int) noexcept
    {
        return RatioValue(m_value++);
    }

    RatioValue& operator --() noexcept
    {
        return RatioValue(--m_value);
    }

    RatioValue operator --(int) noexcept
    {
        return RatioValue(m_value--);
    }

    RatioValue& operator +=(RatioValue const& that) noexcept
    {
        m_value += that.m_value;
        return *this;
    }

    RatioValue& operator -=(RatioValue const& that) noexcept
    {
        m_value -= that.m_value;
        return *this;
    }

    RatioValue& operator %=(RatioValue const& that) noexcept
    {
        m_value %= that.m_value;
        return *this;
    }

    RatioValue operator *(T that) const noexcept
    {
        return RatioValue(m_value * that);
    }

    RatioValue operator /(T that) const noexcept
    {
        assert(0 != that);
        return RatioValue(m_value / that);
    }

    RatioValue& operator *=(T that) noexcept
    {
        return *this = *this * that;
    }

    RatioValue& operator /=(T that) noexcept
    {
        return *this = *this / that;
    }

    template<typename Fraction>
    RatioValue operator *(Fraction const& that) const noexcept
    {
        assert(0 != that.d);
        return RatioValue((m_value * that.n) / that.d);
    }

    template<typename Fraction>
    RatioValue operator /(Fraction const& that) const noexcept
    {
        assert(0 != that.n);
        return RatioValue((m_value * that.d) / that.n);
    }

    template<typename Fraction>
    RatioValue& operator *=(Fraction const& that) const noexcept
    {
        return *this = *this * that;
    }

    template<typename Fraction>
    RatioValue& operator /=(Fraction const& that) const noexcept
    {
        return *this = *this / that;
    }

    T value() const noexcept
    {
        return m_value;
    }

    template<typename Ratio, typename Type = T>
    typename std::enable_if<IsRatio<Ratio>::value, RatioValue<Ratio, Type>>::type
    to() const noexcept
    {
        using Factor = std::ratio_divide<R, Ratio>;
        return RatioValue<Ratio, Type>(static_cast<Type>(m_value) * static_cast<Type>(Factor::num) / static_cast<Type>(Factor::den));
    }

    template<typename TRatioValue>
    typename std::enable_if<!IsRatio<TRatioValue>::value, TRatioValue>::type
    to() const noexcept
    {
        return to<typename TRatioValue::Ratio, typename TRatioValue::Type>();
    }

    template<typename TRatio, typename TType = T>
    explicit operator RatioValue<TRatio, TType>() const noexcept
    {
        return to<TRatio, TType>();
    }

private:
    T m_value{ 0 };
};

} // namespace math
} // namespace hlib

