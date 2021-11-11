#include "dot_drawer.h"

#include "draw/goom_draw.h"
#include "utils/goom_rand_base.h"
#include "utils/graphics/image_bitmaps.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/mathutils.h"
#include "v2d.h"

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
namespace FX_UTILS
{
#else
namespace GOOM::VISUAL_FX::FX_UTILS
{
#endif

using COLOR::GetColorMultiply;
using DRAW::IGoomDraw;
using UTILS::IGoomRand;
using UTILS::ImageBitmap;
using UTILS::SmallImageBitmaps;

DotDrawer::DotDrawer(DRAW::IGoomDraw& draw,
                     UTILS::IGoomRand& goomRand,
                     const UTILS::SmallImageBitmaps& smallBitmaps,
                     const UTILS::Weights<size_t>& minDotSizes,
                     const UTILS::Weights<size_t>& normalDotSizes) noexcept
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
  constexpr size_t MAX_DOT_SIZE = 7;
  m_currentDotSize = GetNextDotSize(MAX_DOT_SIZE);
  m_beadedLook = m_goomRand.ProbabilityOfMInN(3, 20);
}

void DotDrawer::DrawDots(const V2dInt& pt, const std::vector<Pixel>& colors, const float brightness)
{
  size_t dotSize = m_currentDotSize;
  if (m_beadedLook)
  {
    dotSize = GetNextDotSize(MAX_IMAGE_DOT_SIZE);
  }

  if (dotSize > 1)
  {
    const auto getColor1 =
        [=]([[maybe_unused]] const size_t x, [[maybe_unused]] const size_t y, const Pixel& bgnd)
    { return GetColorMultiply(bgnd, COLOR::GetBrighterColor(brightness, colors[0])); };

    const auto getColor2 =
        [=]([[maybe_unused]] const size_t x, [[maybe_unused]] const size_t y, const Pixel& bgnd)
    { return GetColorMultiply(bgnd, COLOR::GetBrighterColor(brightness, colors[1])); };

    const std::vector<IGoomDraw::GetBitmapColorFunc> getColors{getColor1, getColor2};
    const ImageBitmap& bitmap = GetImageBitmap(m_currentDotSize);

    m_goomDraw.Bitmap(pt.x, pt.y, bitmap, getColors);
  }
}

auto DotDrawer::GetNextDotSize(const size_t maxSize) const -> size_t
{
  constexpr size_t MAX_MIN_DOT_SIZE = 7;
  if (maxSize <= MAX_MIN_DOT_SIZE)
  {
    return m_minDotSizes.GetRandomWeighted();
  }
  return m_normalDotSizes.GetRandomWeighted();
}

inline auto DotDrawer::GetImageBitmap(const size_t size) const -> const ImageBitmap&
{
  return m_smallBitmaps.GetImageBitmap(SmallImageBitmaps::ImageNames::CIRCLE,
                                       stdnew::clamp(size, MIN_IMAGE_DOT_SIZE, MAX_IMAGE_DOT_SIZE));
}

#if __cplusplus <= 201402L
} // namespace FX_UTILS
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::FX_UTILS
#endif
