#include "image_displacement.h"

#include "filter_buffers.h"
#include "goom_graphic.h"
#include "normalized_coords.h"
#include "point2d.h"
#include "utils/graphics/image_bitmaps.h"

#include <cmath>
#include <cstdint>
#include <memory>

namespace GOOM::VISUAL_FX::FILTERS
{

using UTILS::GRAPHICS::ImageBitmap;
using UTILS::MATH::IGoomRand;

ImageDisplacement::ImageDisplacement(const std::string& imageFilename,
                                     [[maybe_unused]] const IGoomRand& goomRand)
  : m_imageBuffer(std::make_unique<ImageBitmap>(imageFilename)),
    m_imageFilename{imageFilename},
    m_xMax{static_cast<int32_t>(m_imageBuffer->GetWidth() - 1)},
    m_yMax{static_cast<int32_t>(m_imageBuffer->GetHeight() - 1)},
    m_normalizedCoordsConverter{m_imageBuffer->GetWidth(), m_imageBuffer->GetHeight(),
                                ZoomFilterBuffers::MIN_SCREEN_COORD_ABS_VAL}
{
}

auto ImageDisplacement::GetDisplacementVector(const NormalizedCoords& normalizedCoords) const
    -> Point2dFlt
{
  const Point2dInt imagePoint = NormalizedCoordsToImagePoint(normalizedCoords);

  if ((imagePoint.x < 0) || (imagePoint.x > m_xMax))
  {
    return {0.0F, 0.0F};
  }
  if ((imagePoint.y < 0) || (imagePoint.y > m_yMax))
  {
    return {0.0F, 0.0F};
  }

  const Pixel color =
      (*m_imageBuffer)(static_cast<size_t>(imagePoint.x), static_cast<size_t>(imagePoint.y));

  return ColorToNormalizedDisplacement(color);
}

inline auto ImageDisplacement::NormalizedCoordsToImagePoint(
    const NormalizedCoords& normalizedCoords) const -> Point2dInt
{
  const NormalizedCoords normalizedZoom = m_zoomFactor * normalizedCoords;
  return m_normalizedCoordsConverter.NormalizedToScreenCoordsFlt(normalizedZoom).ToInt();
}

inline auto ImageDisplacement::ColorToNormalizedDisplacement(const Pixel& color) const -> Point2dFlt
{
  const float normalizedDisplacementX =
      NormalizedCoords::MAX_NORMALIZED_COORD * m_amplitude * (color.RFlt() - m_xColorCutoff);
  const float normalizedDisplacementY =
      NormalizedCoords::MAX_NORMALIZED_COORD * m_amplitude * (color.GFlt() - m_yColorCutoff);
  //const float normalizedDisplacementY =
  //         (ProbabilityOfMInN(1, 2) ? color.GFlt() : color.BFlt()) - 0.5F;

  return {normalizedDisplacementX, normalizedDisplacementY};
}

} // namespace GOOM::VISUAL_FX::FILTERS
