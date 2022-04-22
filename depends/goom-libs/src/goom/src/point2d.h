#pragma once

#include "utils/math/misc.h"

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
  [[nodiscard]] auto ToFlt() const noexcept -> Point2dFlt;
  auto Translate(const Vec2dInt& vec) noexcept -> void;
};

[[nodiscard]] auto operator==(const Point2dInt& point1, const Point2dInt& point2) noexcept -> bool;
[[nodiscard]] auto operator+(const Point2dInt& point, const Vec2dInt& vec) noexcept -> Point2dInt;
[[nodiscard]] auto operator-(const Point2dInt& point, const Vec2dInt& vec) noexcept -> Point2dInt;
[[nodiscard]] auto operator+(const Point2dInt& point, int32_t offset) noexcept -> Point2dInt;
[[nodiscard]] auto operator-(const Point2dInt& point, int32_t offset) noexcept -> Point2dInt;
[[nodiscard]] auto lerp(const Point2dInt& point1, const Point2dInt& point2, float t) noexcept
    -> Point2dInt;
[[nodiscard]] auto Distance(const Point2dInt& point1, const Point2dInt& point2) noexcept -> int32_t;

struct Vec2dInt
{
  int32_t x = 0;
  int32_t y = 0;
  Vec2dInt() noexcept = default;
  Vec2dInt(int32_t xx, int32_t yy) noexcept;
  Vec2dInt(uint32_t xx, uint32_t yy) noexcept;
  explicit Vec2dInt(const Point2dInt& point) noexcept;
  [[nodiscard]] auto ToFlt() const noexcept -> Vec2dFlt;
  auto operator+=(const Vec2dInt& vec) noexcept -> Vec2dInt&;
};
[[nodiscard]] auto operator==(const Vec2dInt& vec1, const Vec2dInt& vec2) noexcept -> bool;
[[nodiscard]] auto operator+(const Vec2dInt& vec1, const Vec2dInt& vec2) noexcept -> Vec2dInt;
[[nodiscard]] auto operator-(const Vec2dInt& vec1, const Vec2dInt& vec2) noexcept -> Vec2dInt;
[[nodiscard]] auto operator+(const Vec2dInt& vec1, int32_t offset) noexcept -> Vec2dInt;
[[nodiscard]] auto operator-(const Vec2dInt& vec1, int32_t offset) noexcept -> Vec2dInt;

struct Point2dFlt
{
  float x = 0.0F;
  float y = 0.0F;
  Point2dFlt() noexcept = default;
  Point2dFlt(float xx, float yy) noexcept;
  [[nodiscard]] auto ToInt() const noexcept -> Point2dInt;
  auto Translate(const Vec2dFlt& vec) noexcept -> void;
  auto Translate(float scalar) noexcept -> void;
  auto Rotate(float angle) noexcept -> void;
  auto Scale(float scalar) noexcept -> void;
};

[[nodiscard]] auto operator+(const Point2dFlt& point, const Vec2dFlt& vec) noexcept -> Point2dFlt;
[[nodiscard]] auto operator-(const Point2dFlt& point, const Vec2dFlt& vec) noexcept -> Point2dFlt;
[[nodiscard]] auto operator+(const Point2dFlt& point, float offset) noexcept -> Point2dFlt;
[[nodiscard]] auto operator-(const Point2dFlt& point, float offset) noexcept -> Point2dFlt;
[[nodiscard]] auto lerp(const Point2dFlt& point1, const Point2dFlt& point2, float t) noexcept
    -> Point2dFlt;
[[nodiscard]] auto Distance(const Point2dFlt& point1, const Point2dFlt& point2) noexcept -> float;

