module;

#include <cstddef>
#include <memory>
#include <string>

module Goom.FilterFx.FilterUtils.ImageDisplacement;

import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.Graphics.ImageBitmaps;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.GoomGraphic;
import Goom.Lib.Point2d;

namespace GOOM::FILTER_FX::FILTER_UTILS
{

using UTILS::GRAPHICS::ImageBitmap;
using UTILS::MATH::GoomRand;

ImageDisplacement::ImageDisplacement(const std::string& imageFilename,
                                     [[maybe_unused]] const GoomRand& goomRand)
  : m_imageBuffer(std::make_unique<ImageBitmap>(imageFilename)), m_imageFilename{imageFilename}
{
}

auto ImageDisplacement::GetDisplacementVector(
    const NormalizedCoords& normalizedCoords) const noexcept -> Vec2dFlt
{
  const auto imagePoint = NormalizedCoordsToImagePoint(normalizedCoords);

  if ((imagePoint.x < 0) || (imagePoint.x > m_xMax))
  {
    return {.x = 0.0F, .y = 0.0F};
  }
  if ((imagePoint.y < 0) || (imagePoint.y > m_yMax))
  {
    return {.x = 0.0F, .y = 0.0F};
  }

  const auto color =
      (*m_imageBuffer)(static_cast<size_t>(imagePoint.x), static_cast<size_t>(imagePoint.y));

  return ColorToNormalizedDisplacement(color);
}

inline auto ImageDisplacement::NormalizedCoordsToImagePoint(
    const NormalizedCoords& normalizedCoords) const noexcept -> Point2dInt
{
  const auto normalizedZoom = NormalizedCoords{m_xZoomFactor * normalizedCoords.GetX(),
                                               m_yZoomFactor * normalizedCoords.GetY()};
  return ToPoint2dInt(m_normalizedCoordsConverter.NormalizedToOtherCoordsFlt(normalizedZoom));
}

inline auto ImageDisplacement::ColorToNormalizedDisplacement(const Pixel& color) const noexcept
    -> Vec2dFlt
{
  const auto normalizedDisplacementX =
      NormalizedCoords::MAX_COORD * (m_amplitude.x * (color.RFlt() - m_xColorCutoff));
  const auto normalizedDisplacementY =
      NormalizedCoords::MAX_COORD * (m_amplitude.y * (color.GFlt() - m_yColorCutoff));
  //const auto normalizedDisplacementY =
  //         (ProbabilityOfMInN(1, 2) ? color.GFlt() : color.BFlt()) - 0.5F;

  return {.x = normalizedDisplacementX, .y = normalizedDisplacementY};
}

} // namespace GOOM::FILTER_FX::FILTER_UTILS
