#pragma once

#include "goom_types.h"
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

  constexpr explicit NormalizedCoords(const Point2dFlt& alreadyNormalized) noexcept;
  constexpr NormalizedCoords(float xAlreadyNormalized, float yAlreadyNormalized) noexcept;

  [[nodiscard]] constexpr auto GetX() const noexcept -> float;
  [[nodiscard]] constexpr auto GetY() const noexcept -> float;
  constexpr auto SetX(float xNormalized) noexcept -> void;
  constexpr auto SetY(float yNormalized) noexcept -> void;

  constexpr auto operator+=(const NormalizedCoords& other) noexcept -> NormalizedCoords&;
  constexpr auto operator-=(const NormalizedCoords& other) noexcept -> NormalizedCoords&;
  constexpr auto operator*=(float scalar) noexcept -> NormalizedCoords&;

  [[nodiscard]] auto Equals(const NormalizedCoords& other) const noexcept -> bool;

private:
  friend class NormalizedCoordsConverter;
  Point2dFlt m_fltCoords;
};

[[nodiscard]] constexpr auto operator+(const NormalizedCoords& coords1,
                                       const NormalizedCoords& coords2) noexcept
    -> NormalizedCoords;
[[nodiscard]] constexpr auto operator-(const NormalizedCoords& coords1,
                                       const NormalizedCoords& coords2) noexcept
    -> NormalizedCoords;
[[nodiscard]] constexpr auto operator*(float scalar, const NormalizedCoords& coords) noexcept
    -> NormalizedCoords;
[[nodiscard]] constexpr auto operator*(const Point2dFlt& scalars,
                                       const NormalizedCoords& coords) noexcept -> NormalizedCoords;
[[nodiscard]] constexpr auto GetSqDistance(const NormalizedCoords& coords1,
                                           const NormalizedCoords& coords2) noexcept -> float;

class NormalizedCoordsConverter
{
public:
  constexpr NormalizedCoordsConverter(const Dimensions& dimensions,
                                      float minScreenCoordVal,
                                      bool doNotScale = true) noexcept;

  [[nodiscard]] constexpr auto ScreenToNormalizedCoords(
      const Point2dInt& screenCoords) const noexcept -> NormalizedCoords;
  [[nodiscard]] constexpr auto NormalizedToScreenCoordsFlt(
      const NormalizedCoords& normalizedCoords) const noexcept -> Point2dFlt;
  [[nodiscard]] constexpr auto GetXMinNormalizedCoordVal() const noexcept -> float;
  [[nodiscard]] constexpr auto GetYMinNormalizedCoordVal() const noexcept -> float;

  constexpr auto Inc(NormalizedCoords& normalizedCoords) const noexcept -> void;
  constexpr auto IncX(NormalizedCoords& normalizedCoords) const noexcept -> void;
  constexpr auto IncY(NormalizedCoords& normalizedCoords) const noexcept -> void;

private:
  const float m_xRatioScreenToNormalizedCoord;
  const float m_yRatioScreenToNormalizedCoord;
  const float m_xRatioNormalizedToScreenCoord;
  const float m_yRatioNormalizedToScreenCoord;
  const float m_xMinNormalizedCoordVal;
  const float m_yMinNormalizedCoordVal;
  [[nodiscard]] constexpr auto ScreenToNormalizedXCoord(int32_t screenCoord) const noexcept
      -> float;
  [[nodiscard]] constexpr auto ScreenToNormalizedYCoord(int32_t screenCoord) const noexcept
      -> float;
  [[nodiscard]] constexpr auto NormalizedToScreenXCoordFlt(float normalizedCoord) const noexcept
      -> float;
  [[nodiscard]] constexpr auto NormalizedToScreenYCoordFlt(float normalizedCoord) const noexcept
      -> float;
};

constexpr NormalizedCoordsConverter::NormalizedCoordsConverter(const Dimensions& dimensions,
                                                               const float minScreenCoordVal,
                                                               const bool doNotScale) noexcept
  : m_xRatioScreenToNormalizedCoord{(NormalizedCoords::MAX_NORMALIZED_COORD -
                                     NormalizedCoords::MIN_NORMALIZED_COORD) /
                                    (doNotScale
                                         ? static_cast<float>(std::max(dimensions.GetWidth(),
                                                                       dimensions.GetHeight()) -
                                                              1)
                                         : static_cast<float>(dimensions.GetWidth() - 1))},
    m_yRatioScreenToNormalizedCoord{
        doNotScale
            ? m_xRatioScreenToNormalizedCoord
            : (NormalizedCoords::MAX_NORMALIZED_COORD - NormalizedCoords::MIN_NORMALIZED_COORD) /
                  static_cast<float>(dimensions.GetHeight() - 1)},
    m_xRatioNormalizedToScreenCoord{1.0F / m_xRatioScreenToNormalizedCoord},
    m_yRatioNormalizedToScreenCoord{1.0F / m_yRatioScreenToNormalizedCoord},
    m_xMinNormalizedCoordVal{minScreenCoordVal * m_xRatioScreenToNormalizedCoord},
    m_yMinNormalizedCoordVal{minScreenCoordVal * m_yRatioScreenToNormalizedCoord}
{
}

constexpr auto NormalizedCoordsConverter::GetXMinNormalizedCoordVal() const noexcept -> float
{
  return m_xMinNormalizedCoordVal;
}

constexpr auto NormalizedCoordsConverter::GetYMinNormalizedCoordVal() const noexcept -> float
{
  return m_yMinNormalizedCoordVal;
}

