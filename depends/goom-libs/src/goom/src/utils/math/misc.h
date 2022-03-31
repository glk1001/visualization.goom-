#pragma once

#include "math20.h"

#include <cassert>
#include <cmath>
#include <cstdint>

namespace GOOM::UTILS::MATH
{

static constexpr float TWO_PI = 2.0F * STD20::pi;
static constexpr float HALF_PI = 0.5F * STD20::pi;
static constexpr float THIRD_PI = STD20::pi / 3.0F;

static constexpr float DEGREES_90 = 90.0F;
static constexpr float DEGREES_180 = 180.0F;
static constexpr float DEGREES_360 = 360.0F;

template<typename T>
[[nodiscard]] constexpr auto Sq(const T& x) -> T
{
  return x * x;
}

[[nodiscard]] constexpr auto SqDistance(const float x, const float y) -> float
{
  return Sq(x) + Sq(y);
}

static constexpr float SMALL_FLOAT = 0.00001F;

[[nodiscard]] inline bool FloatsEqual(const float x,
                                      const float y,
                                      const float epsilon = SMALL_FLOAT)
{
  return std::fabs(x - y) < epsilon;
}

template<typename T>
[[nodiscard]] constexpr auto IsEven(const T& n) -> bool
{
  return 0 == (n % 2);
}

template<typename T>
[[nodiscard]] constexpr auto IsOdd(const T& n) -> bool
{
  return 0 != (n % 2);
}

template<typename T>
class Fraction
{
public:
  Fraction() noexcept = delete;

  constexpr Fraction(T numerator, T denominator) noexcept
    : m_numerator{numerator}, m_denominator{denominator}
  {
    assert(m_denominator != 0);
  }

private:
  T m_numerator;
  T m_denominator;

  template<typename U>
  friend constexpr auto operator-(U value, const Fraction<U>& frac) -> Fraction<U>;
  template<typename U>
  friend constexpr auto operator*(const Fraction<U>& frac, U value) -> U;
};

template<typename T>
static constexpr Fraction<T> FRAC_HALF = Fraction<T>{T(1), T(2)};
static constexpr auto I_HALF = FRAC_HALF<int32_t>;
static constexpr auto U_HALF = FRAC_HALF<uint32_t>;
static constexpr auto S_HALF = FRAC_HALF<size_t>;

template<typename T>
constexpr Fraction<T> QUARTER = Fraction<T>{T(1), T(4)};
static constexpr auto I_QUARTER = QUARTER<int32_t>;
static constexpr auto U_QUARTER = QUARTER<uint32_t>;

template<typename T>
constexpr Fraction<T> THREE_QUARTERS = Fraction<T>{T(3), T(4)};
static constexpr auto I_THREE_QUARTERS = THREE_QUARTERS<int32_t>;
static constexpr auto U_THREE_QUARTERS = THREE_QUARTERS<uint32_t>;

template<typename T>
[[nodiscard]] inline auto GetFltFraction(const T numerator, const T denominator) -> float
{
  return static_cast<float>(numerator) / static_cast<float>(denominator);
}
static constexpr float HALF = 1.0F / 2.0F;
static constexpr float THIRD = 1.0F / 3.0F;

class RangeMapper
{
public:
  constexpr RangeMapper() noexcept = default;
  constexpr RangeMapper(double x0, double x1) noexcept;
  constexpr auto operator()(double x0, double x1, double x) const -> double;
  [[nodiscard]] constexpr auto GetXMin() const -> double { return m_xMin; }
  [[nodiscard]] constexpr auto GetXMax() const -> double { return m_xMax; }

private:
  double m_xMin = 0;
  double m_xMax = 0;
  double m_xWidth = 0;
};

constexpr RangeMapper::RangeMapper(const double x0, const double x1) noexcept
  : m_xMin(x0), m_xMax(x1), m_xWidth(m_xMax - m_xMin)
{
}

constexpr auto RangeMapper::operator()(const double x0, const double x1, const double x) const
    -> double
{
  return STD20::lerp(x0, x1, (x - m_xMin) / m_xWidth);
}

template<typename T>
[[nodiscard]] constexpr auto operator-(const T value, const Fraction<T>& frac) -> Fraction<T>
{
  return {(value * frac.m_denominator) - frac.m_numerator, frac.m_denominator};
}

template<typename T>
[[nodiscard]] constexpr auto operator*(const Fraction<T>& frac, const T value) -> T
{
  return (frac.m_numerator * value) / frac.m_denominator;
}

} // namespace GOOM::UTILS::MATH