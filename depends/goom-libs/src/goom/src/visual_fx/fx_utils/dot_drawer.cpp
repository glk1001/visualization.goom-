#include "dot_drawer.h"

#include "draw/goom_draw.h"
#include "point2d.h"
#include "utils/enumutils.h"
#include "utils/graphics/image_bitmaps.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"

#include <array>

namespace GOOM::VISUAL_FX::FX_UTILS
{

using COLOR::GetColorMultiply;
using DRAW::IGoomDraw;
using UTILS::NUM;
using UTILS::GRAPHICS::ImageBitmap;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::Weights;

DotDrawer::DotDrawer(DRAW::IGoomDraw& draw,
                     const IGoomRand& goomRand,
                     const SmallImageBitmaps& smallBitmaps,
                     const Weights<DotSizes>& minDotSizes,
                     const Weights<DotSizes>& normalDotSizes) noexcept
  : m_goomDraw{draw},
    m_goomRand{goomRand},
    m_smallBitmaps{smallBitmaps},
    m_minDotSizes{minDotSizes},
    m_normalDotSizes{normalDotSizes}
{
  static_assert(MAX_IMAGE_DOT_SIZE <= SmallImageBitmaps::MAX_IMAGE_SIZE, "Max dot size mismatch.");
}

void DotDrawer::ChangeDotSizes()
{
  static constexpr size_t MAX_DOT_SIZE = 7;
  m_currentDotSize = GetNextDotSize(MAX_DOT_SIZE);
  static constexpr float PROB_BEADED_LOOK = 0.15F;
  m_beadedLook = m_goomRand.ProbabilityOf(PROB_BEADED_LOOK);
}

void DotDrawer::DrawDot(const Point2dInt& pt,
                        const std::vector<Pixel>& colors,
                        const float brightness)
{
  size_t dotSize = m_currentDotSize;
  if (m_beadedLook)
  {
    dotSize = GetNextDotSize(MAX_IMAGE_DOT_SIZE);
  }
  if (dotSize <= 1)
  {
    return;
  }

  const auto getColor1 = [&brightness, &colors]([[maybe_unused]] const size_t x,
                                                [[maybe_unused]] const size_t y, const Pixel& bgnd)
  { return GetColorMultiply(bgnd, COLOR::GetBrighterColor(brightness, colors[0])); };

  const auto getColor2 = [&brightness, &colors]([[maybe_unused]] const size_t x,
                                                [[maybe_unused]] const size_t y, const Pixel& bgnd)
  { return GetColorMultiply(bgnd, COLOR::GetBrighterColor(brightness, colors[1])); };

  const std::vector<IGoomDraw::GetBitmapColorFunc> getColors{getColor1, getColor2};
  const ImageBitmap& bitmap = GetImageBitmap(m_currentDotSize);

  m_goomDraw.Bitmap(pt.x, pt.y, bitmap, getColors);
}

auto DotDrawer::GetNextDotSize(const size_t maxSize) const -> size_t
{
  // clang-format off
  static const std::array<size_t, NUM<DotSizes>> s_DOT_SIZES = {{
      1,
      3,
      5,
      7,
      9,
     11,
     13,
     15,
  }};
  // clang-format on

  if (constexpr size_t MAX_MIN_DOT_SIZE = 7; maxSize <= MAX_MIN_DOT_SIZE)
  {
    return s_DOT_SIZES.at(static_cast<size_t>(m_minDotSizes.GetRandomWeighted()));
  }
  return s_DOT_SIZES.at(static_cast<size_t>(m_normalDotSizes.GetRandomWeighted()));
}

inline auto DotDrawer::GetImageBitmap(const size_t size) const -> const ImageBitmap&
{
  return m_smallBitmaps.GetImageBitmap(SmallImageBitmaps::ImageNames::CIRCLE,
                                       std::clamp(size, MIN_IMAGE_DOT_SIZE, MAX_IMAGE_DOT_SIZE));
}

} // namespace GOOM::VISUAL_FX::FX_UTILS
