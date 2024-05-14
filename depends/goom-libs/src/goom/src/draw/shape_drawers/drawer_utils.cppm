module;

#include "goom/goom_config.h"
#include "goom/goom_types.h"
#include "goom/point2d.h"

#include <cstdint>

export module Goom.Draw.ShaperDrawers.DrawerUtils;

import Goom.Color.ColorUtils;
import Goom.Draw.GoomDrawBase;

export namespace GOOM::DRAW::SHAPE_DRAWERS
{

auto BrightenColors(float brightness, MultiplePixels& colors) -> void;

class PixelDrawerNoClipping
{
public:
  explicit PixelDrawerNoClipping(IGoomDraw& draw) noexcept : m_draw{&draw} {}

  auto DrawPixels(const Point2dInt& point, const float brightness, MultiplePixels colors) noexcept
      -> void
  {
    BrightenColors(brightness, colors);
    m_draw->DrawPixels(point, colors);
  }

private:
  IGoomDraw* m_draw;
};

class PixelDrawerWithClipping
{
public:
  explicit PixelDrawerWithClipping(IGoomDraw& draw) noexcept : m_draw{&draw} {}

  auto DrawPixels(const Point2dInt& point,
                  [[maybe_unused]] const float brightness,
                  MultiplePixels colors) noexcept -> void
  {
    BrightenColors(brightness, colors);
    m_draw->DrawClippedPixels(point, colors);
  }

private:
  IGoomDraw* m_draw;
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

inline auto BrightenColors(const float brightness, MultiplePixels& colors) -> void
{
  colors.color1 = COLOR::GetBrighterColor(brightness, colors.color1);
  colors.color2 = COLOR::GetBrighterColor(brightness, colors.color2);
}

} // namespace GOOM::DRAW::SHAPE_DRAWERS
