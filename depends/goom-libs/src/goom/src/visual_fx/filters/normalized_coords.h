#pragma once

#include "point2d.h"
#include "utils/mathutils.h"

#include <cstdint>

namespace GOOM::VISUAL_FX::FILTERS
{

class NormalizedCoords
{
public:
  // IMPORTANT: Max coord must be 2.0F - other filter functions
  //            implicitly depend on this.
  static constexpr float MAX_NORMALIZED_COORD = 2.0F;
  static constexpr float MIN_NORMALIZED_COORD = -MAX_NORMALIZED_COORD;

  static void SetScreenDimensions(uint32_t width, uint32_t height, float minScreenCoordVal);
  [[nodiscard]] static auto GetMinNormalizedCoordVal() -> float;

  explicit NormalizedCoords(const Point2dInt& screenCoords) noexcept;
  explicit NormalizedCoords(const Point2dFlt& alreadyNormalized) noexcept;
  NormalizedCoords(float xAlreadyNormalized, float yAlreadyNormalized) noexcept;

  [[nodiscard]] auto GetScreenCoordsFlt() const -> Point2dFlt;

  void Inc();
  void IncX();
  void IncY();

  [[nodiscard]] auto GetX() const -> float;
  [[nodiscard]] auto GetY() const -> float;
  void SetX(float xNormalized);
  void SetY(float yNormalized);

  auto operator+=(const NormalizedCoords& other) -> NormalizedCoords&;
  auto operator-=(const NormalizedCoords& other) -> NormalizedCoords&;
  auto operator*=(float scalar) -> NormalizedCoords&;

  [[nodiscard]] auto Equals(const NormalizedCoords& other) const -> bool;

private:
  static float s_ratioScreenToNormalizedCoord;
  static float s_ratioNormalizedToScreenCoord;
  static float s_minNormalizedCoordVal;
  Point2dFlt m_normalizedCoords;

  [[nodiscard]] static auto NormalizedToScreenCoordsFlt(const Point2dFlt& normalizedCoords)
      -> Point2dFlt;
  [[nodiscard]] static auto ScreenToNormalizedCoords(const Point2dInt& screenCoords) -> Point2dFlt;
  [[nodiscard]] static auto ScreenToNormalizedCoord(int32_t screenCoord) -> float;
};

[[nodiscard]] auto operator+(const NormalizedCoords& coords1, const NormalizedCoords& coords2)
    -> NormalizedCoords;
[[nodiscard]] auto operator-(const NormalizedCoords& coords1, const NormalizedCoords& coords2)
    -> NormalizedCoords;
[[nodiscard]] auto operator*(float scalar, const NormalizedCoords& coords) -> NormalizedCoords;
[[nodiscard]] auto GetSqDistance(const NormalizedCoords& coords1, const NormalizedCoords& coords2)
    -> float;

inline auto NormalizedCoords::NormalizedToScreenCoordsFlt(const Point2dFlt& normalizedCoords)
    -> Point2dFlt
{
  return {s_ratioNormalizedToScreenCoord * (normalizedCoords.x - MIN_NORMALIZED_COORD),
          s_ratioNormalizedToScreenCoord * (normalizedCoords.y - MIN_NORMALIZED_COORD)};
}

inline auto NormalizedCoords::ScreenToNormalizedCoords(const Point2dInt& screenCoords) -> Point2dFlt
{
  return {ScreenToNormalizedCoord(screenCoords.x), ScreenToNormalizedCoord(screenCoords.y)};
}

inline auto NormalizedCoords::ScreenToNormalizedCoord(const int32_t screenCoord) -> float
{
  return MIN_NORMALIZED_COORD + (s_ratioScreenToNormalizedCoord * static_cast<float>(screenCoord));
}

inline auto NormalizedCoords::GetMinNormalizedCoordVal() -> float
{
  return s_minNormalizedCoordVal;
}

inline void NormalizedCoords::Inc()
{
  IncX();
  IncY();
}

inline void NormalizedCoords::IncX()
{
  m_normalizedCoords.x += s_ratioScreenToNormalizedCoord;
}

inline void NormalizedCoords::IncY()
{
  m_normalizedCoords.y += s_ratioScreenToNormalizedCoord;
}

inline NormalizedCoords::NormalizedCoords(const Point2dInt& screenCoords) noexcept
  : m_normalizedCoords{ScreenToNormalizedCoords(screenCoords)}
{
}

inline NormalizedCoords::NormalizedCoords(const Point2dFlt& alreadyNormalized) noexcept
  : m_normalizedCoords{alreadyNormalized}
{
}

inline NormalizedCoords::NormalizedCoords(const float xAlreadyNormalized,
                                          const float yAlreadyNormalized) noexcept
  : m_normalizedCoords{xAlreadyNormalized, yAlreadyNormalized}
{
}

inline auto NormalizedCoords::GetScreenCoordsFlt() const -> Point2dFlt
{
  return NormalizedToScreenCoordsFlt(m_normalizedCoords);
}

inline auto NormalizedCoords::GetX() const -> float
{
  return m_normalizedCoords.x;
}

inline auto NormalizedCoords::GetY() const -> float
{
  return m_normalizedCoords.y;
}

inline void NormalizedCoords::SetX(const float xNormalized)
{
  m_normalizedCoords.x = xNormalized;
}

inline void NormalizedCoords::SetY(const float yNormalized)
{
  m_normalizedCoords.y = yNormalized;
}

inline auto NormalizedCoords::Equals(const NormalizedCoords& other) const -> bool
{
  return UTILS::floats_equal(GetX(), other.GetX()) && UTILS::floats_equal(GetY(), other.GetY());
}

inline auto NormalizedCoords::operator+=(const NormalizedCoords& other) -> NormalizedCoords&
{
  m_normalizedCoords.x += other.m_normalizedCoords.x;
  m_normalizedCoords.y += other.m_normalizedCoords.y;
  return *this;
}

inline auto NormalizedCoords::operator-=(const NormalizedCoords& other) -> NormalizedCoords&
{
  m_normalizedCoords.x -= other.m_normalizedCoords.x;
  m_normalizedCoords.y -= other.m_normalizedCoords.y;
  return *this;
}

inline auto NormalizedCoords::operator*=(const float scalar) -> NormalizedCoords&
{
  m_normalizedCoords.x *= scalar;
  m_normalizedCoords.y *= scalar;
  return *this;
}

inline auto operator+(const NormalizedCoords& coords1, const NormalizedCoords& coords2)
    -> NormalizedCoords
{
  NormalizedCoords coords3{coords1};
  return coords3 += coords2;
}

inline auto operator-(const NormalizedCoords& coords1, const NormalizedCoords& coords2)
    -> NormalizedCoords
{
  NormalizedCoords coords3{coords1};
  return coords3 -= coords2;
}

inline auto operator*(const float scalar, const NormalizedCoords& coords) -> NormalizedCoords
{
  NormalizedCoords coords1{coords};
  return coords1 *= scalar;
}

inline auto GetSqDistance(const NormalizedCoords& coords1, const NormalizedCoords& coords2) -> float
{
  return UTILS::SqDistance(coords1.GetX() - coords2.GetX(), coords1.GetY() - coords2.GetY());
}

} // namespace GOOM::VISUAL_FX::FILTERS
