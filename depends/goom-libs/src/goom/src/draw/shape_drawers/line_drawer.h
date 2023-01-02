#pragma once

#include "draw/goom_draw.h"
#include "drawer_utils.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "point2d.h"

#include <cstdint>
#include <functional>
#include <vector>

namespace GOOM::DRAW::SHAPE_DRAWERS
{

class LineDrawer
{
public:
  explicit LineDrawer(IGoomDraw& draw) noexcept;

  auto DrawLine(const Point2dInt& point1,
                const Point2dInt& point2,
                const Pixel& color,
                uint8_t thickness) noexcept -> void;
  auto DrawLine(const Point2dInt& point1,
                const Point2dInt& point2,
                const std::vector<Pixel>& colors,
                uint8_t thickness) noexcept -> void;

private:
  IGoomDraw& m_draw;

  static constexpr auto WU_CLIP_MARGIN = 2;
  ClipTester m_wuClipTester{m_draw.GetDimensions(), WU_CLIP_MARGIN};
  auto DrawWuLine(const Point2dInt& point1,
                  const Point2dInt& point2,
                  const std::vector<Pixel>& colors) noexcept -> void;
  using PlotPointFunc = const std::function<void(const Point2dInt& point, float brightness)>;
  static auto WuLine(float x0, float y0, float x1, float y1, const PlotPointFunc& plot) noexcept
      -> void;

  auto DrawThickLine(const Point2dInt& point1,
                     const Point2dInt& point2,
                     const std::vector<Pixel>& colors,
                     uint8_t thickness) noexcept -> void;
};

inline LineDrawer::LineDrawer(IGoomDraw& draw) noexcept : m_draw{draw}
{
}

inline auto LineDrawer::DrawLine(const Point2dInt& point1,
                                 const Point2dInt& point2,
                                 const Pixel& color,
                                 const uint8_t thickness) noexcept -> void
{
  DrawLine(point1, point2, std::vector<Pixel>{color}, thickness);
}

} // namespace GOOM::DRAW::SHAPE_DRAWERS
