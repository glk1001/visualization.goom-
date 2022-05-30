#include "draw_methods.h"

#include "color/colorutils.h"
#include "goom_config.h"
#include "goom_graphic.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace GOOM::DRAW
{

using COLOR::GetBrighterColor;

DrawMethods::DrawMethods(const uint32_t screenWidth,
                         const uint32_t screenHeight,
                         const DrawPixelsToDeviceFunc& func)
  : m_screenWidth{screenWidth}, m_screenHeight{screenHeight}, m_drawPixelsToDevice{func}
{
}

void DrawMethods::DrawCircle(const int32_t x0,
                             const int32_t y0,
                             const int32_t radius,
                             const Pixel& color)
{
  const std::vector<Pixel> colors{color};
  DrawCircle(x0, y0, radius, colors);
}

// Function for circle-generation using Bresenham's algorithm
void DrawMethods::DrawBresenhamCircle(const int32_t x0,
                                      const int32_t y0,
                                      const int32_t radius,
                                      const PlotCirclePointsFunc& plotter)
{
  const auto drawCircle8 = [&](const int xc, int const yc, const int x, const int y)
  {
    plotter(xc - x, yc + y, xc + x, yc + y);
    plotter(xc - x, yc - y, xc + x, yc - y);
    plotter(xc - y, yc + x, xc + y, yc + x);
    plotter(xc - y, yc - x, xc + y, yc - x);
  };

  int x = 0;
  int y = radius;

  drawCircle8(x0, y0, x, y);

  int d = 3 - (2 * radius);
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
    drawCircle8(x0, y0, x, y);
  }
}

void DrawMethods::DrawCircle(const int32_t x0,
                             const int32_t y0,
                             const int32_t radius,
                             const std::vector<Pixel>& colors)
{
  auto plotter = [&](const int x1, const int y1, const int x2, const int y2) -> void
  {
    if ((static_cast<uint32_t>(x1) >= m_screenWidth) ||
        (static_cast<uint32_t>(y1) >= m_screenHeight))
    {
      return;
    }
    if ((static_cast<uint32_t>(x2) >= m_screenWidth) ||
        (static_cast<uint32_t>(y2) >= m_screenHeight))
    {
      return;
    }
    DrawPixels(x1, y1, colors);
    if ((x1 == x2) && (y1 == y2))
    {
      return;
    }
    DrawPixels(x2, y2, colors);
  };

  DrawBresenhamCircle(x0, y0, radius, plotter);
}

void DrawMethods::DrawHorizontalLine(const int x1,
                                     const int y,
                                     const int x2,
                                     const std::vector<Pixel>& colors)
{
  const int xEnd = x1 == x2 ? x1 : x2;
  for (int x = x1; x <= xEnd; ++x)
  {
    DrawPixels(x, y, colors);
  }
}


void DrawMethods::DrawFilledCircle(const int32_t x0,
                                   const int32_t y0,
                                   const int32_t radius,
                                   const std::vector<Pixel>& colors)
{
  auto plotter = [&](const int x1, const int y1, const int x2,
                     [[maybe_unused]] const int y2) -> void
  {
    Expects(y1 == y2);
    DrawHorizontalLine(x1, y1, x2, colors);
  };

  DrawBresenhamCircle(x0, y0, radius, plotter);
}

static constexpr int LINE_THICKNESS_MIDDLE = 0;
static constexpr int LINE_THICKNESS_DRAW_CLOCKWISE = 1;
static constexpr int LINE_THICKNESS_DRAW_COUNTERCLOCKWISE = 2;

void DrawMethods::DrawLine(const int32_t x1,
                           const int32_t y1,
                           const int32_t x2,
                           const int32_t y2,
                           const Pixel& color,
                           const uint8_t thickness)
{
  const std::vector<Pixel> colors{color};
  DrawLine(x1, y1, x2, y2, colors, thickness);
}

void DrawMethods::DrawLine(const int32_t x1,
                           const int32_t y1,
                           const int32_t x2,
                           const int32_t y2,
                           const std::vector<Pixel>& colors,
                           const uint8_t thickness)
{
  if (1 == thickness)
  {
    DrawWuLine(x1, y1, x2, y2, colors);
  }
  else
  {
    DrawThickLine(x1, y1, x2, y2, colors, thickness, LINE_THICKNESS_MIDDLE);
  }
}

