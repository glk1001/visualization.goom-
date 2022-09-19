#include "catch2/catch.hpp"
#include "utils/math/misc.h"

#include <cstdint>

namespace GOOM::UNIT_TESTS
{

using UTILS::MATH::FloatToIrreducibleFraction;
using UTILS::MATH::Gcd;
using UTILS::MATH::Lcm;
using UTILS::MATH::PowerOf2;
using UTILS::MATH::RationalNumber;

TEST_CASE("Gcd")
{
  REQUIRE(Gcd(8, 12) == 4);
  REQUIRE(Gcd(12, 8) == 4);

  REQUIRE(Gcd(54, 24) == 6);
  REQUIRE(Gcd(24, 54) == 6);
}

TEST_CASE("Lcm")
{
  REQUIRE(Lcm(4, 6) == 12);
  REQUIRE(Lcm(6, 4) == 12);

  REQUIRE(Lcm(21, 6) == 42);
  REQUIRE(Lcm(6, 21) == 42);
}

TEST_CASE("PowerOf2")
{
  REQUIRE(PowerOf2(0) == 1);
  REQUIRE(PowerOf2(0U) == 1U);

  REQUIRE(PowerOf2(1U) == 2U);
  REQUIRE(PowerOf2(1U) == 2U);

  REQUIRE(PowerOf2(4) == 16);
  REQUIRE(PowerOf2(4U) == 16U);

  REQUIRE(PowerOf2(16) == 65536);
  REQUIRE(PowerOf2(16U) == 65536U);
}

TEST_CASE("Frac")
{
  RationalNumber frac = FloatToIrreducibleFraction(4.5F);
  REQUIRE(frac.numerator == 9);
  REQUIRE(frac.denominator == 2);
  REQUIRE(frac.isRational == true);

  frac = FloatToIrreducibleFraction(0.25F);
  REQUIRE(frac.numerator == 1);
  REQUIRE(frac.denominator == 4);
  REQUIRE(frac.isRational == true);

  frac = FloatToIrreducibleFraction(1.20F);
  REQUIRE(frac.numerator == 6);
  REQUIRE(frac.denominator == 5);
  REQUIRE(frac.isRational == true);

  frac = FloatToIrreducibleFraction(1.50F);
  REQUIRE(frac.numerator == 3);
  REQUIRE(frac.denominator == 2);
  REQUIRE(frac.isRational == true);

  frac = FloatToIrreducibleFraction(3.0F);
  REQUIRE(frac.numerator == 3);
  REQUIRE(frac.denominator == 1);
  REQUIRE(frac.isRational == true);

  frac = FloatToIrreducibleFraction(6.0F / 7.0F);
  REQUIRE(frac.isRational == false);
}

} // namespace GOOM::UNIT_TESTS