struct Vec2dFlt
{
  float x = 0.0F;
  float y = 0.0F;
  Vec2dFlt() noexcept = default;
  Vec2dFlt(float xx, float yy) noexcept;
  explicit Vec2dFlt(const Point2dFlt& point) noexcept;
  [[nodiscard]] auto ToInt() const noexcept -> Vec2dInt;
  auto operator+=(const Vec2dFlt& vec) noexcept -> Vec2dFlt&;
};
[[nodiscard]] auto operator+(const Vec2dFlt& vec1, const Vec2dFlt& vec2) noexcept -> Vec2dFlt;
[[nodiscard]] auto operator-(const Vec2dFlt& vec1, const Vec2dFlt& vec2) noexcept -> Vec2dFlt;
[[nodiscard]] auto operator+(const Vec2dFlt& vec1, float offset) noexcept -> Vec2dFlt;
[[nodiscard]] auto operator-(const Vec2dFlt& vec1, float offset) noexcept -> Vec2dFlt;

inline Point2dInt::Point2dInt(const int32_t xx, const int32_t yy) noexcept : x{xx}, y{yy}
{
}

inline Point2dInt::Point2dInt(const uint32_t xx, const uint32_t yy) noexcept
  : x{static_cast<int32_t>(xx)}, y{static_cast<int32_t>(yy)}
{
}

inline auto Point2dInt::ToFlt() const noexcept -> Point2dFlt
{
  return {static_cast<float>(x), static_cast<float>(y)};
}

inline auto Point2dInt::Translate(const Vec2dInt& vec) noexcept -> void
{
  x += vec.x;
  y += vec.y;
}

inline auto operator==(const Point2dInt& point1, const Point2dInt& point2) noexcept -> bool
{
  return (point1.x == point2.x) && (point1.y == point2.y);
}

inline auto operator+(const Point2dInt& point, const Vec2dInt& vec) noexcept -> Point2dInt
{
  return {point.x + vec.x, point.y + vec.y};
}

inline auto operator-(const Point2dInt& point, const Vec2dInt& vec) noexcept -> Point2dInt
{
  return {point.x - vec.x, point.y - vec.y};
}

inline auto operator+(const Point2dInt& point, const int32_t offset) noexcept -> Point2dInt
{
  return {point.x + offset, point.y + offset};
}

