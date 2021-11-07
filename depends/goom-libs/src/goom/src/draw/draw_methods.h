#pragma once

#include "color/colorutils.h"
#include "goom_graphic.h"

#ifdef GOOM_DEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace DRAW
{
#else
namespace GOOM::DRAW
{
#endif

using DrawPixelFunc =
    std::function<void(int32_t x, int32_t y, const std::vector<Pixel>& newColors)>;

class DrawMethods
{
public:
  DrawMethods(uint32_t screenWidth, uint32_t screenHeight, const DrawPixelFunc& func);

  void DrawCircle(int32_t x0, int32_t y0, int32_t radius, const Pixel& color) const;
  void DrawCircle(int32_t x0, int32_t y0, int32_t radius, const std::vector<Pixel>& colors) const;
  void DrawFilledCircle(int32_t x0,
                        int32_t y0,
                        int32_t radius,
                        const std::vector<Pixel>& colors) const;

  void DrawLine(
      int32_t x1, int32_t y1, int32_t x2, int32_t y2, const Pixel& color, uint8_t thickness) const;
  void DrawLine(int32_t x1,
                int32_t y1,
                int32_t x2,
                int32_t y2,
                const std::vector<Pixel>& colors,
                uint8_t thickness) const;

  void DrawPixels(int32_t x, int32_t y, const std::vector<Pixel>& newColors) const;

private:
  const uint32_t m_screenWidth;
  const uint32_t m_screenHeight;
  const DrawPixelFunc m_drawPixelFunc;

  using PlotCirclePointsFunc = std::function<void(int32_t x1, int32_t y1, int32_t x2, int32_t y2)>;
  static void DrawBresenhamCircle(int32_t x0,
                                  int32_t y0,
                                  int32_t radius,
                                  const PlotCirclePointsFunc& plotter);

  void DrawHorizontalLine(int x1, int y, int x2, const std::vector<Pixel>& colors) const;
  void DrawThickLine(int x0,
                     int y0,
                     int x1,
                     int y1,
                     const std::vector<Pixel>& colors,
                     uint8_t thickness,
                     uint8_t thicknessMode) const;
  void DrawLineOverlap(int x0,
                       int y0,
                       int x1,
                       int y1,
                       const std::vector<Pixel>& colors,
                       float brightness,
                       uint8_t overlap) const;
  void DrawWuLine(int x1, int y1, int x2, int y2, const std::vector<Pixel>& colors) const;
  using PlotPointFunc = const std::function<void(int x, int y, float brightness)>;
  static void WuLine(float x0, float y0, float x1, float y1, const PlotPointFunc& plot);
};

inline void DrawMethods::DrawPixels(const int32_t x,
                                    const int32_t y,
                                    const std::vector<Pixel>& newColors) const
{
#ifdef GOOM_DEBUG
  assert(x >= 0);
  assert(y >= 0);
  assert(static_cast<uint32_t>(x) < m_screenWidth);
  assert(static_cast<uint32_t>(y) < m_screenHeight);
#endif
  m_drawPixelFunc(x, y, newColors);
}

#if __cplusplus <= 201402L
} // namespace DRAW
} // namespace GOOM
#else
} // namespace GOOM::DRAW
#endif

