//#undef NO_LOGGING

#include "line_drawer_noisy_pixels.h"

#include "debugging_logger.h"
#include "drawer_utils.h"
#include "goom_config.h"
#include "goom_logger.h"

namespace GOOM::DRAW::SHAPE_DRAWERS
{

using UTILS::MATH::IGoomRand;


LineDrawerNoisyPixels::NoisyPixelDrawer::NoisyPixelDrawer(IGoomDraw& draw,
                                                          const IGoomRand& goomRand,
                                                          const uint8_t noiseRadius,
                                                          const uint8_t numNoisePixels) noexcept
  : m_draw{draw}, m_goomRand{goomRand}, m_noiseRadius{noiseRadius}, m_numNoisePixels{numNoisePixels}
{
}

auto LineDrawerNoisyPixels::NoisyPixelDrawer::DrawPixels(const Point2dInt& point,
                                                         const float brightness,
                                                         MultiplePixels colors) noexcept -> void
{
  Expects(m_useMainColorsForNoise or (not m_noiseColors.empty()));

  if (m_useMainPointWithoutNoise or (0 == m_noiseRadius))
  {
    BrightenColors(brightness, colors);
    m_draw.DrawClippedPixels(point, colors);

    if (0 == m_noiseRadius)
    {
      return;
    }
  }

  if (not m_useMainColorsForNoise)
  {
    colors = m_noiseColors;
  }

  BrightenColors(m_overallBrightnessFactor * brightness, colors);

  for (auto i = 0; i < m_numNoisePixels; ++i)
  {
    const auto noisyPoint =
        Point2dInt{point.x + m_goomRand.GetRandInRange(-m_noiseRadius, m_noiseRadius + 1),
                   point.y + m_goomRand.GetRandInRange(-m_noiseRadius, m_noiseRadius + 1)};

    m_draw.DrawClippedPixels(noisyPoint, colors);
  }
}

// clang-format off
LineDrawerNoisyPixels::LineDrawerNoisyPixels(IGoomDraw& draw,
                                             const IGoomRand& goomRand,
                                             const uint8_t noiseRadius,
                                             const uint8_t numNoisePixels) noexcept
  : m_lineDrawer{NoisyPixelDrawer{draw, goomRand, noiseRadius, numNoisePixels}}
{
}
// clang-format on

} // namespace GOOM::DRAW::SHAPE_DRAWERS
