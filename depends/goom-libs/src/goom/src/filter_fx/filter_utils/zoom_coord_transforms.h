#pragma once

#include "../normalized_coords.h"
#include "point2d.h"

namespace GOOM::FILTER_FX::FILTER_UTILS
{

class ZoomCoordTransforms
{
public:
  explicit ZoomCoordTransforms(const Dimensions& screenDimensions) noexcept;

  [[nodiscard]] auto NormalizedToTranPoint(const NormalizedCoords& normalizedPoint) const noexcept
      -> Point2dInt;

  [[nodiscard]] static auto TranToScreenPoint(const Point2dInt& tranPoint) noexcept -> Point2dInt;
  [[nodiscard]] static auto ScreenToTranPoint(const Point2dInt& screenPoint) noexcept -> Point2dInt;

private:
  NormalizedCoordsConverter m_normalizedCoordsConverter;
  static constexpr auto DIM_FILTER_COEFFS_EXP = 4U;
  static constexpr auto DIM_FILTER_COEFFS     = UTILS::MATH::PowerOf2(DIM_FILTER_COEFFS_EXP);
};

inline ZoomCoordTransforms::ZoomCoordTransforms(const Dimensions& screenDimensions) noexcept
  : m_normalizedCoordsConverter{
        {screenDimensions.GetWidth() << DIM_FILTER_COEFFS_EXP,
         screenDimensions.GetWidth() << DIM_FILTER_COEFFS_EXP},
        1.0F / static_cast<float>(DIM_FILTER_COEFFS)
}
{
}

inline auto ZoomCoordTransforms::TranToScreenPoint(const Point2dInt& tranPoint) noexcept
    -> Point2dInt
{
  // Note: Truncation here but seems OK. Trying to round adds about 2ms.
  return {tranPoint.x >> DIM_FILTER_COEFFS_EXP, tranPoint.y >> DIM_FILTER_COEFFS_EXP};
}

inline auto ZoomCoordTransforms::ScreenToTranPoint(const Point2dInt& screenPoint) noexcept
    -> Point2dInt
{
  return {screenPoint.x << DIM_FILTER_COEFFS_EXP, screenPoint.y << DIM_FILTER_COEFFS_EXP};
}

inline auto ZoomCoordTransforms::NormalizedToTranPoint(
    const NormalizedCoords& normalizedPoint) const noexcept -> Point2dInt
{
  return ToPoint2dInt(m_normalizedCoordsConverter.NormalizedToOtherCoordsFlt(normalizedPoint));
}

} // namespace GOOM::FILTER_FX::FILTER_UTILS
