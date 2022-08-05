#pragma once

#include "goom_config.h"
#include "goom_graphic.h"
#include "goom_types.h"

#include <cstdint>
#include <functional>
#include <vector>

namespace GOOM::DRAW
{

using DrawPixelsToDeviceFunc =
    std::function<void(int32_t x, int32_t y, const std::vector<Pixel>& newColors)>;

class DrawMethods
{
public:
  DrawMethods(const Dimensions& dimensions, const DrawPixelsToDeviceFunc& func);

  void DrawCircle(int32_t x0, int32_t y0, int32_t radius, const Pixel& color);
  void DrawCircle(int32_t x0, int32_t y0, int32_t radius, const std::vector<Pixel>& colors);
  void DrawFilledCircle(int32_t x0, int32_t y0, int32_t radius, const std::vector<Pixel>& colors);

  void DrawLine(
      int32_t x1, int32_t y1, int32_t x2, int32_t y2, const Pixel& color, uint8_t thickness);
  void DrawLine(int32_t x1,
                int32_t y1,
                int32_t x2,
                int32_t y2,
                const std::vector<Pixel>& colors,
                uint8_t thickness);

  void DrawPixels(int32_t x, int32_t y, const std::vector<Pixel>& newColors);

private:
  const Dimensions m_dimensions;
  const DrawPixelsToDeviceFunc m_drawPixelsToDevice;

  using PlotCirclePointsFunc = std::function<void(int32_t x1, int32_t y1, int32_t x2, int32_t y2)>;
  static void DrawBresenhamCircle(int32_t x0,
                                  int32_t y0,
                                  int32_t radius,
                                  const PlotCirclePointsFunc& plotter);

  void DrawHorizontalLine(int x1, int y, int x2, const std::vector<Pixel>& colors);
  void DrawThickLine(int x0,
                     int y0,
                     int x1,
                     int y1,
                     const std::vector<Pixel>& colors,
                     uint8_t thickness,
                     uint8_t thicknessMode);
  void DrawLineOverlap(int x0,
                       int y0,
                       int x1,
                       int y1,
                       const std::vector<Pixel>& colors,
                       float brightness,
                       uint8_t overlap);
  void DrawWuLine(int x1, int y1, int x2, int y2, const std::vector<Pixel>& colors);
  using PlotPointFunc = const std::function<void(int x, int y, float brightness)>;
  static void WuLine(float x0, float y0, float x1, float y1, const PlotPointFunc& plot);
};

inline void DrawMethods::DrawPixels(const int32_t x,
                                    const int32_t y,
                                    const std::vector<Pixel>& newColors)
{
  Expects(x >= 0);
  Expects(y >= 0);
  Expects(static_cast<uint32_t>(x) < m_dimensions.GetWidth());
  Expects(static_cast<uint32_t>(y) < m_dimensions.GetHeight());

  m_drawPixelsToDevice(x, y, newColors);
}

} // namespace GOOM::DRAW
