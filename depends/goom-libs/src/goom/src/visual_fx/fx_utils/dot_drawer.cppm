module;

#include "goom/point2d.h"

#include <algorithm>
#include <cstdint>
#include <vector>

export module Goom.VisualFx.FxUtils:DotDrawer;

import Goom.Color.ColorUtils;
import Goom.Draw.GoomDrawBase;
import Goom.Draw.ShapeDrawers.BitmapDrawer;
import Goom.Utils.EnumUtils;
import Goom.Utils.Graphics.ImageBitmaps;
import Goom.Utils.Graphics.PixelUtils;
import Goom.Utils.Graphics.SmallImageBitmaps;
import Goom.Utils.Math.GoomRandBase;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;

export namespace GOOM::VISUAL_FX::FX_UTILS
{

enum class DotSizes : UnderlyingEnumType
{
  DOT_SIZE01,
  DOT_SIZE02,
  DOT_SIZE03,
  DOT_SIZE04,
  DOT_SIZE05,
  DOT_SIZE06,
  DOT_SIZE07,
  DOT_SIZE08,
};

class DotDrawer
{
public:
  struct WeightProperties
  {
    UTILS::MATH::Weights<DotSizes> minDotSizes;
    UTILS::MATH::Weights<DotSizes> normalDotSizes;
  };

  DotDrawer(DRAW::IGoomDraw& draw,
            const UTILS::MATH::IGoomRand& goomRand,
            const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps,
            const WeightProperties& weightProperties) noexcept;

  auto ChangeDotSizes() noexcept -> void;

  auto DrawDot(const Point2dInt& point,
               const DRAW::MultiplePixels& colors,
               float brightness) noexcept -> void;
  auto DrawDot(DotSizes dotSize,
               const Point2dInt& point,
               const DRAW::MultiplePixels& colors,
               float brightness) noexcept -> void;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  const UTILS::GRAPHICS::SmallImageBitmaps* m_smallBitmaps;
  DRAW::SHAPE_DRAWERS::BitmapDrawer m_bitmapDrawer;

  static constexpr auto MIN_IMAGE_DOT_SIZE = 3U;
  static constexpr auto MAX_IMAGE_DOT_SIZE = 15U;
  uint32_t m_currentDotSize                = MIN_IMAGE_DOT_SIZE;
  bool m_beadedLook                        = false;
  uint32_t m_maxBeadedDotSize              = MAX_IMAGE_DOT_SIZE;
  UTILS::MATH::Weights<DotSizes> m_minDotSizes;
  UTILS::MATH::Weights<DotSizes> m_normalDotSizes;
  [[nodiscard]] auto GetNextDotSize(uint32_t maxSize) const noexcept -> uint32_t;
  [[nodiscard]] auto GetImageBitmap(uint32_t size) const noexcept
      -> const UTILS::GRAPHICS::ImageBitmap&;

  auto DrawDot(uint32_t dotSize,
               const Point2dInt& point,
               const DRAW::MultiplePixels& colors,
               float brightness) noexcept -> void;

  static constexpr UTILS::EnumMap<DotSizes, uint32_t> INT_DOT_SIZE{{{
      {DotSizes::DOT_SIZE01, 1U},
      {DotSizes::DOT_SIZE02, 3U},
      {DotSizes::DOT_SIZE03, 5U},
      {DotSizes::DOT_SIZE04, 7U},
      {DotSizes::DOT_SIZE05, 9U},
      {DotSizes::DOT_SIZE06, 11U},
      {DotSizes::DOT_SIZE07, 13U},
      {DotSizes::DOT_SIZE08, 15U},
  }}};
  static_assert(INT_DOT_SIZE[static_cast<DotSizes>(UTILS::NUM<DotSizes> - 1)] <=
                MAX_IMAGE_DOT_SIZE);
};

} // namespace GOOM::VISUAL_FX::FX_UTILS

