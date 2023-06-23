#include "test_patterns.h"

#include "draw/goom_draw.h"
#include "draw/shape_drawers/line_drawer.h"
#include "goom_types.h"
#include "point2d.h"

namespace GOOM::UTILS::GRAPHICS
{

using DRAW::IGoomDraw;
using GOOM::COLOR::GetBrighterColor;
using GOOM::DRAW::MultiplePixels;
using GOOM::DRAW::SHAPE_DRAWERS::LineDrawerClippedEndPoints;

auto DrawTestPattern(IGoomDraw& draw, const Dimensions& dimensions) -> void
{
  const auto lineThickness = 3;
  const auto width         = dimensions.GetIntWidth();
  const auto height        = dimensions.GetIntHeight();
  const auto x0            = lineThickness;
  const auto y0            = lineThickness;
  const auto x1            = width - 1 - lineThickness;
  const auto y1            = height - 1 - lineThickness;
  const auto topLeft       = Point2dInt{x0, y0};
  const auto topRight      = Point2dInt{x1, y0};
  const auto bottomLeft    = Point2dInt{x0, y1};
  const auto bottomRight   = Point2dInt{x1, y1};

  static constexpr auto BRIGHTNESS = 10.0F;
  const auto white                 = GetBrighterColor(BRIGHTNESS, WHITE_PIXEL);
  const auto color                 = MultiplePixels{white, white};

  auto lineDrawer = LineDrawerClippedEndPoints{draw};
  lineDrawer.SetLineThickness(lineThickness);

  lineDrawer.DrawLine(topLeft, topRight, color);
  lineDrawer.DrawLine(topRight, bottomRight, color);
  lineDrawer.DrawLine(bottomRight, bottomLeft, color);
  lineDrawer.DrawLine(bottomLeft, topLeft, color);
  lineDrawer.DrawLine(bottomLeft, topRight, color);
  lineDrawer.DrawLine(bottomRight, topLeft, color);
}

} // namespace GOOM::UTILS::GRAPHICS
