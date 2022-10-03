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
  constexpr NormalizedCoordsConverter(const Dimensions& otherDimensions,
                                      float minOtherCoordVal,
                                      bool doNotScale = true) noexcept;

  [[nodiscard]] constexpr auto OtherToNormalizedCoords(const Point2dInt& otherCoords) const noexcept
      -> NormalizedCoords;
  [[nodiscard]] constexpr auto NormalizedToOtherCoordsFlt(
      const NormalizedCoords& normalizedCoords) const noexcept -> Point2dFlt;
  [[nodiscard]] constexpr auto GetXMinNormalizedCoordVal() const noexcept -> float;
  [[nodiscard]] constexpr auto GetYMinNormalizedCoordVal() const noexcept -> float;

  constexpr auto Inc(NormalizedCoords& normalizedCoords) const noexcept -> void;
  constexpr auto IncX(NormalizedCoords& normalizedCoords) const noexcept -> void;
  constexpr auto IncY(NormalizedCoords& normalizedCoords) const noexcept -> void;

private:
  const float m_xRatioOtherToNormalizedCoord;
  const float m_yRatioOtherToNormalizedCoord;
  const float m_xRatioNormalizedToOtherCoord;
  const float m_yRatioNormalizedToOtherCoord;
  const float m_xMinNormalizedCoordVal;
  const float m_yMinNormalizedCoordVal;
  [[nodiscard]] constexpr auto OtherToNormalizedXCoord(int32_t otherCoord) const noexcept -> float;
  [[nodiscard]] constexpr auto OtherToNormalizedYCoord(int32_t otherCoord) const noexcept -> float;
  [[nodiscard]] constexpr auto NormalizedToOtherXCoordFlt(float normalizedCoord) const noexcept
      -> float;
  [[nodiscard]] constexpr auto NormalizedToOtherYCoordFlt(float normalizedCoord) const noexcept
      -> float;
};

constexpr NormalizedCoordsConverter::NormalizedCoordsConverter(const Dimensions& otherDimensions,
                                                               const float minOtherCoordVal,
                                                               const bool doNotScale) noexcept
  : m_xRatioOtherToNormalizedCoord{(NormalizedCoords::MAX_NORMALIZED_COORD -
                                    NormalizedCoords::MIN_NORMALIZED_COORD) /
                                   (doNotScale
                                        ? static_cast<float>(std::max(otherDimensions.GetWidth(),
                                                                      otherDimensions.GetHeight()) -
                                                             1)
                                        : static_cast<float>(otherDimensions.GetWidth() - 1))},
    m_yRatioOtherToNormalizedCoord{
        doNotScale
            ? m_xRatioOtherToNormalizedCoord
            : (NormalizedCoords::MAX_NORMALIZED_COORD - NormalizedCoords::MIN_NORMALIZED_COORD) /
                  static_cast<float>(otherDimensions.GetHeight() - 1)},
    m_xRatioNormalizedToOtherCoord{1.0F / m_xRatioOtherToNormalizedCoord},
    m_yRatioNormalizedToOtherCoord{1.0F / m_yRatioOtherToNormalizedCoord},
    m_xMinNormalizedCoordVal{minOtherCoordVal * m_xRatioOtherToNormalizedCoord},
    m_yMinNormalizedCoordVal{minOtherCoordVal * m_yRatioOtherToNormalizedCoord}
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

constexpr auto NormalizedCoordsConverter::OtherToNormalizedCoords(
    const Point2dInt& otherCoords) const noexcept -> NormalizedCoords
{
  return {OtherToNormalizedXCoord(otherCoords.x), OtherToNormalizedYCoord(otherCoords.y)};
}

constexpr auto NormalizedCoordsConverter::NormalizedToOtherCoordsFlt(
    const NormalizedCoords& normalizedCoords) const noexcept -> Point2dFlt
{
  return {NormalizedToOtherXCoordFlt(normalizedCoords.m_fltCoords.x),
          NormalizedToOtherYCoordFlt(normalizedCoords.m_fltCoords.y)};
}

constexpr auto NormalizedCoordsConverter::OtherToNormalizedXCoord(
    const int32_t otherCoord) const noexcept -> float
{
  return NormalizedCoords::MIN_NORMALIZED_COORD +
         (m_xRatioOtherToNormalizedCoord * static_cast<float>(otherCoord));
}

constexpr auto NormalizedCoordsConverter::OtherToNormalizedYCoord(
    const int32_t otherCoord) const noexcept -> float
{
  return NormalizedCoords::MIN_NORMALIZED_COORD +
         (m_yRatioOtherToNormalizedCoord * static_cast<float>(otherCoord));
}

constexpr auto NormalizedCoordsConverter::NormalizedToOtherXCoordFlt(
    const float normalizedCoord) const noexcept -> float
{
  return m_xRatioNormalizedToOtherCoord *
         (normalizedCoord - NormalizedCoords::MIN_NORMALIZED_COORD);
}

constexpr auto NormalizedCoordsConverter::NormalizedToOtherYCoordFlt(
    const float normalizedCoord) const noexcept -> float
{
  return m_yRatioNormalizedToOtherCoord *
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
  normalizedCoords.m_fltCoords.x += m_xRatioOtherToNormalizedCoord;
}

constexpr auto NormalizedCoordsConverter::IncY(NormalizedCoords& normalizedCoords) const noexcept
    -> void
{
  normalizedCoords.m_fltCoords.y += m_yRatioOtherToNormalizedCoord;
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
