module;

//#undef NO_LOGGING

#include "draw/goom_draw.h"
#include "goom/point2d.h"

#include <cstddef>

module Goom.Draw.ShaperDrawers.LineDrawerNoisyPixels;

import Goom.Draw.ShaperDrawers.DrawerUtils;
import Goom.Utils.Math.Misc;
import Goom.Utils.Math.GoomRandBase;

namespace GOOM::DRAW::SHAPE_DRAWERS
{

using UTILS::MATH::IGoomRand;


LineDrawerNoisyPixels::NoisyPixelDrawer::NoisyPixelDrawer(IGoomDraw& draw,
                                                          const IGoomRand& goomRand,
                                                          const NoiseParams& noiseParams) noexcept
  : m_draw{&draw},
    m_goomRand{&goomRand},
    m_noiseRadius{noiseParams.noiseRadius},
    m_numNoisePixelsPerPixel{noiseParams.numNoisePixelsPerPixel}
{
  SetNoisePerPixel();
}

auto LineDrawerNoisyPixels::NoisyPixelDrawer::SetNoisePerPixel() noexcept -> void
{
  if (m_usePureNoise)
  {
    return;
  }

  for (auto& noiseList : m_noisePerPixelList)
  {
    noiseList.resize(static_cast<size_t>(m_numNoisePixelsPerPixel));
    for (auto& noise : noiseList)
    {
      noise = m_goomRand->GetRandInRange(-m_noiseRadius, m_noiseRadius + 1);
    }
  }
}

auto LineDrawerNoisyPixels::NoisyPixelDrawer::DrawPixels(const Point2dInt& point,
                                                         const float brightness,
                                                         MultiplePixels colors) noexcept -> void
{
  if (0 == m_noiseRadius)
  {
    DrawMainPoint(point, brightness, colors);
    return;
  }

  if (m_useMainPointWithoutNoise)
  {
    DrawMainPoint(point, brightness, colors);
  }

  DrawNoisePoints(point, brightness, colors);
}

inline auto LineDrawerNoisyPixels::NoisyPixelDrawer::DrawMainPoint(const Point2dInt& point,
                                                                   const float brightness,
                                                                   MultiplePixels& colors) noexcept
    -> void
{
  BrightenColors(brightness, colors);
  m_draw->DrawClippedPixels(point, colors);
}

inline auto LineDrawerNoisyPixels::NoisyPixelDrawer::DrawNoisePoints(
    const Point2dInt& point, const float brightness, MultiplePixels& colors) noexcept -> void
{
  if (not m_useMainColorsForNoise)
  {
    colors = m_noiseColors;
  }
  BrightenColors(m_overallBrightnessFactor * brightness, colors);

  if (m_usePureNoise)
  {
    DrawPureNoisePoints(point, colors);
  }
  else
  {
    DrawPatternedNoisePoints(point, colors);
  }
}

inline auto LineDrawerNoisyPixels::NoisyPixelDrawer::DrawPureNoisePoints(
    const Point2dInt& point, const MultiplePixels& colors) noexcept -> void
{
  for (auto i = 0; i < m_numNoisePixelsPerPixel; ++i)
  {
    const auto noisePoint =
        Point2dInt{point.x + m_goomRand->GetRandInRange(-m_noiseRadius, +m_noiseRadius + 1),
                   point.y + m_goomRand->GetRandInRange(-m_noiseRadius, +m_noiseRadius + 1)};
    m_draw->DrawClippedPixels(noisePoint, colors);
  }
}

inline auto LineDrawerNoisyPixels::NoisyPixelDrawer::DrawPatternedNoisePoints(
    const Point2dInt& point, const MultiplePixels& colors) noexcept -> void
{
  for (const auto& noise : m_noisePerPixelList.at(m_currentNoisePerPixelIndex))
  {
    m_draw->DrawClippedPixels(point + noise, colors);
  }

  IncrementCurrentNoisePerPixelIndex();
}

inline auto LineDrawerNoisyPixels::NoisyPixelDrawer::IncrementCurrentNoisePerPixelIndex() noexcept
    -> void
{
  ++m_currentNoisePerPixelIndex;
  if (m_currentNoisePerPixelIndex >= m_noisePerPixelList.size())
  {
    m_currentNoisePerPixelIndex = 0U;
  }
}

// clang-format off
LineDrawerNoisyPixels::LineDrawerNoisyPixels(IGoomDraw& draw,
                                             const IGoomRand& goomRand,
                                             const NoiseParams& noiseParams) noexcept
  : m_lineDrawer{NoisyPixelDrawer{draw, goomRand, noiseParams}}
{
}
// clang-format on

} // namespace GOOM::DRAW::SHAPE_DRAWERS
