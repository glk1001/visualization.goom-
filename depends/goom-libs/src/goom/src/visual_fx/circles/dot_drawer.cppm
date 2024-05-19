module;

#include "goom/goom_config.h"

#include <cstdint>

export module Goom.VisualFx.CirclesFx.DotDrawer;

import Goom.Color.ColorMapBase;
import Goom.Color.ColorMaps;
import Goom.Color.ColorUtils;
import Goom.Color.RandomColorMaps;
import Goom.Draw.GoomDrawBase;
import Goom.Draw.ShapeDrawers.BitmapDrawer;
import Goom.Draw.ShaperDrawers.CircleDrawer;
import Goom.Utils.EnumUtils;
import Goom.Utils.Math.IncrementedValues;
import Goom.Utils.Math.Misc;
import Goom.Utils.Math.TValues;
import Goom.Utils.Math.GoomRandBase;
import Goom.VisualFx.CirclesFx.Helper;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

export namespace GOOM::VISUAL_FX::CIRCLES
{

class DotDrawer
{
public:
  DotDrawer(DRAW::IGoomDraw& draw,
            const UTILS::MATH::IGoomRand& goomRand,
            const Helper& helper) noexcept;

  auto SetWeightedColorMaps(const COLOR::RandomColorMaps& weightedMaps) noexcept -> void;
  auto SetGlobalBrightnessFactor(float val) noexcept -> void;

  auto DrawDot(const Point2dInt& pos,
               uint32_t diameter,
               const DRAW::MultiplePixels& colors) noexcept -> void;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  const Helper* m_helper;
  DRAW::SHAPE_DRAWERS::BitmapDrawer m_bitmapDrawer;
  DRAW::SHAPE_DRAWERS::CircleDrawer m_circleDrawer;
  float m_globalBrightnessFactor = 1.0F;

  static constexpr float MIN_BGND_MIX_T = 0.2F;
  static constexpr float MAX_BGND_MIX_T = 0.8F;
  float m_bgndMainColorMixT;
  float m_bgndLowColorMixT;
  enum class DecorationType : UnderlyingEnumType
  {
    NO_DECORATION,
    BLACK_LINES,
    BRIGHT_LINES,
    DIFFERENT_COLORS,
  };
  DecorationType m_decorationType;
  Pixel m_differentColor = BLACK_PIXEL;

  bool m_doCircleDotShapes = true;
  COLOR::ColorMapPtrWrapper m_outerCircleDotColorMap{nullptr};
  static constexpr float MIN_OUTER_CIRCLE_DOT_COLOR_MIX_T = 0.1F;
  static constexpr float MAX_OUTER_CIRCLE_DOT_COLOR_MIX_T = 0.9F;
  float m_outerCircleDotColorMix                          = MIN_OUTER_CIRCLE_DOT_COLOR_MIX_T;

  auto DrawBitmapDot(const Point2dInt& position,
                     uint32_t diameter,
                     const DRAW::MultiplePixels& colors) noexcept -> void;
  [[nodiscard]] auto GetRandomDifferentColor(
      const COLOR::RandomColorMaps& weightedMaps) const noexcept -> Pixel;
  [[nodiscard]] auto GetRandomDecorationType() const noexcept -> DecorationType;
  [[nodiscard]] static auto IsSpecialPoint(const Point2dInt& bitmapPoint,
                                           uint32_t diameter) noexcept -> bool;
  [[nodiscard]] auto GetDotMixedColor(const Point2dInt& bitmapPoint,
                                      uint32_t diameter,
                                      const Pixel& bgnd,
                                      const Pixel& color,
                                      float mixT) const noexcept -> Pixel;
  auto DrawCircleDot(const Point2dInt& centre,
                     uint32_t diameter,
                     const DRAW::MultiplePixels& colors,
                     const COLOR::IColorMap& innerColorMap) noexcept -> void;
  [[nodiscard]] static auto GetCircleColors(float brightness,
                                            const DRAW::MultiplePixels& colors) noexcept
      -> DRAW::MultiplePixels;
  [[nodiscard]] static auto GetCircleColorsWithInner(float brightness,
                                                     const DRAW::MultiplePixels& colors,
                                                     const Pixel& innerColor,
                                                     float innerColorMix) noexcept
      -> DRAW::MultiplePixels;
};

} // namespace GOOM::VISUAL_FX::CIRCLES

namespace GOOM::VISUAL_FX::CIRCLES
{

inline auto DotDrawer::SetGlobalBrightnessFactor(const float val) noexcept -> void
{
  m_globalBrightnessFactor = val;
}

inline auto DotDrawer::GetRandomDecorationType() const noexcept -> DecorationType
{
  return static_cast<DecorationType>(m_goomRand->GetRandInRange(0U, UTILS::NUM<DecorationType>));
}

inline auto DotDrawer::GetRandomDifferentColor(
    const COLOR::RandomColorMaps& weightedMaps) const noexcept -> Pixel
{
  return weightedMaps.GetRandomColorMap().GetColor(m_goomRand->GetRandInRange(0.0F, 1.0F));
}

} // namespace GOOM::VISUAL_FX::CIRCLES

module :private;

