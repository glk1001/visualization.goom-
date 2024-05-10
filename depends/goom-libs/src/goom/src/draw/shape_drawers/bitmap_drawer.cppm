module;

#include "goom/goom_config.h"
#include "goom/point2d.h"

#include <functional>
#include <vector>

export module Goom.Draw.ShapeDrawers.BitmapDrawer;

import Goom.Draw.GoomDrawBase;
import Goom.Utils.Graphics.ImageBitmaps;
import Goom.Lib.GoomGraphic;

export namespace GOOM::DRAW::SHAPE_DRAWERS
{

class BitmapDrawer
{
public:
  explicit BitmapDrawer(IGoomDraw& draw) noexcept;

  using GetBitmapColorFunc =
      std::function<Pixel(const Point2dInt& bitmapPoint, const Pixel& imageColor)>;
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

} // namespace GOOM::DRAW::SHAPE_DRAWERS
