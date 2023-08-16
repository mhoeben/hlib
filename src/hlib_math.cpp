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
#include "hlib/math.hpp"

using namespace hlib;

//
// Implementation
//
math::Fraction<int64_t> math::detail::to_fraction(double value, double tolerance) noexcept
{
    int64_t sign = (value < 0) ? -1 : 1;
    value = std::abs(value);

    int64_t integral = static_cast<int64_t>(value);
    value -= integral;

    if (value < tolerance) {
        return Fraction<int64_t>(integral * sign, 1);
    }

    int64_t a = integral;
    int64_t h1 = 1;
    int64_t h2 = 0;
    int64_t k1 = 0;
    int64_t k2 = 1;

    do {
        int64_t h = a * h1 + h2;
        int64_t k = a * k1 + k2;

        if (std::abs(value - static_cast<double>(h) / k) < tolerance) {
            int64_t common_factor = std::gcd((integral * k + h) * sign, k);
            return Fraction<int64_t>(
                ((integral * k + h) * sign) / common_factor,
                k / common_factor
            );
        }

        a = static_cast<int64_t>(1.0 / (value - static_cast<double>(h) / k));
        h2 = h1;
        h1 = h;
        k2 = k1;
        k1 = k;
    }
    while (true);
}