void DrawMethods::DrawWuLine(
    const int x1, const int y1, const int x2, const int y2, const std::vector<Pixel>& colors)
{
  if ((y1 < 0) || (y2 < 0) || (x1 < 0) || (x2 < 0) || (y1 >= static_cast<int>(m_screenHeight)) ||
      (y2 >= static_cast<int>(m_screenHeight)) || (x1 >= static_cast<int>(m_screenWidth)) ||
      (x2 >= static_cast<int>(m_screenWidth)))
  {
    return;
  }

  std::vector<Pixel> tempColors = colors;
  auto plot = [&](const int x, const int y, const float brightness) -> void
  {
    if ((static_cast<uint32_t>(x) >= m_screenWidth) || (static_cast<uint32_t>(y) >= m_screenHeight))
    {
      return;
    }
    static constexpr float TOLERANCE = 0.001F;
    if (brightness < TOLERANCE)
    {
      return;
    }
    if (brightness >= (1.0F - TOLERANCE))
    {
      DrawPixels(x, y, colors);
    }
    else
    {
      for (size_t i = 0; i < colors.size(); ++i)
      {
        tempColors[i] = GetBrighterColor(brightness, colors[i]);
      }
      DrawPixels(x, y, tempColors);
    }
  };

  WuLine(static_cast<float>(x1), static_cast<float>(y1), static_cast<float>(x2),
         static_cast<float>(y2), plot);
}

// The Xiaolin Wu anti-aliased draw line.
// From https://rosettacode.org/wiki/Xiaolin_Wu%27s_line_algorithm#C.2B.2B
//
void DrawMethods::WuLine(float x0, float y0, float x1, float y1, const PlotPointFunc& plot)
{
  const auto iPart = [](const float x) -> int { return static_cast<int>(std::floor(x)); };
  const auto fRound = [](const float x) -> float { return std::round(x); };
  const auto fPart = [](const float x) -> float { return x - std::floor(x); };
  const auto rFPart = [=](const float x) -> float { return 1 - fPart(x); };

  const bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
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

  const float dx = x1 - x0; // because of above swap, must be >= 0
  const float dy = y1 - y0;
  const float gradient = (dx < 0.001F) ? 1 : (dy / dx);

  int xpx11;
  float interY;
  {
    const float xEnd = fRound(x0);
    const float yEnd = y0 + (gradient * (xEnd - x0));
    const float xGap = rFPart(x0 + 0.5F);
    xpx11 = static_cast<int>(xEnd);
    const int ypx11 = iPart(yEnd);
    if (steep)
    {
      plot(ypx11, xpx11, rFPart(yEnd) * xGap);
      plot(ypx11 + 1, xpx11, fPart(yEnd) * xGap);
    }
    else
    {
      plot(xpx11, ypx11, rFPart(yEnd) * xGap);
      plot(xpx11, ypx11 + 1, fPart(yEnd) * xGap);
    }
    interY = yEnd + gradient;
  }

  int xpx12;
  {
    const float xEnd = fRound(x1);
    const float yEnd = y1 + (gradient * (xEnd - x1));
    const float xGap = rFPart(x1 + 0.5F);
    xpx12 = static_cast<int>(xEnd);
    const int ypx12 = iPart(yEnd);
    if (steep)
    {
      plot(ypx12, xpx12, rFPart(yEnd) * xGap);
      plot(ypx12 + 1, xpx12, fPart(yEnd) * xGap);
    }
    else
    {
      plot(xpx12, ypx12, rFPart(yEnd) * xGap);
      plot(xpx12, ypx12 + 1, fPart(yEnd) * xGap);
    }
  }

  if (steep)
  {
    for (int x = xpx11 + 1; x < xpx12; ++x)
    {
      plot(iPart(interY), x, rFPart(interY));
      plot(iPart(interY) + 1, x, fPart(interY));
      interY += gradient;
    }
  }
  else
  {
    for (int x = xpx11 + 1; x < xpx12; ++x)
    {
      plot(x, iPart(interY), rFPart(interY));
      plot(x, iPart(interY) + 1, fPart(interY));
      interY += gradient;
    }
  }
}

/**
 * Modified Bresenham draw(line) with optional overlap. Required for drawThickLine().
 * Overlap draws additional pixel when changing minor direction. For standard bresenham overlap,
 * choose LINE_OVERLAP_NONE (0).
 *
 *  Sample line:
 *
 *    00+
 *     -0000+
 *         -0000+
 *             -00
 *
 *  0 pixels are drawn for normal line without any overlap
 *  + pixels are drawn if LINE_OVERLAP_MAJOR
 *  - pixels are drawn if LINE_OVERLAP_MINOR
 */

static constexpr int LINE_OVERLAP_NONE = 0;
static constexpr int LINE_OVERLAP_MAJOR = 1;
static constexpr int LINE_OVERLAP_MINOR = 2;

