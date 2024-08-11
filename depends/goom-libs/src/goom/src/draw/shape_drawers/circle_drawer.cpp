module;

#include <cstdint>

module Goom.Draw.ShaperDrawers.CircleDrawer;

import Goom.Draw.GoomDrawBase;
import Goom.Draw.ShaperDrawers.DrawerUtils;
import Goom.Lib.AssertUtils;
import Goom.Lib.Point2d;

namespace GOOM::DRAW::SHAPE_DRAWERS
{

auto CircleDrawer::DrawCircle(const Point2dInt& centre,
                              const int32_t radius,
                              const MultiplePixels& colors) noexcept -> void
{
  if (ClipTester{m_draw->GetDimensions(), radius}.IsOutside(centre))
  {
    return;
  }

  const auto plot = [this, &colors](const Point2dInt& point1, const Point2dInt& point2)
  {
    m_draw->DrawPixels(point1, colors);
    if (point1 == point2)
    {
      return;
    }
    m_draw->DrawPixels(point2, colors);
  };

  DrawBresenhamCircle(centre, radius, plot);
}

auto CircleDrawer::DrawFilledCircle(const Point2dInt& centre,
                                    const int32_t radius,
                                    const MultiplePixels& colors) noexcept -> void
{
  if (ClipTester{m_draw->GetDimensions(), radius}.IsOutside(centre))
  {
    return;
  }

  const auto plot = [this, &colors](const Point2dInt& point1, const Point2dInt& point2)
  {
    Expects(point1.y == point2.y);
    DrawHorizontalLine(point1, point2.x, colors);
  };

  DrawBresenhamCircle(centre, radius, plot);
}

auto CircleDrawer::DrawHorizontalLine(const Point2dInt& point1,
                                      const int32_t x2,
                                      const MultiplePixels& colors) noexcept -> void
{
  const auto xEnd = point1.x == x2 ? point1.x : x2;
  for (auto x = point1.x; x <= xEnd; ++x)
  {
    m_draw->DrawPixels({.x = x, .y = point1.y}, colors);
  }
}

// Function for circle-generation using Bresenham's algorithm.
auto CircleDrawer::DrawBresenhamCircle(const Point2dInt& centre,
                                       const int32_t radius,
                                       const PlotCirclePointsFunc& plot) noexcept -> void
{
  const auto drawCircle8 =
      [&plot](const int32_t xc, int32_t const yc, const int32_t x, const int32_t y)
  {
    plot({.x = xc - x, .y = yc + y}, {.x = xc + x, .y = yc + y});
    plot({.x = xc - x, .y = yc - y}, {.x = xc + x, .y = yc - y});
    plot({.x = xc - y, .y = yc + x}, {.x = xc + y, .y = yc + x});
    plot({.x = xc - y, .y = yc - x}, {.x = xc + y, .y = yc - x});
  };

  int32_t x = 0;
  int32_t y = radius;

  drawCircle8(centre.x, centre.y, x, y);

  int32_t d = 3 - (2 * radius); // NOLINT(readability-identifier-length)
  while (y >= x)
  {
    ++x;

    if (static constexpr auto FACTOR = 4; d > 0)
    {
      --y;
      static constexpr auto D_POS_INC = 10;
      d += (FACTOR * (x - y)) + D_POS_INC;
    }
    else
    {
      static constexpr auto D_NEG_INC = 6;
      d += (FACTOR * x) + D_NEG_INC;
    }
    drawCircle8(centre.x, centre.y, x, y);
  }
}

} // namespace GOOM::DRAW::SHAPE_DRAWERS
