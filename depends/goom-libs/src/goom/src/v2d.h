#pragma once

#include "utils/mathutils.h"

#include <cmath>
#include <cstdint>

namespace GOOM
{

struct V2dFlt;

struct V2dInt
{
  int32_t x = 0;
  int32_t y = 0;
  V2dInt() noexcept = default;
  V2dInt(int32_t xx, int32_t yy) noexcept;
  V2dInt(uint32_t xx, uint32_t yy) noexcept;
  [[nodiscard]] auto ToFlt() const -> V2dFlt;
  auto operator==(const V2dInt& point) const -> bool;
  auto operator+=(const V2dInt& point) -> V2dInt&;
};

[[nodiscard]] auto operator+(const V2dInt& point1, const V2dInt& point2) -> V2dInt;
auto lerp(const V2dInt& point1, const V2dInt& point2, float t) -> V2dInt;

struct V2dFlt
{
  float x = 0.0;
  float y = 0.0;
  V2dFlt() noexcept = default;
  V2dFlt(float xx, float yy) noexcept;
  [[nodiscard]] auto ToInt() const -> V2dInt;
  auto operator+=(const V2dFlt& point) -> V2dFlt&;
  auto operator+=(float scalar) -> V2dFlt&;
  auto operator-=(const V2dFlt& point) -> V2dFlt&;
  auto operator*=(float scalar) -> V2dFlt&;
  auto operator*(float scalar) const -> V2dFlt;
};

[[nodiscard]] auto operator+(const V2dFlt& point1, const V2dFlt& point2) -> V2dFlt;
[[nodiscard]] auto operator-(const V2dFlt& point1, const V2dFlt& point2) -> V2dFlt;
[[nodiscard]] auto operator*(float scalar, const V2dFlt& point) -> V2dFlt;
auto lerp(const V2dFlt& point1, const V2dFlt& point2, float t) -> V2dFlt;

inline V2dInt::V2dInt(int32_t xx, int32_t yy) noexcept : x{xx}, y{yy}
{
}

inline V2dInt::V2dInt(uint32_t xx, uint32_t yy) noexcept
  : x{static_cast<int32_t>(xx)}, y{static_cast<int32_t>(yy)}
{
}

inline auto V2dInt::ToFlt() const -> V2dFlt
{
  return {static_cast<float>(x), static_cast<float>(y)};
}

inline auto V2dInt::operator==(const V2dInt& point) const -> bool
{
  return (x == point.x) && (y == point.y);
}

inline auto V2dInt::operator+=(const V2dInt& point) -> V2dInt&
{
  x += point.x;
  y += point.y;
  return *this;
}

inline auto operator+(const V2dInt& point1, const V2dInt& point2) -> V2dInt
{
  return {point1.x + point2.x, point1.y + point2.y};
}

inline auto operator-(const V2dInt& point1, const V2dInt& point2) -> V2dInt
{
  return {point1.x - point2.x, point1.y - point2.y};
}

inline V2dFlt::V2dFlt(const float xx, const float yy) noexcept : x{xx}, y{yy}
{
}

inline auto V2dFlt::ToInt() const -> V2dInt
{
  return {static_cast<int32_t>(std::round(x)), static_cast<int32_t>(std::round(y))};
}

inline auto V2dFlt::operator+=(const V2dFlt& point) -> V2dFlt&
{
  x += point.x;
  y += point.y;
  return *this;
}

inline auto V2dFlt::operator-=(const V2dFlt& point) -> V2dFlt&
{
  x += -point.x;
  y += -point.y;
  return *this;
}

inline auto V2dFlt::operator+=(const float scalar) -> V2dFlt&
{
  x += scalar;
  y += scalar;
  return *this;
}

inline auto V2dFlt::operator*=(const float scalar) -> V2dFlt&
{
  x *= scalar;
  y *= scalar;
  return *this;
}

inline auto operator+(const V2dFlt& point1, const V2dFlt& point2) -> V2dFlt
{
  return {point1.x + point2.x, point1.y + point2.y};
}

inline auto V2dFlt::operator*(const float scalar) const -> V2dFlt
{
  return V2dFlt{scalar * x, scalar * y};
}

inline auto operator*(const float scalar, const V2dFlt& point) -> V2dFlt
{
  return point * scalar;
}

inline auto lerp(const V2dInt& point1, const V2dInt& point2, const float t) -> V2dInt
{
  return {
      static_cast<int32_t>(
          std::round(stdnew::lerp(static_cast<float>(point1.x), static_cast<float>(point2.x), t))),
      static_cast<int32_t>(
          std::round(stdnew::lerp(static_cast<float>(point1.y), static_cast<float>(point2.y), t))),
  };
}

inline auto lerp(const V2dFlt& point1, const V2dFlt& point2, float t) -> V2dFlt
{
  return {
      stdnew::lerp(static_cast<float>(point1.x), static_cast<float>(point2.x), t),
      stdnew::lerp(static_cast<float>(point1.y), static_cast<float>(point2.y), t),
  };
}

} // namespace GOOM
