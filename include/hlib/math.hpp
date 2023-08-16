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

namespace hlib
{
namespace math
{

template<typename N = int, typename D = N>
struct Fraction
{
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

    bool operator<(Fraction const& that) const noexcept
    {
        return n * that.d < that.n * d;
    }

    bool operator<=(Fraction const& that) const noexcept
    {
        return n * that.d <= that.n * d;
    }

    bool operator>(Fraction const& that) const noexcept
    {
        return n * that.d > that.n * d;
    }

    bool operator>=(Fraction const& that) const noexcept
    {
        return n * that.d >= that.n * d;
    }
};

namespace detail
{

Fraction<int64_t> to_fraction(double value, double tolerance) noexcept;

} // namespace detail

template<typename F, typename T = double>
F to(T value, T tolerance = 1.0E-6) noexcept
{
    Fraction<int64_t> const f = detail::to_fraction(value, tolerance);
    return F(
        static_cast<typename F::Numerator>(f.n),
        static_cast<typename F::Denominator>(f.d)
    );
}

template<typename T, typename N = int, typename D = N>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
to(Fraction<N, D> const& fraction) noexcept
{
    return static_cast<T>(fraction.n) / static_cast<T>(fraction.d);
}

template<typename T, typename N = int, typename D = N>
typename std::enable_if<!std::is_same<bool, T>::value
                     && std::is_integral<T>::value, T>::type
to(Fraction<N, D> const& fraction) noexcept
{
    return static_cast<T>(std::round<>(to<double>(fraction)));
}

} // namespace math
} // namespace hlib

