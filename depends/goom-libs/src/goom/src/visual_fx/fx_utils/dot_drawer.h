#pragma once

#include "draw/goom_draw.h"
#include "utils/enum_utils.h"
#include "utils/math/goom_rand_base.h"

#include <vector>

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

enum class DotSizes
{
  DOT_SIZE01,
  DOT_SIZE02,
  DOT_SIZE03,
  DOT_SIZE04,
  DOT_SIZE05,
  DOT_SIZE06,
  DOT_SIZE07,
  DOT_SIZE08,
  _num // unused, and marks the enum end
};

class DotDrawer
{
public:
  DotDrawer(DRAW::IGoomDraw& draw,
            const UTILS::MATH::IGoomRand& goomRand,
            const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps,
            const UTILS::MATH::Weights<DotSizes>& minDotSizes,
            const UTILS::MATH::Weights<DotSizes>& normalDotSizes) noexcept;

  void ChangeDotSizes();

  void DrawDot(const Point2dInt& point, const DRAW::MultiplePixels& colors, float brightness);
  void DrawDot(DotSizes dotSize,
               const Point2dInt& point,
               const DRAW::MultiplePixels& colors,
               float brightness);

private:
  DRAW::IGoomDraw& m_goomDraw;
  const GOOM::UTILS::MATH::IGoomRand& m_goomRand;
  const GOOM::UTILS::GRAPHICS::SmallImageBitmaps& m_smallBitmaps;

  static constexpr size_t MIN_IMAGE_DOT_SIZE = 3;
  static constexpr size_t MAX_IMAGE_DOT_SIZE = 15;
  size_t m_currentDotSize                    = MIN_IMAGE_DOT_SIZE;
  bool m_beadedLook                          = false;
  const UTILS::MATH::Weights<DotSizes> m_minDotSizes;
  const UTILS::MATH::Weights<DotSizes> m_normalDotSizes;
  [[nodiscard]] auto GetNextDotSize(size_t maxSize) const -> size_t;
  [[nodiscard]] auto GetImageBitmap(size_t size) const -> const UTILS::GRAPHICS::ImageBitmap&;

  void DrawDot(size_t dotSize,
               const Point2dInt& point,
               const DRAW::MultiplePixels& colors,
               float brightness);

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
};

} // namespace VISUAL_FX::FX_UTILS
} // namespace GOOM