constexpr auto NormalizedCoordsConverter::ScreenToNormalizedCoords(
    const Point2dInt& screenCoords) const noexcept -> NormalizedCoords
{
  return {ScreenToNormalizedXCoord(screenCoords.x), ScreenToNormalizedYCoord(screenCoords.y)};
}

constexpr auto NormalizedCoordsConverter::NormalizedToScreenCoordsFlt(
    const NormalizedCoords& normalizedCoords) const noexcept -> Point2dFlt
{
  return {NormalizedToScreenXCoordFlt(normalizedCoords.m_fltCoords.x),
          NormalizedToScreenYCoordFlt(normalizedCoords.m_fltCoords.y)};
}

constexpr auto NormalizedCoordsConverter::ScreenToNormalizedXCoord(
    const int32_t screenCoord) const noexcept -> float
{
  return NormalizedCoords::MIN_NORMALIZED_COORD +
         (m_xRatioScreenToNormalizedCoord * static_cast<float>(screenCoord));
}

constexpr auto NormalizedCoordsConverter::ScreenToNormalizedYCoord(
    const int32_t screenCoord) const noexcept -> float
{
  return NormalizedCoords::MIN_NORMALIZED_COORD +
         (m_yRatioScreenToNormalizedCoord * static_cast<float>(screenCoord));
}

constexpr auto NormalizedCoordsConverter::NormalizedToScreenXCoordFlt(
    const float normalizedCoord) const noexcept -> float
{
  return m_xRatioNormalizedToScreenCoord *
         (normalizedCoord - NormalizedCoords::MIN_NORMALIZED_COORD);
}

constexpr auto NormalizedCoordsConverter::NormalizedToScreenYCoordFlt(
    const float normalizedCoord) const noexcept -> float
{
  return m_yRatioNormalizedToScreenCoord *
         (normalizedCoord - NormalizedCoords::MIN_NORMALIZED_COORD);
}

constexpr auto NormalizedCoordsConverter::Inc(NormalizedCoords& normalizedCoords) const noexcept
    -> void
{
  IncX(normalizedCoords);
  IncY(normalizedCoords);
}

constexpr auto NormalizedCoordsConverter::IncX(NormalizedCoords& normalizedCoords) const noexcept
    -> void
{
  normalizedCoords.m_fltCoords.x += m_xRatioScreenToNormalizedCoord;
}

constexpr auto NormalizedCoordsConverter::IncY(NormalizedCoords& normalizedCoords) const noexcept
    -> void
{
  normalizedCoords.m_fltCoords.y += m_yRatioScreenToNormalizedCoord;
}

constexpr NormalizedCoords::NormalizedCoords(const Point2dFlt& alreadyNormalized) noexcept
  : m_fltCoords{alreadyNormalized}
{
}

constexpr NormalizedCoords::NormalizedCoords(const float xAlreadyNormalized,
                                             const float yAlreadyNormalized) noexcept
  : m_fltCoords{xAlreadyNormalized, yAlreadyNormalized}
{
}

constexpr auto NormalizedCoords::GetX() const noexcept -> float
{
  return m_fltCoords.x;
}

constexpr auto NormalizedCoords::GetY() const noexcept -> float
{
  return m_fltCoords.y;
}

constexpr auto NormalizedCoords::SetX(const float xNormalized) noexcept -> void
{
  m_fltCoords.x = xNormalized;
}

constexpr auto NormalizedCoords::SetY(const float yNormalized) noexcept -> void
{
  m_fltCoords.y = yNormalized;
}

inline auto NormalizedCoords::Equals(const NormalizedCoords& other) const noexcept -> bool
{
  return UTILS::MATH::FloatsEqual(GetX(), other.GetX()) &&
         UTILS::MATH::FloatsEqual(GetY(), other.GetY());
}

constexpr auto NormalizedCoords::operator+=(const NormalizedCoords& other) noexcept
    -> NormalizedCoords&
{
  m_fltCoords.x += other.m_fltCoords.x;
  m_fltCoords.y += other.m_fltCoords.y;
  return *this;
}

constexpr auto NormalizedCoords::operator-=(const NormalizedCoords& other) noexcept
    -> NormalizedCoords&
{
  m_fltCoords.x -= other.m_fltCoords.x;
  m_fltCoords.y -= other.m_fltCoords.y;
  return *this;
}

constexpr auto NormalizedCoords::operator*=(const float scalar) noexcept -> NormalizedCoords&
{
  m_fltCoords.x *= scalar;
  m_fltCoords.y *= scalar;
  return *this;
}

constexpr auto operator+(const NormalizedCoords& coords1, const NormalizedCoords& coords2) noexcept
    -> NormalizedCoords
{
  auto coords3 = coords1;
  return coords3 += coords2;
}

constexpr auto operator-(const NormalizedCoords& coords1, const NormalizedCoords& coords2) noexcept
    -> NormalizedCoords
{
  auto coords3 = coords1;
  return coords3 -= coords2;
}

constexpr auto operator*(const float scalar, const NormalizedCoords& coords) noexcept
    -> NormalizedCoords
{
  auto coords1 = coords;
  return coords1 *= scalar;
}

constexpr auto operator*(const Point2dFlt& scalars, const NormalizedCoords& coords) noexcept
    -> NormalizedCoords
{
  return {scalars.x * coords.GetX(), scalars.y * coords.GetY()};
}

constexpr auto GetSqDistance(const NormalizedCoords& coords1,
                             const NormalizedCoords& coords2) noexcept -> float
{
  return UTILS::MATH::SqDistance(coords1.GetX() - coords2.GetX(), coords1.GetY() - coords2.GetY());
}

} // namespace GOOM::FILTER_FX
