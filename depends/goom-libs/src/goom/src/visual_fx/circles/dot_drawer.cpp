#include "dot_drawer.h"

#include "color/color_utils.h"
#include "goom_config.h"
#include "utils/math/misc.h"
#include "utils/t_values.h"

namespace GOOM::VISUAL_FX::CIRCLES
{

using COLOR::GetBrighterColor;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using DRAW::MultiplePixels;
using UTILS::IncrementedValue;
using UTILS::TValue;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::U_HALF;

static constexpr auto PROB_CIRCLES                          = 0.5F;
static constexpr auto DOT_INSIDE_MIN_BRIGHTNESS_FACTOR      = 0.5F;
static constexpr auto DOT_INSIDE_MAX_BRIGHTNESS_FACTOR      = 1.0F;
static constexpr auto DECORATION_DIFFERENT_COLOR_BRIGHTNESS = 1.0F;
static constexpr auto DECORATION_SPECIAL_BRIGHTNESS         = 2.0F;

DotDrawer::DotDrawer(DRAW::IGoomDraw& draw,
                     const IGoomRand& goomRand,
                     const Helper& helper) noexcept
  : m_goomRand{&goomRand},
    m_helper{&helper},
    m_bitmapDrawer{draw},
    m_circleDrawer{draw},
    m_bgndMainColorMixT{m_goomRand->GetRandInRange(MIN_BGND_MIX_T, MAX_BGND_MIX_T)},
    m_bgndLowColorMixT{m_goomRand->GetRandInRange(MIN_BGND_MIX_T, MAX_BGND_MIX_T)},
    m_decorationType{GetRandomDecorationType()},
    m_differentColor{GetRandomDifferentColor(RandomColorMaps{*m_goomRand})}
{
}

auto DotDrawer::SetWeightedColorMaps(const RandomColorMaps& weightedMaps) noexcept -> void
{
  m_bgndMainColorMixT      = m_goomRand->GetRandInRange(MIN_BGND_MIX_T, MAX_BGND_MIX_T);
  m_bgndLowColorMixT       = m_goomRand->GetRandInRange(MIN_BGND_MIX_T, MAX_BGND_MIX_T);
  m_decorationType         = GetRandomDecorationType();
  m_differentColor         = GetRandomDifferentColor(weightedMaps);
  m_outerCircleDotColorMap = &weightedMaps.GetRandomColorMap();

  m_doCircleDotShapes      = m_goomRand->ProbabilityOf(PROB_CIRCLES);
  m_outerCircleDotColorMix = m_goomRand->GetRandInRange(MIN_OUTER_CIRCLE_DOT_COLOR_MIX_T,
                                                        MAX_OUTER_CIRCLE_DOT_COLOR_MIX_T);
}

auto DotDrawer::DrawDot(const Point2dInt& pos,
                        const uint32_t diameter,
                        const MultiplePixels& colors) noexcept -> void
{
  if (m_doCircleDotShapes)
  {
    DrawCircleDot(pos, diameter, colors, *m_outerCircleDotColorMap);
  }
  else
  {
    DrawBitmapDot(pos, diameter, colors);
  }
}

inline auto DotDrawer::DrawCircleDot(const Point2dInt& centre,
                                     const uint32_t diameter,
                                     const MultiplePixels& colors,
                                     const IColorMap& innerColorMap) noexcept -> void
{
  const auto maxRadius = static_cast<int32_t>(diameter + 3) / 2;
  auto innerColorT =
      TValue{UTILS::TValue::StepType::SINGLE_CYCLE, static_cast<uint32_t>(maxRadius - 1)};
  static constexpr auto INNER_COLOR_CUTOFF_RADIUS = 4;

  const auto minBrightness = m_globalBrightnessFactor * DOT_INSIDE_MIN_BRIGHTNESS_FACTOR;
  const auto maxBrightness = m_globalBrightnessFactor * DOT_INSIDE_MAX_BRIGHTNESS_FACTOR;
  auto brightness          = IncrementedValue{minBrightness,
                                     maxBrightness,
                                     TValue::StepType::SINGLE_CYCLE,
                                     static_cast<uint32_t>(maxRadius)};

  // '> 1' means leave a little hole in the middle of the circles.
  for (auto radius = maxRadius; radius > 1; --radius)
  {
    const auto innerColor = innerColorMap.GetColor(innerColorT());
    const auto circleColors =
        radius <= INNER_COLOR_CUTOFF_RADIUS
            ? GetCircleColors(brightness(), colors)
            : GetCircleColorsWithInner(brightness(), colors, innerColor, m_outerCircleDotColorMix);

    m_circleDrawer.DrawCircle(centre, radius, circleColors);

    brightness.Increment();
    innerColorT.Increment();
  }
}

inline auto DotDrawer::GetCircleColors(const float brightness,
                                       const MultiplePixels& colors) noexcept -> MultiplePixels
{
  return {
      GetBrighterColor(brightness, GetMainColor(colors)),
      GetBrighterColor(brightness, GetLowColor(colors)),
  };
}

inline auto DotDrawer::GetCircleColorsWithInner(const float brightness,
                                                const MultiplePixels& colors,
                                                const Pixel& innerColor,
                                                const float innerColorMix) noexcept
    -> MultiplePixels
{
  return {
      GetBrighterColor(brightness,
                       IColorMap::GetColorMix(GetMainColor(colors), innerColor, innerColorMix)),
      GetBrighterColor(brightness,
                       IColorMap::GetColorMix(GetLowColor(colors), innerColor, innerColorMix)),
  };
}

inline auto DotDrawer::DrawBitmapDot(const Point2dInt& position,
                                     const uint32_t diameter,
                                     const MultiplePixels& colors) noexcept -> void
{
  const auto getMainColor =
      [this, &colors, &diameter](const size_t x, const size_t y, const Pixel& bgnd)
  { return GetDotMixedColor(x, y, diameter, bgnd, GetMainColor(colors), m_bgndMainColorMixT); };

  const auto getLowColor =
      [this, &colors, &diameter](const size_t x, const size_t y, const Pixel& bgnd)
  { return GetDotMixedColor(x, y, diameter, bgnd, GetLowColor(colors), m_bgndLowColorMixT); };

  m_bitmapDrawer.Bitmap(
      position, m_helper->bitmapGetter->GetBitmap(diameter), {getMainColor, getLowColor});
}

inline auto DotDrawer::GetDotMixedColor(const size_t x,
                                        const size_t y,
                                        const uint32_t diameter,
                                        const Pixel& bgnd,
                                        const Pixel& color,
                                        const float mixT) const noexcept -> Pixel
{
  if (0 == bgnd.A())
  {
    return BLACK_PIXEL;
  }

  const auto mixedColor = IColorMap::GetColorMix(bgnd, color, mixT);

  if (not IsSpecialPoint(x, y, diameter))
  {
    return GetBrighterColor(m_globalBrightnessFactor, mixedColor);
  }

  switch (m_decorationType)
  {
    case DecorationType::NO_DECORATION:
      return GetBrighterColor(m_globalBrightnessFactor, mixedColor);
    case DecorationType::BLACK_LINES:
      return BLACK_PIXEL;
    case DecorationType::DIFFERENT_COLORS:
      return GetBrighterColor(m_globalBrightnessFactor * DECORATION_DIFFERENT_COLOR_BRIGHTNESS,
                              m_differentColor);
    case DecorationType::BRIGHT_LINES:
      return GetBrighterColor(m_globalBrightnessFactor * DECORATION_SPECIAL_BRIGHTNESS, mixedColor);
    default:
      FailFast();
  }
}

inline auto DotDrawer::IsSpecialPoint(const size_t x,
                                      const size_t y,
                                      const uint32_t diameter) noexcept -> bool
{
  if (static constexpr auto EDGE_CUTOFF = 3U; (x <= EDGE_CUTOFF) ||
                                              (x >= (diameter - EDGE_CUTOFF)) ||
                                              (y <= EDGE_CUTOFF) || (y >= (diameter - EDGE_CUTOFF)))
  {
    return false;
  }
  if (((U_HALF * diameter) == x) || ((U_HALF * diameter) == y))
  {
    return true;
  }
  if ((x == y) || ((diameter - x) == y))
  {
    return true;
  }

  return false;
}

} // namespace GOOM::VISUAL_FX::CIRCLES
