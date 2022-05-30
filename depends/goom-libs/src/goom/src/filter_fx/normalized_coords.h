#pragma once

#include "point2d.h"
#include "utils/math/misc.h"

#include <algorithm>
#include <cstdint>

namespace GOOM::FILTER_FX
{

class NormalizedCoords
{
public:
  // IMPORTANT: Max coord must be 2.0F - other filter functions
  //            implicitly depend on this.
  static constexpr float MAX_NORMALIZED_COORD = 2.0F;
  static constexpr float MIN_NORMALIZED_COORD = -MAX_NORMALIZED_COORD;

  explicit NormalizedCoords(const Point2dFlt& alreadyNormalized) noexcept;
  NormalizedCoords(float xAlreadyNormalized, float yAlreadyNormalized) noexcept;

  [[nodiscard]] auto GetX() const -> float;
  [[nodiscard]] auto GetY() const -> float;
  auto SetX(float xNormalized) -> void;
  auto SetY(float yNormalized) -> void;

  auto operator+=(const NormalizedCoords& other) -> NormalizedCoords&;
  auto operator-=(const NormalizedCoords& other) -> NormalizedCoords&;
  auto operator*=(float scalar) -> NormalizedCoords&;

  [[nodiscard]] auto Equals(const NormalizedCoords& other) const -> bool;

private:
  friend class NormalizedCoordsConverter;
  Point2dFlt m_fltCoords;
};

[[nodiscard]] auto operator+(const NormalizedCoords& coords1, const NormalizedCoords& coords2)
    -> NormalizedCoords;
[[nodiscard]] auto operator-(const NormalizedCoords& coords1, const NormalizedCoords& coords2)
    -> NormalizedCoords;
[[nodiscard]] auto operator*(float scalar, const NormalizedCoords& coords) -> NormalizedCoords;
[[nodiscard]] auto GetSqDistance(const NormalizedCoords& coords1, const NormalizedCoords& coords2)
    -> float;

class NormalizedCoordsConverter
{
public:
  NormalizedCoordsConverter(uint32_t width, uint32_t height, float minScreenCoordVal);

  [[nodiscard]] auto ScreenToNormalizedCoords(const Point2dInt& screenCoords) const
      -> NormalizedCoords;
  [[nodiscard]] auto NormalizedToScreenCoordsFlt(const NormalizedCoords& normalizedCoords) const
      -> Point2dFlt;
  [[nodiscard]] auto GetMinNormalizedCoordVal() const -> float;

  auto Inc(NormalizedCoords& normalizedCoords) const -> void;
  auto IncX(NormalizedCoords& normalizedCoords) const -> void;
  auto IncY(NormalizedCoords& normalizedCoords) const -> void;

private:
  const float m_ratioScreenToNormalizedCoord;
  const float m_ratioNormalizedToScreenCoord;
  const float m_minNormalizedCoordVal;
  [[nodiscard]] auto ScreenToNormalizedCoord(int32_t screenCoord) const -> float;
  [[nodiscard]] auto NormalizedToScreenCoordFlt(float normalizedCoord) const -> float;
};

inline NormalizedCoordsConverter::NormalizedCoordsConverter(const uint32_t width,
                                                            const uint32_t height,
                                                            const float minScreenCoordVal)
  : m_ratioScreenToNormalizedCoord{(NormalizedCoords::MAX_NORMALIZED_COORD -
                                    NormalizedCoords::MIN_NORMALIZED_COORD) /
                                   static_cast<float>(std::max(width, height) - 1)},
    m_ratioNormalizedToScreenCoord{1.0F / m_ratioScreenToNormalizedCoord},
    m_minNormalizedCoordVal{minScreenCoordVal * m_ratioScreenToNormalizedCoord}
{
}

inline auto NormalizedCoordsConverter::GetMinNormalizedCoordVal() const -> float
{
  return m_minNormalizedCoordVal;
}

inline auto NormalizedCoordsConverter::ScreenToNormalizedCoords(
    const Point2dInt& screenCoords) const -> NormalizedCoords
{
  return {ScreenToNormalizedCoord(screenCoords.x), ScreenToNormalizedCoord(screenCoords.y)};
}

inline auto NormalizedCoordsConverter::NormalizedToScreenCoordsFlt(
    const NormalizedCoords& normalizedCoords) const -> Point2dFlt
{
  return {NormalizedToScreenCoordFlt(normalizedCoords.m_fltCoords.x),
          NormalizedToScreenCoordFlt(normalizedCoords.m_fltCoords.y)};
}

inline auto NormalizedCoordsConverter::ScreenToNormalizedCoord(const int32_t screenCoord) const
    -> float
{
  return NormalizedCoords::MIN_NORMALIZED_COORD +
         (m_ratioScreenToNormalizedCoord * static_cast<float>(screenCoord));
}

inline auto NormalizedCoordsConverter::NormalizedToScreenCoordFlt(const float normalizedCoord) const
    -> float
{
  return m_ratioNormalizedToScreenCoord *
         (normalizedCoord - NormalizedCoords::MIN_NORMALIZED_COORD);
}

inline auto NormalizedCoordsConverter::Inc(NormalizedCoords& normalizedCoords) const -> void
{
  IncX(normalizedCoords);
  IncY(normalizedCoords);
}

inline auto NormalizedCoordsConverter::IncX(NormalizedCoords& normalizedCoords) const -> void
{
  normalizedCoords.m_fltCoords.x += m_ratioScreenToNormalizedCoord;
}

inline auto NormalizedCoordsConverter::IncY(NormalizedCoords& normalizedCoords) const -> void
{
  normalizedCoords.m_fltCoords.y += m_ratioScreenToNormalizedCoord;
}

inline NormalizedCoords::NormalizedCoords(const Point2dFlt& alreadyNormalized) noexcept
  : m_fltCoords{alreadyNormalized}
{
}

inline NormalizedCoords::NormalizedCoords(const float xAlreadyNormalized,
                                          const float yAlreadyNormalized) noexcept
  : m_fltCoords{xAlreadyNormalized, yAlreadyNormalized}
{
}

inline auto NormalizedCoords::GetX() const -> float
{
  return m_fltCoords.x;
}

inline auto NormalizedCoords::GetY() const -> float
{
  return m_fltCoords.y;
}

inline auto NormalizedCoords::SetX(const float xNormalized) -> void
{
  m_fltCoords.x = xNormalized;
}

inline auto NormalizedCoords::SetY(const float yNormalized) -> void
{
  m_fltCoords.y = yNormalized;
}

inline auto NormalizedCoords::Equals(const NormalizedCoords& other) const -> bool
{
  return UTILS::MATH::FloatsEqual(GetX(), other.GetX()) &&
         UTILS::MATH::FloatsEqual(GetY(), other.GetY());
}

inline auto NormalizedCoords::operator+=(const NormalizedCoords& other) -> NormalizedCoords&
{
  m_fltCoords.x += other.m_fltCoords.x;
  m_fltCoords.y += other.m_fltCoords.y;
  return *this;
}

inline auto NormalizedCoords::operator-=(const NormalizedCoords& other) -> NormalizedCoords&
{
  m_fltCoords.x -= other.m_fltCoords.x;
  m_fltCoords.y -= other.m_fltCoords.y;
  return *this;
}

inline auto NormalizedCoords::operator*=(const float scalar) -> NormalizedCoords&
{
  m_fltCoords.x *= scalar;
  m_fltCoords.y *= scalar;
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
  return UTILS::MATH::SqDistance(coords1.GetX() - coords2.GetX(), coords1.GetY() - coords2.GetY());
}

} // namespace GOOM::FILTER_FX
