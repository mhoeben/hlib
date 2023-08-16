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
#include "test.hpp"
#include "hlib/math.hpp"

using namespace hlib;
using namespace hlib::math;

TEST_CASE("Fraction", "[math]")
{
    REQUIRE(Fraction<>(1, 1) == Fraction<>(1, 1));
    REQUIRE(Fraction<>(2, 2) == Fraction<>(1, 1));
    REQUIRE(Fraction<>(2, 2) == Fraction<>(-1, -1));

    // TODO
}

TEST_CASE("Fraction to Double", "[math]")
{
    REQUIRE(true == essentially_equal(1.0, to<double>(Fraction<>(1, 1))));
    REQUIRE(true == essentially_equal(0.5, to<double>(Fraction<>(1, 2))));

    // TODO
}

TEST_CASE("Fraction to Integer", "[math]")
{
    REQUIRE(1 == to<int>(Fraction<>(1, 1)));
    REQUIRE(2 == to<int>(Fraction<>(2, 1)));
    REQUIRE(3 == to<int>(Fraction<>(6, 2)));

    REQUIRE(-1 == to<int>(Fraction<>(-1,  1)));
    REQUIRE(-1 == to<int>(Fraction<>( 2, -2)));
    REQUIRE( 1 == to<int>(Fraction<>(-3, -3)));

    REQUIRE(0 == to<int>(Fraction(1, 3)));
    REQUIRE(1 == to<int>(Fraction(1, 2)));
    REQUIRE(0 == to<int>(Fraction(-1, 3)));
    REQUIRE(-1 == to<int>(Fraction(-2, 4)));
}

TEST_CASE("Double to Fraction", "[math]")
{
    REQUIRE(Fraction<>(1, 1) == to<Fraction<>>(1.0));

    // TODO
}

