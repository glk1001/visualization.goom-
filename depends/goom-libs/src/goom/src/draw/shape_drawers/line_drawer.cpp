// NOLINTBEGIN: Not my code

#include "line_drawer.h"

#include "color/color_utils.h"
#include "drawer_utils.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "line_draw_thick.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace GOOM::DRAW::SHAPE_DRAWERS
{

using COLOR::GetBrighterColor;

auto LineDrawer::DrawLine(const Point2dInt& point1,
                          const Point2dInt& point2,
                          const std::vector<Pixel>& colors,
                          const uint8_t thickness) noexcept -> void
{
  if (1 == thickness)
  {
    DrawWuLine(point1, point2, colors);
  }
  else
  {
    DrawThickLine(point1, point2, colors, thickness);
  }
}

auto LineDrawer::DrawWuLine(const Point2dInt& point1,
                            const Point2dInt& point2,
                            const std::vector<Pixel>& colors) noexcept -> void
{
  if (m_wuClipTester.IsOutside(point1) or m_wuClipTester.IsOutside(point2))
  {
    return;
  }

  std::vector<Pixel> tempColors = colors;
  auto plot = [this, &colors, &tempColors](const Point2dInt& point, const float brightness)
  {
    static constexpr float TOLERANCE = 0.001F;
    if (brightness < TOLERANCE)
    {
      return;
    }
    if (brightness >= (1.0F - TOLERANCE))
    {
      m_draw.DrawPixels(point, colors);
    }
    else
    {
      for (size_t i = 0; i < colors.size(); ++i)
      {
        tempColors[i] = GetBrighterColor(brightness, colors[i]);
      }
      m_draw.DrawPixels(point, tempColors);
    }
  };

  WuLine(static_cast<float>(point1.x),
         static_cast<float>(point1.y),
         static_cast<float>(point2.x),
         static_cast<float>(point2.y),
         plot);
}

// The Xiaolin Wu anti-aliased draw line.
// From https://rosettacode.org/wiki/Xiaolin_Wu%27s_line_algorithm#C.2B.2B
//
auto LineDrawer::WuLine(float x0, float y0, float x1, float y1, const PlotPointFunc& plot) noexcept
    -> void
{
  const auto iPart  = [](const float x) { return static_cast<int>(std::floor(x)); };
  const auto fRound = [](const float x) { return std::round(x); };
  const auto fPart  = [](const float x) { return x - std::floor(x); };
  const auto rFPart = [=](const float x) { return 1 - fPart(x); };

  const auto steep = std::abs(y1 - y0) > std::abs(x1 - x0);
  if (steep)
  {
    std::swap(x0, y0);
    std::swap(x1, y1);
  }
  if (x0 > x1)
  {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }

  const auto dx       = x1 - x0; // because of above swap, must be >= 0
  const auto dy       = y1 - y0;
  const auto gradient = (dx < 0.001F) ? 1 : (dy / dx);

  int32_t xpx11;
  float interY;
  {
    const auto xEnd  = fRound(x0);
    const auto yEnd  = y0 + (gradient * (xEnd - x0));
    const auto xGap  = rFPart(x0 + 0.5F);
    xpx11            = static_cast<int>(xEnd);
    const auto ypx11 = iPart(yEnd);
    if (steep)
    {
      plot({ypx11, xpx11}, rFPart(yEnd) * xGap);
      plot({ypx11 + 1, xpx11}, fPart(yEnd) * xGap);
    }
    else
    {
      plot({xpx11, ypx11}, rFPart(yEnd) * xGap);
      plot({xpx11, ypx11 + 1}, fPart(yEnd) * xGap);
    }
    interY = yEnd + gradient;
  }

  int32_t xpx12;
  {
    const auto xEnd  = fRound(x1);
    const auto yEnd  = y1 + (gradient * (xEnd - x1));
    const auto xGap  = rFPart(x1 + 0.5F);
    xpx12            = static_cast<int>(xEnd);
    const auto ypx12 = iPart(yEnd);
    if (steep)
    {
      plot({ypx12, xpx12}, rFPart(yEnd) * xGap);
      plot({ypx12 + 1, xpx12}, fPart(yEnd) * xGap);
    }
    else
    {
      plot({xpx12, ypx12}, rFPart(yEnd) * xGap);
      plot({xpx12, ypx12 + 1}, fPart(yEnd) * xGap);
    }
  }

  if (steep)
  {
    for (auto x = xpx11 + 1; x < xpx12; ++x)
    {
      plot({iPart(interY), x}, rFPart(interY));
      plot({iPart(interY) + 1, x}, fPart(interY));
      interY += gradient;
    }
  }
  else
  {
    for (auto x = xpx11 + 1; x < xpx12; ++x)
    {
      plot({x, iPart(interY)}, rFPart(interY));
      plot({x, iPart(interY) + 1}, fPart(interY));
      interY += gradient;
    }
  }
}

auto LineDrawer::DrawThickLine(const Point2dInt& point1,
                               const Point2dInt& point2,
                               const std::vector<Pixel>& colors,
                               const uint8_t thickness) noexcept -> void
{
  Expects(thickness > 1);

  if (const ClipTester thickClipTester{m_draw.GetDimensions(), static_cast<int32_t>(thickness)};
      thickClipTester.IsOutside(point1) or thickClipTester.IsOutside(point2))
  {
    return;
  }

  const float brightness = (0.8F * 2.0F) / static_cast<float>(thickness);

  std::vector<Pixel> tempColors = colors;
  const auto plot = [this, &colors, &tempColors, &brightness](const int32_t x, const int32_t y)
  {
    static constexpr float TOLERANCE = 0.001F;
    if (brightness >= (1.0F - TOLERANCE))
    {
      m_draw.DrawPixels({x, y}, colors);
    }
    else
    {
      for (size_t i = 0; i < colors.size(); ++i)
      {
        tempColors[i] = GetBrighterColor(brightness, colors[i]);
      }
      m_draw.DrawPixels({x, y}, tempColors);
    }
  };

  auto getWidth = [&thickness]([[maybe_unused]] const int32_t pointNum,
                               [[maybe_unused]] const int32_t lineLength)
  { return static_cast<double>(thickness); };

  DrawVariableThicknessLine(plot, point1.x, point1.y, point2.x, point2.y, getWidth, getWidth);
}

} // namespace GOOM::DRAW::SHAPE_DRAWERS

// NOLINTEND