inline auto operator-(const Point2dInt& point, const int32_t offset) noexcept -> Point2dInt
{
  return {point.x - offset, point.y - offset};
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

inline auto Vec2dInt::ToFlt() const noexcept -> Vec2dFlt
{
  return {static_cast<float>(x), static_cast<float>(y)};
}

inline auto operator==(const Vec2dInt& vec1, const Vec2dInt& vec2) noexcept -> bool
{
  return (vec1.x == vec2.x) && (vec1.y == vec2.y);
}

inline auto Vec2dInt::operator+=(const Vec2dInt& vec) noexcept -> Vec2dInt&
{
  x += vec.x;
  y += vec.y;
  return *this;
}

inline auto operator+(const Vec2dInt& vec1, const Vec2dInt& vec2) noexcept -> Vec2dInt
{
  return {vec1.x + vec2.x, vec1.y + vec2.y};
}

inline auto operator-(const Vec2dInt& vec1, const Vec2dInt& vec2) noexcept -> Vec2dInt
{
  return {vec1.x - vec2.x, vec1.y - vec2.y};
}

inline auto operator+(const Vec2dInt& vec1, const int32_t& offset) noexcept -> Vec2dInt
{
  return {vec1.x + offset, vec1.y + offset};
}

inline auto operator-(const Vec2dInt& vec1, const int32_t& offset) noexcept -> Vec2dInt
{
  return {vec1.x - offset, vec1.y - offset};
}

inline Point2dFlt::Point2dFlt(const float xx, const float yy) noexcept : x{xx}, y{yy}
{
}

inline auto Point2dFlt::ToInt() const noexcept -> Point2dInt
{
  return {static_cast<int32_t>(std::round(x)), static_cast<int32_t>(std::round(y))};
}

inline auto Point2dFlt::Translate(const Vec2dFlt& vec) noexcept -> void
{
  x += vec.x;
  y += vec.y;
}

inline auto Point2dFlt::Translate(const float scalar) noexcept -> void
{
  x += scalar;
  y += scalar;
}

inline auto Point2dFlt::Rotate(const float angle) noexcept -> void
{
  const float sinAngle = std::sin(angle);
  const float cosAngle = std::cos(angle);
  const float xNext = (x * cosAngle) - (y * sinAngle);
  y = (x * sinAngle) + (y * cosAngle);
  x = xNext;
}

inline auto Point2dFlt::Scale(const float scalar) noexcept -> void
{
  x *= scalar;
  y *= scalar;
}

inline auto operator+(const Point2dFlt& point, const Vec2dFlt& vec) noexcept -> Point2dFlt
{
  return {point.x + vec.x, point.y + vec.y};
}

inline auto operator-(const Point2dFlt& point, const Vec2dFlt& vec) noexcept -> Point2dFlt
{
  return {point.x - vec.x, point.y - vec.y};
}

inline auto operator+(const Point2dFlt& point, float offset) noexcept -> Point2dFlt
{
  return {point.x + offset, point.y + offset};
}

inline auto operator-(const Point2dFlt& point, float offset) noexcept -> Point2dFlt
{
  return {point.x - offset, point.y - offset};
}

inline auto operator*(const float scalar, const Point2dFlt& point) noexcept -> Point2dFlt
{
  return {scalar * point.x, scalar * point.y};
}

inline Vec2dFlt::Vec2dFlt(const float xx, const float yy) noexcept : x{xx}, y{yy}
{
}

inline Vec2dFlt::Vec2dFlt(const Point2dFlt& point) noexcept : x{point.x}, y{point.y}
{
}

inline auto Vec2dFlt::ToInt() const noexcept -> Vec2dInt
{
  return {static_cast<int32_t>(std::round(x)), static_cast<int32_t>(std::round(y))};
}

inline auto Vec2dFlt::operator+=(const Vec2dFlt& vec) noexcept -> Vec2dFlt&
{
  x += vec.x;
  y += vec.y;
  return *this;
}

inline auto operator+(const Vec2dFlt& vec1, const Vec2dFlt& vec2) noexcept -> Vec2dFlt
{
  return {vec1.x + vec2.x, vec1.y + vec2.y};
}

inline auto operator-(const Vec2dFlt& vec1, const Vec2dFlt& vec2) noexcept -> Vec2dFlt
{
  return {vec1.x - vec2.x, vec1.y - vec2.y};
}

inline auto operator+(const Vec2dFlt& vec1, const float offset) noexcept -> Vec2dFlt
{
  return {vec1.x + offset, vec1.y + offset};
}

inline auto operator-(const Vec2dFlt& vec1, const float offset) noexcept -> Vec2dFlt
{
  return {vec1.x - offset, vec1.y - offset};
}

inline auto lerp(const Point2dInt& point1, const Point2dInt& point2, const float t) noexcept
    -> Point2dInt
{
  return {
      static_cast<int32_t>(
          std::round(STD20::lerp(static_cast<float>(point1.x), static_cast<float>(point2.x), t))),
      static_cast<int32_t>(
          std::round(STD20::lerp(static_cast<float>(point1.y), static_cast<float>(point2.y), t))),
  };
}

inline auto lerp(const Point2dFlt& point1, const Point2dFlt& point2, const float t) noexcept
    -> Point2dFlt
{
  return {
      STD20::lerp(point1.x, point2.x, t),
      STD20::lerp(point1.y, point2.y, t),
  };
}

inline auto Distance(const Point2dInt& point1, const Point2dInt& point2) noexcept -> int32_t
{
  return static_cast<int32_t>(std::lround(Distance(point1.ToFlt(), point2.ToFlt())));
}

inline auto Distance(const Point2dFlt& point1, const Point2dFlt& point2) noexcept -> float
{
  return std::sqrt(UTILS::MATH::SqDistance(point1.x - point2.x, point1.y - point2.y));
}

} // namespace GOOM
