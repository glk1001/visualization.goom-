#pragma once

#include "utils/mathutils.h"

#include <cmath>
#include <cstdint>

namespace GOOM
{

struct Point2dFlt;
struct Vec2dInt;
struct Vec2dFlt;

struct Point2dInt
{
  int32_t x = 0;
  int32_t y = 0;
  Point2dInt() noexcept = default;
  Point2dInt(int32_t xx, int32_t yy) noexcept;
  Point2dInt(uint32_t xx, uint32_t yy) noexcept;
  [[nodiscard]] auto ToFlt() const -> Point2dFlt;
  void Translate(const Vec2dInt& vec);
};

[[nodiscard]] auto operator==(const Point2dInt& point1, const Point2dInt& point2) -> bool;
[[nodiscard]] auto operator+(const Point2dInt& point, const Vec2dInt& vec) -> Point2dInt;
[[nodiscard]] auto operator-(const Point2dInt& point, const Vec2dInt& vec) -> Point2dInt;
[[nodiscard]] auto lerp(const Point2dInt& point1, const Point2dInt& point2, float t) -> Point2dInt;

struct Vec2dInt
{
  int32_t x = 0;
  int32_t y = 0;
  Vec2dInt() noexcept = default;
  Vec2dInt(int32_t xx, int32_t yy) noexcept;
  Vec2dInt(uint32_t xx, uint32_t yy) noexcept;
  explicit Vec2dInt(const Point2dInt& point) noexcept;
  [[nodiscard]] auto ToFlt() const -> Vec2dFlt;
  auto operator+=(const Vec2dInt& vec) -> Vec2dInt&;
};
[[nodiscard]] auto operator==(const Vec2dInt& vec1, const Vec2dInt& vec2) -> bool;
[[nodiscard]] auto operator+(const Vec2dInt& vec1, const Vec2dInt& vec2) -> Vec2dInt;
[[nodiscard]] auto operator-(const Vec2dInt& vec1, const Vec2dInt& vec2) -> Vec2dInt;

struct Point2dFlt
{
  float x = 0.0F;
  float y = 0.0F;
  Point2dFlt() noexcept = default;
  Point2dFlt(float xx, float yy) noexcept;
  [[nodiscard]] auto ToInt() const -> Point2dInt;
  void Translate(const Vec2dFlt& vec);
  void Translate(float scalar);
  void Scale(float scalar);
};

[[nodiscard]] auto operator+(const Point2dFlt& point, const Vec2dFlt& vec) -> Point2dFlt;
[[nodiscard]] auto operator-(const Point2dFlt& point, const Vec2dFlt& vec) -> Point2dFlt;
[[nodiscard]] auto lerp(const Point2dFlt& point1, const Point2dFlt& point2, float t) -> Point2dFlt;

struct Vec2dFlt
{
  float x = 0.0F;
  float y = 0.0F;
  Vec2dFlt() noexcept = default;
  Vec2dFlt(float xx, float yy) noexcept;
  [[nodiscard]] auto ToInt() const -> Vec2dInt;
  auto operator+=(const Vec2dFlt& vec) -> Vec2dFlt&;
};
[[nodiscard]] auto operator+(const Vec2dFlt& vec1, const Vec2dFlt& vec2) -> Vec2dFlt;
[[nodiscard]] auto operator-(const Vec2dFlt& vec1, const Vec2dFlt& vec2) -> Vec2dFlt;

inline Point2dInt::Point2dInt(const int32_t xx, const int32_t yy) noexcept : x{xx}, y{yy}
{
}

inline Point2dInt::Point2dInt(const uint32_t xx, const uint32_t yy) noexcept
  : x{static_cast<int32_t>(xx)}, y{static_cast<int32_t>(yy)}
{
}

inline auto Point2dInt::ToFlt() const -> Point2dFlt
{
  return {static_cast<float>(x), static_cast<float>(y)};
}

inline void Point2dInt::Translate(const Vec2dInt& vec)
{
  x += vec.x;
  y += vec.y;
}

inline auto operator==(const Point2dInt& point1, const Point2dInt& point2) -> bool
{
  return (point1.x == point2.x) && (point1.y == point2.y);
}

inline auto operator+(const Point2dInt& point, const Vec2dInt& vec) -> Point2dInt
{
  return {point.x + vec.x, point.y + vec.y};
}

inline auto operator-(const Point2dInt& point, const Vec2dInt& vec) -> Point2dInt
{
  return {point.x - vec.x, point.y - vec.y};
}

inline Vec2dInt::Vec2dInt(const int32_t xx, const int32_t yy) noexcept : x{xx}, y{yy}
{
}

inline Vec2dInt::Vec2dInt(const uint32_t xx, const uint32_t yy) noexcept
  : x{static_cast<int32_t>(xx)}, y{static_cast<int32_t>(yy)}
{
}

inline Vec2dInt::Vec2dInt(const Point2dInt& point) noexcept : x{point.x}, y{point.y}
{
}

inline auto Vec2dInt::ToFlt() const -> Vec2dFlt
{
  return {static_cast<float>(x), static_cast<float>(y)};
}

inline auto operator==(const Vec2dInt& vec1, const Vec2dInt& vec2) -> bool
{
  return (vec1.x == vec2.x) && (vec1.y == vec2.y);
}

inline auto Vec2dInt::operator+=(const Vec2dInt& vec) -> Vec2dInt&
{
  x += vec.x;
  y += vec.y;
  return *this;
}

inline auto operator+(const Vec2dInt& vec1, const Vec2dInt& vec2) -> Vec2dInt
{
  return {vec1.x + vec2.x, vec1.y + vec2.y};
}

inline auto operator-(const Vec2dInt& vec1, const Vec2dInt& vec2) -> Vec2dInt
{
  return {vec1.x - vec2.x, vec1.y - vec2.y};
}

inline Point2dFlt::Point2dFlt(const float xx, const float yy) noexcept : x{xx}, y{yy}
{
}

inline auto Point2dFlt::ToInt() const -> Point2dInt
{
  return {static_cast<int32_t>(std::round(x)), static_cast<int32_t>(std::round(y))};
}

inline void Point2dFlt::Translate(const Vec2dFlt& vec)
{
  x += vec.x;
  y += vec.y;
}

inline void Point2dFlt::Translate(const float scalar)
{
  x += scalar;
  y += scalar;
}

inline void Point2dFlt::Scale(const float scalar)
{
  x *= scalar;
  y *= scalar;
}

inline auto operator+(const Point2dFlt& point, const Vec2dFlt& vec) -> Point2dFlt
{
  return {point.x + vec.x, point.y + vec.y};
}

inline auto operator*(const float scalar, const Point2dFlt& point) -> Point2dFlt
{
  return {scalar * point.x, scalar * point.y};
}

inline Vec2dFlt::Vec2dFlt(const float xx, const float yy) noexcept : x{xx}, y{yy}
{
}

inline auto Vec2dFlt::ToInt() const -> Vec2dInt
{
  return {static_cast<int32_t>(std::round(x)), static_cast<int32_t>(std::round(y))};
}

inline auto Vec2dFlt::operator+=(const Vec2dFlt& vec) -> Vec2dFlt&
{
  x += vec.x;
  y += vec.y;
  return *this;
}

inline auto operator+(const Vec2dFlt& vec1, const Vec2dFlt& vec2) -> Vec2dFlt
{
  return {vec1.x + vec2.x, vec1.y + vec2.y};
}

inline auto operator-(const Vec2dFlt& vec1, const Vec2dFlt& vec2) -> Vec2dFlt
{
  return {vec1.x - vec2.x, vec1.y - vec2.y};
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

inline auto lerp(const Point2dFlt& point1, const Point2dFlt& point2, const float t) -> Point2dFlt
{
  return {
      stdnew::lerp(point1.x, point2.x, t),
      stdnew::lerp(point1.y, point2.y, t),
  };
}

} // namespace GOOM
