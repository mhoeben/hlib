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
    REQUIRE(true == essentially_equal(1.0, Fraction<>(1, 1).to<double>()));
    REQUIRE(true == essentially_equal(0.5, Fraction<>(1, 2).to<double>()));

    // TODO
}

TEST_CASE("Fraction to Integer", "[math]")
{
    REQUIRE(1 == Fraction<>(1, 1).to<int>());
    REQUIRE(2 == Fraction<>(2, 1).to<int>());
    REQUIRE(3 == Fraction<>(6, 2).to<int>());

    REQUIRE(-1 == Fraction<>(-1,  1).to<int>());
    REQUIRE(-1 == Fraction<>( 2, -2).to<int>());
    REQUIRE( 1 == Fraction<>(-3, -3).to<int>());

    REQUIRE(0 == Fraction(1, 3).to<int>());
    REQUIRE(1 == Fraction(1, 2).to<int>());
    REQUIRE(0 == Fraction(-1, 3).to<int>());
    REQUIRE(-1 == Fraction(-2, 4).to<int>());
}

TEST_CASE("Double to Fraction", "[math]")
{
    REQUIRE(Fraction<>(1, 1) == to_fraction<>(1.0));

    // TODO
}

TEST_CASE("RatioValue", "[math]")
{
    RatioValue<std::deci> value(1);

    REQUIRE(100 == value.to<std::milli>().value);

    typedef RatioValue<std::milli> MilliRatio;
    REQUIRE(100 == value.to<MilliRatio>().value);
}

TEST_CASE("RatioValue and Fraction", "[math]")
{
    REQUIRE(40 == (RatioValue<>(60) * Fraction<>(2, 3)).value);
    REQUIRE(60 == (RatioValue<>(40) / Fraction<>(2, 3)).value);
}
