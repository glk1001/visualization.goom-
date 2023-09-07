#pragma once

#include "color/color_maps.h"
#include "color/color_maps_base.h"
#include "color/random_color_maps.h"
#include "draw/goom_draw.h"
#include "draw/shape_drawers/bitmap_drawer.h"
#include "draw/shape_drawers/circle_drawer.h"
#include "goom/goom_graphic.h"
#include "goom/goom_types.h"
#include "goom/point2d.h"
#include "helper.h"
#include "utils/enum_utils.h"
#include "utils/math/goom_rand_base.h"

#include <cstdint>

namespace GOOM::VISUAL_FX::CIRCLES
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
    _num // unused, and marks the enum end
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
