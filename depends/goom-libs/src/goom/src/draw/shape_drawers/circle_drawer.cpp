#include "circle_drawer.h"

#include "drawer_utils.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "point2d.h"

#include <cstdint>
#include <vector>

namespace GOOM::DRAW::SHAPE_DRAWERS
{

auto CircleDrawer::DrawCircle(const Point2dInt& centre,
                              const int32_t radius,
                              const std::vector<Pixel>& colors) noexcept -> void
{
  if (ClipTester{m_draw.GetDimensions(), radius}.IsOutside(centre))
  {
    return;
  }

  const auto plot = [this, &colors](const Point2dInt& point1, const Point2dInt& point2)
  {
    m_draw.DrawPixels(point1, colors);
    if (point1 == point2)
    {
      return;
    }
    m_draw.DrawPixels(point2, colors);
  };

  DrawBresenhamCircle(centre, radius, plot);
}

auto CircleDrawer::DrawFilledCircle(const Point2dInt& centre,
                                    const int32_t radius,
                                    const std::vector<Pixel>& colors) noexcept -> void
{
  if (ClipTester{m_draw.GetDimensions(), radius}.IsOutside(centre))
  {
    return;
  }

  const auto plot = [this, &colors](const Point2dInt& point1, const Point2dInt& point2)
  {
    Expects(point1.y == point2.y);
    DrawHorizontalLine(point1.x, point1.y, point2.x, colors);
  };

  DrawBresenhamCircle(centre, radius, plot);
}

auto CircleDrawer::DrawHorizontalLine(const int32_t x1,
                                      const int32_t y,
                                      const int32_t x2,
                                      const std::vector<Pixel>& colors) noexcept -> void
{
  const auto xEnd = x1 == x2 ? x1 : x2;
  for (int32_t x = x1; x <= xEnd; ++x)
  {
    m_draw.DrawPixels({x, y}, colors);
  }
}

// Function for circle-generation using Bresenham's algorithm
auto CircleDrawer::DrawBresenhamCircle(const Point2dInt& centre,
                                       const int32_t radius,
                                       const PlotCirclePointsFunc& plot) noexcept -> void
{
  const auto drawCircle8 =
      [&plot](const int32_t xc, int32_t const yc, const int32_t x, const int32_t y)
  {
    plot({xc - x, yc + y}, {xc + x, yc + y});
    plot({xc - x, yc - y}, {xc + x, yc - y});
    plot({xc - y, yc + x}, {xc + y, yc + x});
    plot({xc - y, yc - x}, {xc + y, yc - x});
  };

  int32_t x = 0;
  int32_t y = radius;

  drawCircle8(centre.x, centre.y, x, y);

  int32_t d = 3 - (2 * radius);
  while (y >= x)
  {
    ++x;

    if (d > 0)
    {
      --y;
      d += (4 * (x - y)) + 10;
    }
    else
    {
      d += (4 * x) + 6;
    }
    drawCircle8(centre.x, centre.y, x, y);
  }
}

} // namespace GOOM::DRAW::SHAPE_DRAWERS