void DrawMethods::DrawLineOverlap(int x0,
                                  int y0,
                                  const int x1,
                                  const int y1,
                                  const std::vector<Pixel>& colors,
                                  const float brightness,
                                  const uint8_t overlap)
{
  if ((y0 < 0) || (y1 < 0) || (x0 < 0) || (x1 < 0) || (y0 >= static_cast<int>(m_screenHeight)) ||
      (y1 >= static_cast<int>(m_screenHeight)) || (x0 >= static_cast<int>(m_screenWidth)) ||
      (x1 >= static_cast<int>(m_screenWidth)))
  {
    return;
  }

  std::vector<Pixel> tempColors = colors;
  auto plot = [&](const int x, const int y) -> void
  {
    if ((static_cast<uint32_t>(x) >= m_screenWidth) || (static_cast<uint32_t>(y) >= m_screenHeight))
    {
      return;
    }
    static constexpr float TOLERANCE = 0.001F;
    if (brightness >= (1.0F - TOLERANCE))
    {
      DrawPixels(x, y, colors);
    }
    else
    {
      for (size_t i = 0; i < colors.size(); ++i)
      {
        tempColors[i] = GetBrighterColor(brightness, colors[i]);
      }
      DrawPixels(x, y, tempColors);
    }
  };

  if ((x0 == x1) || (y0 == y1))
  {
    //horizontal or vertical line -> fillRect() is faster than drawLine()
    //        LocalDisplay.fillRect(aXStart, aYStart, aXEnd, aYEnd, aColor);
    // ????????????????????????????????????????????????????????????????????????????????????????????
    DrawWuLine(x0, y0, x1, y1, colors);
  }
  else
  {
    int32_t error;
    int32_t stepX;
    int32_t stepY;

    // Calculate direction.
    int32_t deltaX = x1 - x0;
    int32_t deltaY = y1 - y0;
    if (deltaX < 0)
    {
      deltaX = -deltaX;
      stepX = -1;
    }
    else
    {
      stepX = +1;
    }
    if (deltaY < 0)
    {
      deltaY = -deltaY;
      stepY = -1;
    }
    else
    {
      stepY = +1;
    }

    const int32_t deltaXTimes2 = deltaX << 1;
    const int32_t deltaYTimes2 = deltaY << 1;

    // Draw start pixel.
    plot(x0, y0);
    if (deltaX > deltaY)
    {
      // Start value represents a half step in Y direction.
      error = deltaYTimes2 - deltaX;
      while (x0 != x1)
      {
        // Step in main direction.
        x0 += stepX;
        if (error >= 0)
        {
          if (overlap == LINE_OVERLAP_MAJOR)
          {
            // Draw pixel in main direction before changing.
            plot(x0, y0);
          }
          // change Y
          y0 += stepY;
          if (overlap == LINE_OVERLAP_MINOR)
          {
            // Draw pixel in minor direction before changing.
            plot(x0 - stepX, y0);
          }
          error -= deltaXTimes2;
        }
        error += deltaYTimes2;
        plot(x0, y0);
      }
    }
    else
    {
      error = deltaXTimes2 - deltaY;
      while (y0 != y1)
      {
        y0 += stepY;
        if (error >= 0)
        {
          if (overlap == LINE_OVERLAP_MAJOR)
          {
            // Draw pixel in main direction before changing.
            plot(x0, y0);
          }
          x0 += stepX;
          if (overlap == LINE_OVERLAP_MINOR)
          {
            // Draw pixel in minor direction before changing.
            plot(x0, y0 - stepY);
          }
          error -= deltaYTimes2;
        }
        error += deltaXTimes2;
        plot(x0, y0);
      }
    }
  }
}

/**
 * Bresenham with thickness.
 * No pixel missed and every pixel only drawn once!
 * thicknessMode can be one of LINE_THICKNESS_MIDDLE, LINE_THICKNESS_DRAW_CLOCKWISE,
 *   LINE_THICKNESS_DRAW_COUNTERCLOCKWISE
 */

