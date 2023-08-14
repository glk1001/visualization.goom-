#pragma once

#include "goom_config.h"
#include "math20.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numeric>

namespace GOOM::UTILS::MATH
{
inline constexpr auto HALF = 1.0F / 2.0F;

// NOLINTNEXTLINE: problem with 'readability-identifier-naming.ConstexprVariableIgnoredRegexp'
inline constexpr auto PI       = STD20::pi_v<float>;
inline constexpr auto TWO_PI   = 2.0F * PI;
inline constexpr auto HALF_PI  = 0.5F * PI;
inline constexpr auto THIRD_PI = PI / 3.0F;

inline constexpr auto DEGREES_90  = 90.0F;
inline constexpr auto DEGREES_180 = 180.0F;
inline constexpr auto DEGREES_360 = 360.0F;

[[nodiscard]] constexpr auto ToRadians(const float degrees) noexcept -> float
{
  return degrees * (TWO_PI / DEGREES_360);
}

[[nodiscard]] constexpr auto ToDegrees(const float radians) noexcept -> float
{
  return radians * (DEGREES_360 / TWO_PI);
}

[[nodiscard]] constexpr auto Lcm(const int32_t a, const int32_t b) noexcept -> int64_t
{
  return (static_cast<int64_t>(a) / std::gcd(a, b)) * static_cast<int64_t>(b);
}

struct RationalNumber
{
  int32_t numerator;
  int32_t denominator;
  bool isRational;
};

[[nodiscard]] inline auto FloatToIrreducibleFraction(const float fltVal) noexcept -> RationalNumber
{
  static constexpr auto PRECISION = 10000000.0;

  const auto dblVal  = static_cast<double>(fltVal);
  const auto intVal  = std::floor(dblVal);
  const auto fracVal = dblVal - intVal;

  const auto gcdVal = std::gcd(static_cast<int32_t>(std::lround(fracVal * PRECISION)),
                               static_cast<int32_t>(PRECISION));

  const auto numerator   = static_cast<int32_t>(std::lround(fracVal * PRECISION) / gcdVal);
  const auto denominator = static_cast<int32_t>(PRECISION / gcdVal);

  const auto isRational = denominator != static_cast<int32_t>(PRECISION);

  return {(static_cast<int32_t>(intVal) * denominator) + numerator, denominator, isRational};
}

template<typename T>
[[nodiscard]] constexpr auto Sq(const T& x) noexcept -> T
{
  return x * x;
}

template<typename T>
[[nodiscard]] constexpr auto SqDistanceFromZero(const T x, const T y) noexcept -> T
{
  return Sq(x) + Sq(y);
}

template<typename T>
[[nodiscard]] constexpr auto PowerOf2(const T n) noexcept -> T
{
  auto val = static_cast<T>(1);
  for (auto i = static_cast<T>(1); i <= n; ++i)
  {
    val *= 2;
  }
  return val;
}

template<typename T>
[[nodiscard]] constexpr auto Log2(const T n) noexcept -> T
{
  Expects(n > 0);

  auto log2 = static_cast<T>(0);
  auto m    = n;
  while (m >= static_cast<T>(2))
  {
    m = m >> 1U;
    ++log2;
  }
  Ensures(n == PowerOf2(log2));

  return log2;
}

template<typename T>
constexpr auto IntPower(T base, uint32_t exp) noexcept -> T
{
  auto result = static_cast<T>(1);

  while (true)
  {
    if (exp & 1U)
    {
      result *= base;
    }
    exp >>= 1U;
    if (not exp)
    {
      break;
    }
    base *= base;
  }

  return result;
}

[[nodiscard]] inline auto GetSawTooth(const float t, const float period) noexcept -> float
{
  const auto tDivPeriod = t / period;
  return HALF + (tDivPeriod - std::floor(HALF + tDivPeriod));
}

[[nodiscard]] inline auto GetTriangle(const float t, const float period) noexcept -> float
{
  const auto remainder = std::fmod(t, period);
  return t < (HALF * period) ? remainder : (period - remainder);
}

inline constexpr auto SMALL_FLOAT = 0.00001F;

[[nodiscard]] inline auto FloatsEqual(const float x,
                                      const float y,
                                      const float epsilon = SMALL_FLOAT) noexcept -> bool
{
  return std::fabs(x - y) < epsilon;
}

template<typename T>
[[nodiscard]] constexpr auto IsEven(const T& n) noexcept -> bool
{
  return 0 == (n % 2);
}

template<typename T>
[[nodiscard]] constexpr auto IsOdd(const T& n) noexcept -> bool
{
  return 0 != (n % 2);
}

template<typename T>
[[nodiscard]] constexpr auto IsBetween(const T& value, const T& low, const T& high) noexcept -> bool
{
  return (low <= value) and (value <= high);
}

template<typename T>
[[nodiscard]] constexpr auto UnorderedClamp(const T& val, const T& val1, const T& val2) noexcept
    -> T
{
  if (val1 > val2)
  {
    return std::clamp(val, val2, val1);
  }
  return std::clamp(val, val1, val2);
}

template<typename T>
[[nodiscard]] constexpr auto ModIncrement(T val, const T mod) noexcept -> T
{
  ++val;
  if (val < mod)
  {
    return val;
  }
  return 0;
}

template<typename T>
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
[[nodiscard]] constexpr auto ModDecrement(const T val, const T mod) noexcept -> T
{
  auto iVal = static_cast<int32_t>(val);
  --iVal;
  if (iVal >= 0)
  {
    return static_cast<T>(iVal);
  }
  return mod - 1;
}

template<typename T>
class Fraction
{
public:
  struct NumDom
  {
    T numerator;
    T denominator;
  };
  constexpr explicit Fraction(const NumDom& numDom) noexcept
    : m_numerator{numDom.numerator}, m_denominator{numDom.denominator}
  {
    Expects(numDom.denominator != static_cast<T>(0));
  }

private:
  T m_numerator;
  T m_denominator;

