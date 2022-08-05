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
  NormalizedCoordsConverter(const Dimensions& dimensions,
                            float minScreenCoordVal,
                            bool doNotScale = true);

  [[nodiscard]] auto ScreenToNormalizedCoords(const Point2dInt& screenCoords) const
      -> NormalizedCoords;
  [[nodiscard]] auto NormalizedToScreenCoordsFlt(const NormalizedCoords& normalizedCoords) const
      -> Point2dFlt;
  [[nodiscard]] auto GetXMinNormalizedCoordVal() const -> float;
  [[nodiscard]] auto GetYMinNormalizedCoordVal() const -> float;

  auto Inc(NormalizedCoords& normalizedCoords) const -> void;
  auto IncX(NormalizedCoords& normalizedCoords) const -> void;
  auto IncY(NormalizedCoords& normalizedCoords) const -> void;

private:
  const float m_xRatioScreenToNormalizedCoord;
  const float m_yRatioScreenToNormalizedCoord;
  const float m_xRatioNormalizedToScreenCoord;
  const float m_yRatioNormalizedToScreenCoord;
  const float m_xMinNormalizedCoordVal;
  const float m_yMinNormalizedCoordVal;
  [[nodiscard]] auto ScreenToNormalizedXCoord(int32_t screenCoord) const -> float;
  [[nodiscard]] auto ScreenToNormalizedYCoord(int32_t screenCoord) const -> float;
  [[nodiscard]] auto NormalizedToScreenXCoordFlt(float normalizedCoord) const -> float;
  [[nodiscard]] auto NormalizedToScreenYCoordFlt(float normalizedCoord) const -> float;
};

inline NormalizedCoordsConverter::NormalizedCoordsConverter(const Dimensions& dimensions,
                                                            const float minScreenCoordVal,
                                                            const bool doNotScale)
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

inline auto NormalizedCoordsConverter::GetXMinNormalizedCoordVal() const -> float
{
  return m_xMinNormalizedCoordVal;
}

inline auto NormalizedCoordsConverter::GetYMinNormalizedCoordVal() const -> float
{
  return m_yMinNormalizedCoordVal;
}

inline auto NormalizedCoordsConverter::ScreenToNormalizedCoords(
    const Point2dInt& screenCoords) const -> NormalizedCoords
{
  return {ScreenToNormalizedXCoord(screenCoords.x), ScreenToNormalizedYCoord(screenCoords.y)};
}

inline auto NormalizedCoordsConverter::NormalizedToScreenCoordsFlt(
    const NormalizedCoords& normalizedCoords) const -> Point2dFlt
{
  return {NormalizedToScreenXCoordFlt(normalizedCoords.m_fltCoords.x),
          NormalizedToScreenYCoordFlt(normalizedCoords.m_fltCoords.y)};
}

inline auto NormalizedCoordsConverter::ScreenToNormalizedXCoord(const int32_t screenCoord) const
    -> float
{
  return NormalizedCoords::MIN_NORMALIZED_COORD +
         (m_xRatioScreenToNormalizedCoord * static_cast<float>(screenCoord));
}

inline auto NormalizedCoordsConverter::ScreenToNormalizedYCoord(const int32_t screenCoord) const
    -> float
{
  return NormalizedCoords::MIN_NORMALIZED_COORD +
         (m_yRatioScreenToNormalizedCoord * static_cast<float>(screenCoord));
}

inline auto NormalizedCoordsConverter::NormalizedToScreenXCoordFlt(
    const float normalizedCoord) const -> float
{
  return m_xRatioNormalizedToScreenCoord *
         (normalizedCoord - NormalizedCoords::MIN_NORMALIZED_COORD);
}

inline auto NormalizedCoordsConverter::NormalizedToScreenYCoordFlt(
    const float normalizedCoord) const -> float
{
  return m_yRatioNormalizedToScreenCoord *
         (normalizedCoord - NormalizedCoords::MIN_NORMALIZED_COORD);
}

inline auto NormalizedCoordsConverter::Inc(NormalizedCoords& normalizedCoords) const -> void
{
  IncX(normalizedCoords);
  IncY(normalizedCoords);
}

inline auto NormalizedCoordsConverter::IncX(NormalizedCoords& normalizedCoords) const -> void
{
  normalizedCoords.m_fltCoords.x += m_xRatioScreenToNormalizedCoord;
}

inline auto NormalizedCoordsConverter::IncY(NormalizedCoords& normalizedCoords) const -> void
{
  normalizedCoords.m_fltCoords.y += m_yRatioScreenToNormalizedCoord;
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
  auto coords3 = coords1;
  return coords3 += coords2;
}

inline auto operator-(const NormalizedCoords& coords1, const NormalizedCoords& coords2)
    -> NormalizedCoords
{
  auto coords3 = coords1;
  return coords3 -= coords2;
}

inline auto operator*(const float scalar, const NormalizedCoords& coords) -> NormalizedCoords
{
  auto coords1 = coords;
  return coords1 *= scalar;
}

inline auto GetSqDistance(const NormalizedCoords& coords1, const NormalizedCoords& coords2) -> float
{
  return UTILS::MATH::SqDistance(coords1.GetX() - coords2.GetX(), coords1.GetY() - coords2.GetY());
}

} // namespace GOOM::FILTER_FX
