#pragma once

#include "math20.h"

#include <cmath>

namespace GOOM::UTILS::MATH
{

constexpr float TWO_PI = 2.0F * STD20::pi;
constexpr float HALF_PI = 0.5F * STD20::pi;
constexpr float THIRD_PI = STD20::pi / 3.0F;

constexpr float DEGREES_90 = 90.0F;
constexpr float DEGREES_180 = 180.0F;
constexpr float DEGREES_360 = 360.0F;

template<typename T>
[[nodiscard]] constexpr auto Sq(const T& x) -> T
{
  return x * x;
}

[[nodiscard]] constexpr auto SqDistance(const float x, const float y) -> float
{
  return Sq(x) + Sq(y);
}

constexpr float SMALL_FLOAT = 0.00001F;

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
[[nodiscard]] constexpr auto GetHalf(const T& n) -> T
{
  return n / 2;
}

template<typename T>
[[nodiscard]] constexpr auto GetQuarter(const T& n) -> T
{
  return n / 4;
}

template<typename T>
[[nodiscard]] constexpr auto GetThreeQuarters(const T& n) -> T
{
  return (3 * n) / 4;
}

class RangeMapper
{
public:
  RangeMapper() noexcept = default;
  explicit RangeMapper(double x0, double x1) noexcept;
  auto operator()(double x0, double x1, double x) const -> double;
  [[nodiscard]] auto GetXMin() const -> double { return m_xMin; }
  [[nodiscard]] auto GetXMax() const -> double { return m_xMax; }

private:
  double m_xMin = 0;
  double m_xMax = 0;
  double m_xWidth = 0;
};

inline RangeMapper::RangeMapper(const double x0, const double x1) noexcept
  : m_xMin(x0), m_xMax(x1), m_xWidth(m_xMax - m_xMin)
{
}

inline auto RangeMapper::operator()(const double x0, const double x1, const double x) const
    -> double
{
  return STD20::lerp(x0, x1, (x - m_xMin) / m_xWidth);
}

} // namespace GOOM::UTILS::MATH
