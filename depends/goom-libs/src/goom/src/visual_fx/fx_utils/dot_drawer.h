#pragma once

#include "utils/goom_rand_base.h"

#include <vector>

namespace GOOM
{
class Pixel;
class V2dInt;

namespace DRAW
{
class IGoomDraw;
}
namespace UTILS
{
class ImageBitmap;
class SmallImageBitmaps;
}

namespace VISUAL_FX
{
namespace FX_UTILS
{

class DotDrawer
{
public:
  DotDrawer(DRAW::IGoomDraw& draw,
            UTILS::IGoomRand& goomRand,
            const UTILS::SmallImageBitmaps& smallBitmaps) noexcept;

  void ChangeDotSizes();

  void DrawDots(const V2dInt& pt, const std::vector<Pixel>& colors, float brightness);

private:
  DRAW::IGoomDraw& m_goomDraw;
  GOOM::UTILS::IGoomRand& m_goomRand;
  const GOOM::UTILS::SmallImageBitmaps& m_smallBitmaps;

  static constexpr size_t MIN_IMAGE_DOT_SIZE = 3;
  static constexpr size_t MAX_IMAGE_DOT_SIZE = 15;
  size_t m_currentDotSize = MIN_IMAGE_DOT_SIZE;
  bool m_beadedLook = false;
  const UTILS::Weights<size_t> m_dotSizesMin;
  const UTILS::Weights<size_t> m_dotSizes;
  [[nodiscard]] auto GetNextDotSize(size_t maxSize) const -> size_t;
  [[nodiscard]] auto GetImageBitmap(size_t size) const -> const UTILS::ImageBitmap&;
};

} // namespace FX_UTILS
} // namespace VISUAL_FX
} // namespace GOOM
