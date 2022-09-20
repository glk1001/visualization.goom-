#pragma once

#include "../normalized_coords.h"
#include "point2d.h"
#include "utils/math/misc.h"
#include "zoom_filter_coefficients.h"

#include <cstdint>

namespace GOOM::FILTER_FX::FILTER_UTILS
{

class ZoomCoordTransforms
{
public:
  explicit ZoomCoordTransforms(const NormalizedCoordsConverter& normalizedCoordsConverter) noexcept;

  [[nodiscard]] static auto TranCoordToZoomCoeffIndexes(const Point2dInt& tranPoint) noexcept
      -> std::pair<uint32_t, uint32_t>;

  [[nodiscard]] auto NormalizedToTranPoint(const NormalizedCoords& normalizedPoint) const noexcept
      -> Point2dInt;

  [[nodiscard]] static auto TranToScreenPoint(const Point2dInt& tranPoint) noexcept -> Point2dInt;
  [[nodiscard]] static auto ScreenToTranPoint(const Point2dInt& screenPoint) noexcept -> Point2dInt;

private:
  const NormalizedCoordsConverter& m_normalizedCoordsConverter;
  [[nodiscard]] static auto ScreenToTranCoord(float screenCoord) noexcept -> uint32_t;
};

inline ZoomCoordTransforms::ZoomCoordTransforms(
    const NormalizedCoordsConverter& normalizedCoordsConverter) noexcept
  : m_normalizedCoordsConverter{normalizedCoordsConverter}
{
}

inline auto ZoomCoordTransforms::TranToScreenPoint(const Point2dInt& tranPoint) noexcept
    -> Point2dInt
{
  return {tranPoint.x >> ZoomFilterCoefficients::DIM_FILTER_COEFFS_EXP,
          tranPoint.y >> ZoomFilterCoefficients::DIM_FILTER_COEFFS_EXP};
}

inline auto ZoomCoordTransforms::ScreenToTranPoint(const Point2dInt& screenPoint) noexcept
    -> Point2dInt
{
  return {screenPoint.x << ZoomFilterCoefficients::DIM_FILTER_COEFFS_EXP,
          screenPoint.y << ZoomFilterCoefficients::DIM_FILTER_COEFFS_EXP};
}

inline auto ZoomCoordTransforms::ScreenToTranCoord(const float screenCoord) noexcept -> uint32_t
{
  // IMPORTANT: Without 'lround' a faint cross artifact appears in the centre of the screen.
  return static_cast<uint32_t>(
      std::lround(screenCoord * static_cast<float>(ZoomFilterCoefficients::DIM_FILTER_COEFFS)));
}

inline auto ZoomCoordTransforms::TranCoordToZoomCoeffIndexes(const Point2dInt& tranPoint) noexcept
    -> std::pair<uint32_t, uint32_t>
{
  return {static_cast<uint32_t>(tranPoint.x) & ZoomFilterCoefficients::DIM_FILTER_COEFFS_MOD_MASK,
          static_cast<uint32_t>(tranPoint.y) & ZoomFilterCoefficients::DIM_FILTER_COEFFS_MOD_MASK};
}

inline auto ZoomCoordTransforms::NormalizedToTranPoint(
    const NormalizedCoords& normalizedPoint) const noexcept -> Point2dInt
{
  const auto screenCoordsFlt =
      m_normalizedCoordsConverter.NormalizedToScreenCoordsFlt(normalizedPoint);

  return {ScreenToTranCoord(screenCoordsFlt.x), ScreenToTranCoord(screenCoordsFlt.y)};
}

} // namespace GOOM::FILTER_FX::FILTER_UTILS
