#undef NO_LOGGING

#include "line_drawer.h"

#include "goom_config.h"
#include "goom_graphic.h"
#include "goom_logger.h"
#include "line_draw_thick.h"
#include "line_draw_wu.h"
#include "utils/debugging_logger.h"

#include <cstdint>
#include <vector>

namespace GOOM::DRAW::SHAPE_DRAWERS
{

using THICK_LINES::DrawVariableThicknessLine;
using WU_LINES::WuLine;

auto LineDrawer::DrawLine(const Point2dInt& point1,
                          const Point2dInt& point2,
                          const std::vector<Pixel>& colors) noexcept -> void
{
  if (1 == m_thickness)
  {
    DrawWuLine(point1, point2, colors);
  }
  else
  {
    DrawThickLine(point1, point2, colors);
  }
}

inline auto LineDrawer::DrawWuLine(const Point2dInt& point1,
                                   const Point2dInt& point2,
                                   const std::vector<Pixel>& colors) noexcept -> void
{
  Expects(1 == m_thickness);

  m_plotPixel.SetColors(colors);
  const auto plot = [this](const Point2dInt& point, const float brightness)
  {
    m_plotPixel.SetBrightness(brightness);
    m_plotPixel.PlotPoint(point);
  };

  WuLine(static_cast<float>(point1.x),
         static_cast<float>(point1.y),
         static_cast<float>(point2.x),
         static_cast<float>(point2.y),
         plot);
}

inline auto LineDrawer::DrawThickLine(const Point2dInt& point1,
                                      const Point2dInt& point2,
                                      const std::vector<Pixel>& colors) noexcept -> void
{
  Expects(m_thickness > 1);

  const auto brightness = (0.8F * 2.0F) / static_cast<float>(m_thickness);

  m_plotPixel.SetColors(colors);
  m_plotPixel.SetBrightness(brightness);
  const auto plot = [this](const int32_t x, const int32_t y) { m_plotPixel.PlotPoint({x, y}); };

  const auto getWidth =
      [this]([[maybe_unused]] const int32_t pointNum, [[maybe_unused]] const int32_t lineLength)
  { return static_cast<double>(m_thickness); };

  DrawVariableThicknessLine(plot, point1.x, point1.y, point2.x, point2.y, getWidth, getWidth);
}

} // namespace GOOM::DRAW::SHAPE_DRAWERS
