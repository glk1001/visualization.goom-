#pragma once

#include "draw/goom_draw.h"
#include "drawer_types.h"
#include "drawer_utils.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "point2d.h"

#include <vector>

namespace GOOM::DRAW::SHAPE_DRAWERS
{

class LineDrawer
{
public:
  explicit LineDrawer(PixelDrawerFunc pixelDrawer) noexcept;

  [[nodiscard]] auto GetLineThickness() const noexcept -> uint8_t;
  auto SetLineThickness(uint8_t thickness) noexcept -> void;

  auto DrawLine(const Point2dInt& point1, const Point2dInt& point2, const Pixel& color) noexcept
      -> void;
  auto DrawLine(const Point2dInt& point1,
                const Point2dInt& point2,
                const std::vector<Pixel>& colors) noexcept -> void;

private:
  PixelPlotter m_plotPixel;
  uint8_t m_thickness = 1U;

  auto DrawWuLine(const Point2dInt& point1,
                  const Point2dInt& point2,
                  const std::vector<Pixel>& colors) noexcept -> void;

  auto DrawThickLine(const Point2dInt& point1,
                     const Point2dInt& point2,
                     const std::vector<Pixel>& colors) noexcept -> void;
};

class LineDrawerNoClippedEndPoints
{
public:
  explicit LineDrawerNoClippedEndPoints(IGoomDraw& draw) noexcept;

  auto SetLineThickness(uint8_t thickness) noexcept -> void;

  auto DrawLine(const Point2dInt& point1, const Point2dInt& point2, const Pixel& color) noexcept
      -> void;
  auto DrawLine(const Point2dInt& point1,
                const Point2dInt& point2,
                const std::vector<Pixel>& colors) noexcept -> void;

private:
  LineDrawer m_lineDrawer;
};

class LineDrawerClippedEndPoints
{
public:
  explicit LineDrawerClippedEndPoints(IGoomDraw& draw) noexcept;

  auto SetLineThickness(uint8_t thickness) noexcept -> void;

  auto DrawLine(const Point2dInt& point1, const Point2dInt& point2, const Pixel& color) noexcept
      -> void;
  auto DrawLine(const Point2dInt& point1,
                const Point2dInt& point2,
                const std::vector<Pixel>& colors) noexcept -> void;

private:
  Dimensions m_dimensions;
  LineDrawer m_lineDrawer;
  ClipTester m_clipTester{m_dimensions, GetClipMargin()};
  [[nodiscard]] auto GetClipMargin() const noexcept -> int32_t;
};

inline LineDrawerNoClippedEndPoints::LineDrawerNoClippedEndPoints(IGoomDraw& draw) noexcept
  : m_lineDrawer{[&draw](const Point2dInt& point, const MultiplePixels& colors)
                 { draw.DrawClippedPixels(point, colors); }}
{
}

inline auto LineDrawerNoClippedEndPoints::SetLineThickness(const uint8_t thickness) noexcept -> void
{
  m_lineDrawer.SetLineThickness(thickness);
}

inline auto LineDrawerNoClippedEndPoints::DrawLine(const Point2dInt& point1,
                                                   const Point2dInt& point2,
                                                   const Pixel& color) noexcept -> void
{
  DrawLine(point1, point2, std::vector<Pixel>{color});
}

inline auto LineDrawerNoClippedEndPoints::DrawLine(const Point2dInt& point1,
                                                   const Point2dInt& point2,
                                                   const std::vector<Pixel>& colors) noexcept
    -> void
{
  m_lineDrawer.DrawLine(point1, point2, colors);
}

inline LineDrawerClippedEndPoints::LineDrawerClippedEndPoints(IGoomDraw& draw) noexcept
  : m_dimensions{draw.GetDimensions()},
    m_lineDrawer{[&draw](const Point2dInt& point, const MultiplePixels& colors)
                 { draw.DrawPixels(point, colors); }}
{
}

inline auto LineDrawerClippedEndPoints::SetLineThickness(const uint8_t thickness) noexcept -> void
{
  if (thickness == m_lineDrawer.GetLineThickness())
  {
    return;
  }

  m_lineDrawer.SetLineThickness(thickness);
  m_clipTester.SetClipMargin(GetClipMargin());
}

inline auto LineDrawerClippedEndPoints::GetClipMargin() const noexcept -> int32_t
{
  static constexpr auto CLIP_MARGIN_FOR_THIN_LINE = 2;

  return m_lineDrawer.GetLineThickness() == 1U ? CLIP_MARGIN_FOR_THIN_LINE
                                               : m_lineDrawer.GetLineThickness();
}

inline auto LineDrawerClippedEndPoints::DrawLine(const Point2dInt& point1,
                                                 const Point2dInt& point2,
                                                 const Pixel& color) noexcept -> void
{
  DrawLine(point1, point2, std::vector<Pixel>{color});
}

inline auto LineDrawerClippedEndPoints::DrawLine(const Point2dInt& point1,
                                                 const Point2dInt& point2,
                                                 const std::vector<Pixel>& colors) noexcept -> void
{
  if (m_clipTester.IsOutside(point1) or m_clipTester.IsOutside(point2))
  {
    return;
  }

  m_lineDrawer.DrawLine(point1, point2, colors);
}

inline LineDrawer::LineDrawer(PixelDrawerFunc pixelDrawer) noexcept
  : m_plotPixel{std::move(pixelDrawer)}
{
}

inline auto LineDrawer::GetLineThickness() const noexcept -> uint8_t
{
  return m_thickness;
}

inline auto LineDrawer::SetLineThickness(const uint8_t thickness) noexcept -> void
{
  Expects(thickness >= 1U);
  m_thickness = thickness;
}

inline auto LineDrawer::DrawLine(const Point2dInt& point1,
                                 const Point2dInt& point2,
                                 const Pixel& color) noexcept -> void
{
  DrawLine(point1, point2, std::vector<Pixel>{color});
}

} // namespace GOOM::DRAW::SHAPE_DRAWERS
