#pragma once

#include "goom_config.h"
#include "goom_types.h"
#include "point2d.h"

#include <cstdint>

namespace GOOM::DRAW::SHAPE_DRAWERS
{

class ClipTester
{
public:
  ClipTester(const Dimensions& screenDimensions, int32_t clipMargin) noexcept;

  [[nodiscard]] auto IsOutside(const Point2dInt& point) const noexcept;

private:
  struct ClipRect
  {
    int32_t x0;
    int32_t y0;
    int32_t x1;
    int32_t y1;
  };
  ClipRect m_clipRect;
};

inline ClipTester::ClipTester(const Dimensions& screenDimensions, int32_t clipMargin) noexcept
  : m_clipRect{clipMargin,
               clipMargin,
               (screenDimensions.GetIntWidth() - clipMargin) - 1,
               (screenDimensions.GetIntHeight() - clipMargin) - 1}
{
}

inline auto ClipTester::IsOutside(const Point2dInt& point) const noexcept
{
  return (point.x < m_clipRect.x0) or (point.y < m_clipRect.y0) or (point.x > m_clipRect.x1) or
         (point.y > m_clipRect.y1);
}

} // namespace GOOM::DRAW::SHAPE_DRAWERS
