#pragma once

#include "utils/mathutils.h"

#include <cmath>
#include <cstdint>

namespace GOOM
{

struct Point2dFlt;

struct Point2dInt
{
  int32_t x = 0;
  int32_t y = 0;
  Point2dInt() noexcept = default;
  Point2dInt(int32_t xx, int32_t yy) noexcept;
  Point2dInt(uint32_t xx, uint32_t yy) noexcept;
  [[nodiscard]] auto ToFlt() const -> Point2dFlt;
  auto operator==(const Point2dInt& point) const -> bool;
  auto operator+=(const Point2dInt& point) -> Point2dInt&;
};

[[nodiscard]] auto operator+(const Point2dInt& point1, const Point2dInt& point2) -> Point2dInt;
auto lerp(const Point2dInt& point1, const Point2dInt& point2, float t) -> Point2dInt;

struct Point2dFlt
{
  float x = 0.0;
  float y = 0.0;
  Point2dFlt() noexcept = default;
  Point2dFlt(float xx, float yy) noexcept;
  [[nodiscard]] auto ToInt() const -> Point2dInt;
  auto operator+=(const Point2dFlt& point) -> Point2dFlt&;
  auto operator+=(float scalar) -> Point2dFlt&;
  auto operator-=(const Point2dFlt& point) -> Point2dFlt&;
  auto operator*=(float scalar) -> Point2dFlt&;
  auto operator*(float scalar) const -> Point2dFlt;
};

[[nodiscard]] auto operator+(const Point2dFlt& point1, const Point2dFlt& point2) -> Point2dFlt;
[[nodiscard]] auto operator-(const Point2dFlt& point1, const Point2dFlt& point2) -> Point2dFlt;
[[nodiscard]] auto operator*(float scalar, const Point2dFlt& point) -> Point2dFlt;
auto lerp(const Point2dFlt& point1, const Point2dFlt& point2, float t) -> Point2dFlt;

inline Point2dInt::Point2dInt(int32_t xx, int32_t yy) noexcept : x{xx}, y{yy}
{
}

inline Point2dInt::Point2dInt(uint32_t xx, uint32_t yy) noexcept
  : x{static_cast<int32_t>(xx)}, y{static_cast<int32_t>(yy)}
{
}

inline auto Point2dInt::ToFlt() const -> Point2dFlt
{
  return {static_cast<float>(x), static_cast<float>(y)};
}

inline auto Point2dInt::operator==(const Point2dInt& point) const -> bool
{
  return (x == point.x) && (y == point.y);
}

inline auto Point2dInt::operator+=(const Point2dInt& point) -> Point2dInt&
{
  x += point.x;
  y += point.y;
  return *this;
}

inline auto operator+(const Point2dInt& point1, const Point2dInt& point2) -> Point2dInt
{
  return {point1.x + point2.x, point1.y + point2.y};
}

inline auto operator-(const Point2dInt& point1, const Point2dInt& point2) -> Point2dInt
{
  return {point1.x - point2.x, point1.y - point2.y};
}

inline Point2dFlt::Point2dFlt(const float xx, const float yy) noexcept : x{xx}, y{yy}
{
}

inline auto Point2dFlt::ToInt() const -> Point2dInt
{
  return {static_cast<int32_t>(std::round(x)), static_cast<int32_t>(std::round(y))};
}

inline auto Point2dFlt::operator+=(const Point2dFlt& point) -> Point2dFlt&
{
  x += point.x;
  y += point.y;
  return *this;
}

inline auto Point2dFlt::operator-=(const Point2dFlt& point) -> Point2dFlt&
{
  x += -point.x;
  y += -point.y;
  return *this;
}

inline auto Point2dFlt::operator+=(const float scalar) -> Point2dFlt&
{
  x += scalar;
  y += scalar;
  return *this;
}

inline auto Point2dFlt::operator*=(const float scalar) -> Point2dFlt&
{
  x *= scalar;
  y *= scalar;
  return *this;
}

inline auto operator+(const Point2dFlt& point1, const Point2dFlt& point2) -> Point2dFlt
{
  return {point1.x + point2.x, point1.y + point2.y};
}

inline auto Point2dFlt::operator*(const float scalar) const -> Point2dFlt
{
  return Point2dFlt{scalar * x, scalar * y};
}

inline auto operator*(const float scalar, const Point2dFlt& point) -> Point2dFlt
{
  return point * scalar;
}

inline auto lerp(const Point2dInt& point1, const Point2dInt& point2, const float t) -> Point2dInt
{
  return {
      static_cast<int32_t>(
          std::round(stdnew::lerp(static_cast<float>(point1.x), static_cast<float>(point2.x), t))),
      static_cast<int32_t>(
          std::round(stdnew::lerp(static_cast<float>(point1.y), static_cast<float>(point2.y), t))),
  };
}

inline auto lerp(const Point2dFlt& point1, const Point2dFlt& point2, float t) -> Point2dFlt
{
  return {
      stdnew::lerp(static_cast<float>(point1.x), static_cast<float>(point2.x), t),
      stdnew::lerp(static_cast<float>(point1.y), static_cast<float>(point2.y), t),
  };
}

} // namespace GOOM