namespace GOOM::VISUAL_FX::FX_UTILS
{

using COLOR::GetBrighterColor;
using DRAW::MultiplePixels;
using DRAW::SHAPE_DRAWERS::BitmapDrawer;
using UTILS::NUM;
using UTILS::GRAPHICS::GetColorMultiply;
using UTILS::GRAPHICS::ImageBitmap;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::IGoomRand;

DotDrawer::DotDrawer(DRAW::IGoomDraw& draw,
                     const IGoomRand& goomRand,
                     const SmallImageBitmaps& smallBitmaps,
                     const WeightProperties& weightProperties) noexcept
  : m_goomRand{&goomRand},
    m_smallBitmaps{&smallBitmaps},
    m_bitmapDrawer{draw},
    m_minDotSizes{weightProperties.minDotSizes},
    m_normalDotSizes{weightProperties.normalDotSizes}
{
  static_assert(MAX_IMAGE_DOT_SIZE <= SmallImageBitmaps::MAX_IMAGE_SIZE, "Max dot size mismatch.");
}

auto DotDrawer::ChangeDotSizes() noexcept -> void
{
  static constexpr auto MAX_DOT_SIZE = 7U;
  static_assert(MAX_DOT_SIZE <= (MAX_IMAGE_DOT_SIZE - 1U));
  m_currentDotSize = GetNextDotSize(MAX_DOT_SIZE);

  static constexpr auto PROB_BEADED_LOOK = 0.15F;
  m_beadedLook                           = m_goomRand->ProbabilityOf(PROB_BEADED_LOOK);
  m_maxBeadedDotSize = m_goomRand->GetRandInRange(MAX_DOT_SIZE + 1, MAX_IMAGE_DOT_SIZE + 1);
}

auto DotDrawer::DrawDot(const DotSizes dotSize,
                        const Point2dInt& point,
                        const DRAW::MultiplePixels& colors,
                        const float brightness) noexcept -> void
{
  DrawDot(INT_DOT_SIZE[dotSize], point, colors, brightness);
}

auto DotDrawer::DrawDot(const Point2dInt& point,
                        const MultiplePixels& colors,
                        const float brightness) noexcept -> void
{
  const auto dotSize = not m_beadedLook ? m_currentDotSize : GetNextDotSize(m_maxBeadedDotSize);

  DrawDot(dotSize, point, colors, brightness);
}

inline auto DotDrawer::DrawDot(const uint32_t dotSize,
                               const Point2dInt& point,
                               const MultiplePixels& colors,
                               const float brightness) noexcept -> void
{
  if (dotSize <= 1)
  {
    return;
  }

  const auto getColor1 = [&brightness, &colors]([[maybe_unused]] const Point2dInt& bitmapPoint,
                                                const Pixel& bgnd) {
    return GetColorMultiply(bgnd, GetBrighterColor(brightness, colors.color1), colors.color1.A());
  };

  const auto getColor2 = [&brightness, &colors]([[maybe_unused]] const Point2dInt& bitmapPoint,
                                                const Pixel& bgnd) {
    return GetColorMultiply(bgnd, GetBrighterColor(brightness, colors.color2), colors.color2.A());
  };

  const auto getColors = std::vector<BitmapDrawer::GetBitmapColorFunc>{getColor1, getColor2};
  const auto& bitmap   = GetImageBitmap(dotSize);

  m_bitmapDrawer.Bitmap(point, bitmap, getColors);
}

auto DotDrawer::GetNextDotSize(const uint32_t maxSize) const noexcept -> uint32_t
{
  if (static constexpr auto MAX_MIN_DOT_SIZE = 7U; maxSize <= MAX_MIN_DOT_SIZE)
  {
    return INT_DOT_SIZE[m_minDotSizes.GetRandomWeighted()];
  }
  return INT_DOT_SIZE[m_normalDotSizes.GetRandomWeighted()];
}

inline auto DotDrawer::GetImageBitmap(const uint32_t size) const noexcept -> const ImageBitmap&
{
  return m_smallBitmaps->GetImageBitmap(SmallImageBitmaps::ImageNames::CIRCLE,
                                        std::clamp(size, MIN_IMAGE_DOT_SIZE, MAX_IMAGE_DOT_SIZE));
}

} // namespace GOOM::VISUAL_FX::FX_UTILS