  template<typename U>
  friend constexpr auto operator-(U value, const Fraction<U>& frac) noexcept -> Fraction<U>;
  template<typename U>
  friend constexpr auto operator*(const Fraction<U>& frac, U value) noexcept -> U;
};

template<typename T>
inline constexpr auto FRAC_HALF = Fraction<T>{
    {static_cast<T>(1), static_cast<T>(2)}
};
inline constexpr auto I_HALF = FRAC_HALF<int32_t>;
inline constexpr auto U_HALF = FRAC_HALF<uint32_t>;
inline constexpr auto S_HALF = FRAC_HALF<size_t>;

template<typename T>
inline constexpr auto FRAC_QUARTER = Fraction<T>{
    {static_cast<T>(1), static_cast<T>(4)}
};
inline constexpr auto I_QUARTER = FRAC_QUARTER<int32_t>;
inline constexpr auto U_QUARTER = FRAC_QUARTER<uint32_t>;

template<typename T>
inline constexpr auto FRAC_THREE_QUARTERS = Fraction<T>{
    {static_cast<T>(3), static_cast<T>(4)}
};
inline constexpr auto I_THREE_QUARTERS = FRAC_THREE_QUARTERS<int32_t>;
inline constexpr auto U_THREE_QUARTERS = FRAC_THREE_QUARTERS<uint32_t>;

template<typename T>
inline constexpr auto FRAC_THIRD = Fraction<T>{
    {static_cast<T>(1), static_cast<T>(3)}
};
inline constexpr auto I_THIRD = FRAC_THIRD<int32_t>;
inline constexpr auto U_THIRD = FRAC_THIRD<uint32_t>;

template<typename T>
inline constexpr auto FRAC_FIFTH = Fraction<T>{
    {static_cast<T>(1), static_cast<T>(5)}
};
inline constexpr auto I_FIFTH = FRAC_FIFTH<int32_t>;
inline constexpr auto U_FIFTH = FRAC_FIFTH<uint32_t>;

template<typename T>
[[nodiscard]] constexpr auto GetFltFraction(const typename Fraction<T>::NumDom& numDom) noexcept
    -> float
{
  return static_cast<float>(numDom.numerator) / static_cast<float>(numDom.denominator);
}

class RangeMapper
{
public:
  constexpr RangeMapper() noexcept = default;
  constexpr RangeMapper(double x0, double x1) noexcept;
  constexpr auto operator()(double x0, double x1, double x) const noexcept -> double;
  [[nodiscard]] constexpr auto GetXMin() const noexcept -> double { return m_xMin; }
  [[nodiscard]] constexpr auto GetXMax() const noexcept -> double { return m_xMax; }

private:
  double m_xMin   = 0;
  double m_xMax   = 0;
  double m_xWidth = 0;
};

constexpr RangeMapper::RangeMapper(const double x0, const double x1) noexcept
  : m_xMin(x0), m_xMax(x1), m_xWidth(m_xMax - m_xMin)
{
}

constexpr auto RangeMapper::operator()(const double x0,
                                       const double x1,
                                       const double x) const noexcept -> double
{
  return STD20::lerp(x0, x1, (x - m_xMin) / m_xWidth);
}

template<typename T>
[[nodiscard]] constexpr auto operator-(const T value, const Fraction<T>& frac) noexcept
    -> Fraction<T>
{
  return {(value * frac.m_denominator) - frac.m_numerator, frac.m_denominator};
}

template<typename T>
[[nodiscard]] constexpr auto operator*(const Fraction<T>& frac, const T value) noexcept -> T
{
  return (frac.m_numerator * value) / frac.m_denominator;
}

} // namespace GOOM::UTILS::MATH
