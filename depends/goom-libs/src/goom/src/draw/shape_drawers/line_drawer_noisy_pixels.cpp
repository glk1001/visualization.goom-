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
                                                          const IGoomRand& goomRand) noexcept
  : m_draw{draw}, m_goomRand{goomRand}
{
}

auto LineDrawerNoisyPixels::NoisyPixelDrawer::DrawPixels(const Point2dInt& point,
                                                         const float brightness,
                                                         MultiplePixels colors) noexcept -> void
{
  BrightenColors(m_brightnessReducer * brightness, colors);

  if (0 == m_noiseRadius)
  {
    m_draw.DrawClippedPixels(point, colors);
    return;
  }

  for (auto i = 0; i < m_numNoisePixels; ++i)
  {
    const auto noisyPoint =
        Point2dInt{point.x + m_goomRand.GetRandInRange(-m_noiseRadius, m_noiseRadius + 1),
                   point.y + m_goomRand.GetRandInRange(-m_noiseRadius, m_noiseRadius + 1)};

    m_draw.DrawClippedPixels(noisyPoint, colors);
  }
}

// clang-format off
LineDrawerNoisyPixels::LineDrawerNoisyPixels(IGoomDraw& draw, const IGoomRand& goomRand) noexcept
  : m_lineDrawer{NoisyPixelDrawer{draw, goomRand}}
{
}
// clang-format on

} // namespace GOOM::DRAW::SHAPE_DRAWERS
