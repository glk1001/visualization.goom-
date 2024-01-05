#pragma once

#include "draw/goom_draw.h"
#include "draw/shape_drawers/bitmap_drawer.h"
#include "goom/goom_types.h"
#include "utils/enum_utils.h"
#include "utils/math/goom_rand_base.h"

#include <cstdint>

namespace GOOM
{
class Pixel;
struct Point2dInt;

namespace DRAW
{
class IGoomDraw;
}

namespace UTILS::GRAPHICS
{
class ImageBitmap;
class SmallImageBitmaps;
}

namespace VISUAL_FX::FX_UTILS
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

} // namespace VISUAL_FX::FX_UTILS
} // namespace GOOM