namespace GOOM::VISUAL_FX::CIRCLES
{

using COLOR::ColorMaps;
using COLOR::GetBrighterColor;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using DRAW::MultiplePixels;
using UTILS::MATH::I_HALF;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::IncrementedValue;
using UTILS::MATH::TValue;

static constexpr auto PROB_CIRCLES                          = 0.5F;
static constexpr auto DOT_INSIDE_MIN_BRIGHTNESS_FACTOR      = 1.0F;
static constexpr auto DOT_INSIDE_MAX_BRIGHTNESS_FACTOR      = 2.0F;
static constexpr auto DECORATION_DIFFERENT_COLOR_BRIGHTNESS = 2.0F;
static constexpr auto DECORATION_SPECIAL_BRIGHTNESS         = 4.0F;

DotDrawer::DotDrawer(DRAW::IGoomDraw& draw,
                     const IGoomRand& goomRand,
                     const Helper& helper) noexcept
  : m_goomRand{&goomRand},
    m_helper{&helper},
    m_bitmapDrawer{draw},
    m_circleDrawer{draw},
    m_bgndMainColorMixT{m_goomRand->GetRandInRange(MIN_BGND_MIX_T, MAX_BGND_MIX_T)},
    m_bgndLowColorMixT{m_goomRand->GetRandInRange(MIN_BGND_MIX_T, MAX_BGND_MIX_T)},
    m_decorationType{GetRandomDecorationType()}
{
}

auto DotDrawer::SetWeightedColorMaps(const RandomColorMaps& weightedMaps) noexcept -> void
{
  m_bgndMainColorMixT      = m_goomRand->GetRandInRange(MIN_BGND_MIX_T, MAX_BGND_MIX_T);
  m_bgndLowColorMixT       = m_goomRand->GetRandInRange(MIN_BGND_MIX_T, MAX_BGND_MIX_T);
  m_decorationType         = GetRandomDecorationType();
  m_differentColor         = GetRandomDifferentColor(weightedMaps);
  m_outerCircleDotColorMap = weightedMaps.GetRandomColorMap();

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
    DrawCircleDot(pos, diameter, colors, m_outerCircleDotColorMap);
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
  auto innerColorT     = TValue{
          {TValue::StepType::SINGLE_CYCLE, static_cast<uint32_t>(maxRadius - 1)}
  };
  static constexpr auto INNER_COLOR_CUTOFF_RADIUS = 4;

  const auto minBrightness = m_globalBrightnessFactor * DOT_INSIDE_MIN_BRIGHTNESS_FACTOR;
  const auto maxBrightness = m_globalBrightnessFactor * DOT_INSIDE_MAX_BRIGHTNESS_FACTOR;
  auto brightness          = IncrementedValue<float>{minBrightness,
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
                       ColorMaps::GetColorMix(GetMainColor(colors), innerColor, innerColorMix)),
      GetBrighterColor(brightness,
                       ColorMaps::GetColorMix(GetLowColor(colors), innerColor, innerColorMix)),
  };
}

inline auto DotDrawer::DrawBitmapDot(const Point2dInt& position,
                                     const uint32_t diameter,
                                     const MultiplePixels& colors) noexcept -> void
{
  const auto getMainColor =
      [this, &colors, &diameter](const Point2dInt& bitmapPoint, const Pixel& bgnd)
  {
    return GetDotMixedColor(bitmapPoint, diameter, bgnd, GetMainColor(colors), m_bgndMainColorMixT);
  };

  const auto getLowColor = [this, &colors, &diameter](const Point2dInt& bitmapPoint,
                                                      const Pixel& bgnd) {
    return GetDotMixedColor(bitmapPoint, diameter, bgnd, GetLowColor(colors), m_bgndLowColorMixT);
  };

  m_bitmapDrawer.Bitmap(
      position, m_helper->bitmapGetter->GetBitmap(diameter), {getMainColor, getLowColor});
}

inline auto DotDrawer::GetDotMixedColor(const Point2dInt& bitmapPoint,
                                        const uint32_t diameter,
                                        const Pixel& bgnd,
                                        const Pixel& color,
                                        const float mixT) const noexcept -> Pixel
{
  if (0 == bgnd.A())
  {
    return BLACK_PIXEL;
  }

  const auto mixedColor = ColorMaps::GetColorMix(bgnd, color, mixT);

  if (not IsSpecialPoint(bitmapPoint, diameter))
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
  }
}

inline auto DotDrawer::IsSpecialPoint(const Point2dInt& bitmapPoint,
                                      const uint32_t diameter) noexcept -> bool
{
  if (static constexpr auto EDGE_CUTOFF = 3;
      (bitmapPoint.x <= EDGE_CUTOFF) or
      (bitmapPoint.x >= (static_cast<int32_t>(diameter) - EDGE_CUTOFF)) or
      (bitmapPoint.y <= EDGE_CUTOFF) or
      (bitmapPoint.y >= (static_cast<int32_t>(diameter) - EDGE_CUTOFF)))
  {
    return false;
  }
  if (((I_HALF * static_cast<int32_t>(diameter)) == bitmapPoint.x) or
      ((I_HALF * static_cast<int32_t>(diameter)) == bitmapPoint.y))
  {
    return true;
  }
  if ((bitmapPoint.x == bitmapPoint.y) or
      ((static_cast<int32_t>(diameter) - bitmapPoint.x) == bitmapPoint.y))
  {
    return true;
  }

  return false;
}

} // namespace GOOM::VISUAL_FX::CIRCLES
