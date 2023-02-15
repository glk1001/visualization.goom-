#pragma once

#include "draw/goom_draw.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "point2d.h"

#include <cstdint>
#include <functional>
#include <vector>

namespace GOOM
{

namespace UTILS::GRAPHICS
{
class ImageBitmap;
}

namespace DRAW::SHAPE_DRAWERS
{

class BitmapDrawer
{
public:
  explicit BitmapDrawer(IGoomDraw& draw) noexcept;

  using GetBitmapColorFunc = std::function<Pixel(size_t x, size_t y, const Pixel& imageColor)>;
  auto Bitmap(const Point2dInt& centre,
              const UTILS::GRAPHICS::ImageBitmap& bitmap,
              const GetBitmapColorFunc& getColor) noexcept -> void;
  auto Bitmap(const Point2dInt& centre,
              const UTILS::GRAPHICS::ImageBitmap& bitmap,
              const std::vector<GetBitmapColorFunc>& getColors) noexcept -> void;

private:
  IGoomDraw* m_draw;
};

inline BitmapDrawer::BitmapDrawer(IGoomDraw& draw) noexcept : m_draw{&draw}
{
}

inline auto BitmapDrawer::Bitmap(const Point2dInt& centre,
                                 const UTILS::GRAPHICS::ImageBitmap& bitmap,
                                 const GetBitmapColorFunc& getColor) noexcept -> void
{
  // WARNING undefined behaviour - GCC 11 does not like passing just '{getColor}'.
  Bitmap(centre, bitmap, std::vector<GetBitmapColorFunc>{getColor});
}

} // namespace DRAW::SHAPE_DRAWERS
} // namespace GOOM