void DrawMethods::DrawThickLine(int x0,
                                int y0,
                                int x1,
                                int y1,
                                const std::vector<Pixel>& colors,
                                const uint8_t thickness,
                                const uint8_t thicknessMode)
{
  if ((y0 < 0) || (y1 < 0) || (x0 < 0) || (x1 < 0) || (y0 >= static_cast<int>(m_screenHeight)) ||
      (y1 >= static_cast<int>(m_screenHeight)) || (x0 >= static_cast<int>(m_screenWidth)) ||
      (x1 >= static_cast<int>(m_screenWidth)))
  {
    return;
  }

  if (thickness <= 1)
  {
    DrawLineOverlap(x0, y0, x1, y1, colors, 1.0, 0);
  }

  const float brightness = (0.8F * 2.0F) / static_cast<float>(thickness);

  /**
    * For coordinate system with 0.0 top left
    * Swap X and Y delta and calculate clockwise (new delta X inverted)
    * or counterclockwise (new delta Y inverted) rectangular direction.
    * The right rectangular direction for LINE_OVERLAP_MAJOR toggles with each octant.
  */

  int error;
  int stepX;
  int stepY;
  int deltaY = x1 - x0;
  int deltaX = y1 - y0;

  // Mirror 4 quadrants to one and adjust deltas and stepping direction.
  bool swap = true; // count effective mirroring
  if (deltaX < 0)
  {
    deltaX = -deltaX;
    stepX = -1;
    swap = !swap;
  }
  else
  {
    stepX = +1;
  }
  if (deltaY < 0)
  {
    deltaY = -deltaY;
    stepY = -1;
    swap = !swap;
  }
  else
  {
    stepY = +1;
  }

  const int deltaXTimes2 = deltaX << 1;
  const int deltaYTimes2 = deltaY << 1;

  bool overlap;
  // Adjust for right direction of thickness from line origin.
  int drawStartAdjustCount = thickness / 2;
  if (thicknessMode == LINE_THICKNESS_DRAW_COUNTERCLOCKWISE)
  {
    drawStartAdjustCount = thickness - 1;
  }
  else if (thicknessMode == LINE_THICKNESS_DRAW_CLOCKWISE)
  {
    drawStartAdjustCount = 0;
  }

  // Which octant are we now?
  if (deltaX >= deltaY)
  {
    if (swap)
    {
      drawStartAdjustCount = (thickness - 1) - drawStartAdjustCount;
      stepY = -stepY;
    }
    else
    {
      stepX = -stepX;
    }
    /*
     * Vector for draw direction of start of lines is rectangular and counterclockwise to
     * main line direction. Therefore, no pixel will be missed if LINE_OVERLAP_MAJOR is used
     * on change in minor rectangular direction.
     */
    // adjust draw start point
    error = deltaYTimes2 - deltaX;
    for (int i = drawStartAdjustCount; i > 0; --i)
    {
      // change X (main direction here)
      x0 -= stepX;
      x1 -= stepX;
      if (error >= 0)
      {
        // change Y
        y0 -= stepY;
        y1 -= stepY;
        error -= deltaXTimes2;
      }
      error += deltaYTimes2;
    }
    //draw start line
    DrawLineOverlap(x0, y0, x1, y1, colors, brightness, 1);
    // draw 'thickness' number of lines
    error = deltaYTimes2 - deltaX;
    for (int i = thickness; i > 1; --i)
    {
      // change X (main direction here)
      x0 += stepX;
      x1 += stepX;
      overlap = LINE_OVERLAP_NONE;
      if (error >= 0)
      {
        // change Y
        y0 += stepY;
        y1 += stepY;
        error -= deltaXTimes2;
        /*
         * Change minor direction reverse to line (main) direction because of choosing
         * the right (counter)clockwise draw vector. Use LINE_OVERLAP_MAJOR to fill all pixels.
         *
         * EXAMPLE:
         * 1,2 = Pixel of first 2 lines
         * 3 = Pixel of third line in normal line mode
         * - = Pixel which will additionally be drawn in LINE_OVERLAP_MAJOR mode
         *           33
         *       3333-22
         *   3333-222211
         *   33-22221111
         *  221111                     /\
         *  11                          Main direction of start of lines draw vector
         *  -> Line main direction
         *  <- Minor direction of counterclockwise of start of lines draw vector
         */
        overlap = LINE_OVERLAP_MAJOR;
      }
      error += deltaYTimes2;
      DrawLineOverlap(x0, y0, x1, y1, colors, brightness, overlap);
    }
  }
  else
  {
    // the other octant
    if (swap)
    {
      stepX = -stepX;
    }
    else
    {
      drawStartAdjustCount = (thickness - 1) - drawStartAdjustCount;
      stepY = -stepY;
    }
    // adjust draw start point
    error = deltaXTimes2 - deltaY;
    for (int i = drawStartAdjustCount; i > 0; --i)
    {
      y0 -= stepY;
      y1 -= stepY;
      if (error >= 0)
      {
        x0 -= stepX;
        x1 -= stepX;
        error -= deltaYTimes2;
      }
      error += deltaXTimes2;
    }
    // draw start line
    DrawLineOverlap(x0, y0, x1, y1, colors, brightness, 0);
    // draw 'thickness' number of lines
    error = deltaXTimes2 - deltaY;
    for (int i = thickness; i > 1; --i)
    {
      y0 += stepY;
      y1 += stepY;
      overlap = LINE_OVERLAP_NONE;
      if (error >= 0)
      {
        x0 += stepX;
        x1 += stepX;
        error -= deltaYTimes2;
        overlap = LINE_OVERLAP_MAJOR;
      }
      error += deltaXTimes2;
      DrawLineOverlap(x0, y0, x1, y1, colors, brightness, overlap);
    }
  }
}

} // namespace GOOM::DRAW
