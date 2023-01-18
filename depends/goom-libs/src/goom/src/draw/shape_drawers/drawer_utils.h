#pragma once

#include "drawer_types.h"
#include "goom_config.h"
#include "goom_types.h"
#include "point2d.h"

#include <cstdint>

namespace GOOM::DRAW::SHAPE_DRAWERS
{

class PixelPlotter
{
public:
  explicit PixelPlotter(PixelDrawerFunc pixelDrawer) noexcept;

  auto SetColors(const std::vector<Pixel>& colors) noexcept -> void;
  auto SetBrightness(float brightness) noexcept -> void;

  auto PlotPoint(const Point2dInt& point0) noexcept -> void;

private:
  PixelDrawerFunc m_pixelDrawer;
  float m_brightness = 1.0F;
  const std::vector<Pixel>* m_colors{};
  std::vector<Pixel> m_tempColors{};
};

class ClipTester
{
public:
  ClipTester(const Dimensions& screenDimensions, int32_t clipMargin) noexcept;

  auto SetClipMargin(int32_t clipMargin) noexcept -> void;

  [[nodiscard]] auto IsOutside(const Point2dInt& point) const noexcept;

  struct ClipRect
  {
    int32_t x0;
    int32_t y0;
    int32_t x1;
    int32_t y1;
  };
  [[nodiscard]] auto GetClipRect() const noexcept -> const ClipRect&;

private:
  Dimensions m_screenDimensions;
  ClipRect m_clipRect;
  [[nodiscard]] auto GetClipRect(int32_t clipMargin) const noexcept -> ClipRect;
};

inline PixelPlotter::PixelPlotter(PixelDrawerFunc pixelDrawer) noexcept
  : m_pixelDrawer{std::move(pixelDrawer)}
{
}

inline auto PixelPlotter::SetColors(const std::vector<Pixel>& colors) noexcept -> void
{
  m_colors = &colors;
  m_tempColors.resize(m_colors->size());
}

inline auto PixelPlotter::SetBrightness(const float brightness) noexcept -> void
{
  m_brightness = brightness;
}

inline auto PixelPlotter::PlotPoint(const Point2dInt& point) noexcept -> void
{
  //TODO(glk) Why this tolerance test?
  if (static constexpr auto TOLERANCE = 0.001F; m_brightness >= (1.0F - TOLERANCE))
  {
    m_pixelDrawer(point, *m_colors);
  }
  else
  {
    for (auto i = 0U; i < m_colors->size(); ++i)
    {
      m_tempColors[i] = COLOR::GetBrighterColor(m_brightness, (*m_colors)[i]);
    }
    m_pixelDrawer(point, m_tempColors);
  }
}

inline ClipTester::ClipTester(const Dimensions& screenDimensions, const int32_t clipMargin) noexcept
  : m_screenDimensions{screenDimensions}, m_clipRect{GetClipRect(clipMargin)}
{
}

inline auto ClipTester::GetClipRect(const int32_t clipMargin) const noexcept -> ClipRect
{
  return {clipMargin,
          clipMargin,
          (m_screenDimensions.GetIntWidth() - clipMargin) - 1,
          (m_screenDimensions.GetIntHeight() - clipMargin) - 1};
}

inline auto ClipTester::GetClipRect() const noexcept -> const ClipRect&
{
  return m_clipRect;
}

inline auto ClipTester::SetClipMargin(const int32_t clipMargin) noexcept -> void
{
  m_clipRect = GetClipRect(clipMargin);
}

inline auto ClipTester::IsOutside(const Point2dInt& point) const noexcept
{
  return (point.x < m_clipRect.x0) or (point.y < m_clipRect.y0) or (point.x > m_clipRect.x1) or
         (point.y > m_clipRect.y1);
}

} // namespace GOOM::DRAW::SHAPE_DRAWERS
