#pragma once

#include "goom/goom_config.h"
#include "goom/goom_graphic.h"
#include "goom/point2d.h"
#include "line_drawer.h"
#include "line_drawer_moving_noise.h"
#include "line_drawer_noisy_pixels.h"

namespace GOOM::DRAW::SHAPE_DRAWERS
{

class ILineDrawerWithEffects
{
public:
  ILineDrawerWithEffects()                                                 = default;
  ILineDrawerWithEffects(const ILineDrawerWithEffects&)                    = default;
  ILineDrawerWithEffects(ILineDrawerWithEffects&&)                         = default;
  virtual ~ILineDrawerWithEffects()                                        = default;
  auto operator=(const ILineDrawerWithEffects&) -> ILineDrawerWithEffects& = default;
  auto operator=(ILineDrawerWithEffects&&) -> ILineDrawerWithEffects&      = default;

  virtual auto SetLineThickness(uint8_t thickness) noexcept -> void = 0;

  virtual auto DrawLine(const Point2dInt& point1,
                        const Point2dInt& point2,
                        const MultiplePixels& colors) noexcept -> void = 0;
};

template<class LineDrawerPolicy>
class LineDrawerWithEffects : public ILineDrawerWithEffects
{
public:
  explicit LineDrawerWithEffects(LineDrawerPolicy&& lineDrawerPolicy) noexcept;

  [[nodiscard]] auto GetLineDrawer() const noexcept -> const LineDrawerPolicy&;
  [[nodiscard]] auto GetLineDrawer() noexcept -> LineDrawerPolicy&;

  auto SetLineThickness(uint8_t thickness) noexcept -> void override;

  auto DrawLine(const Point2dInt& point1,
                const Point2dInt& point2,
                const MultiplePixels& colors) noexcept -> void override;

private:
  LineDrawerPolicy m_lineDrawer;
};

template<class LineDrawerPolicy>
inline LineDrawerWithEffects<LineDrawerPolicy>::LineDrawerWithEffects(
    LineDrawerPolicy&& lineDrawerPolicy) noexcept
  : m_lineDrawer{std::move(lineDrawerPolicy)}
{
}

template<class LineDrawerPolicy>
inline auto LineDrawerWithEffects<LineDrawerPolicy>::GetLineDrawer() const noexcept
    -> const LineDrawerPolicy&
{
  return m_lineDrawer;
}

template<class LineDrawerPolicy>
inline auto LineDrawerWithEffects<LineDrawerPolicy>::GetLineDrawer() noexcept -> LineDrawerPolicy&
{
  return m_lineDrawer;
}

template<class LineDrawerPolicy>
inline auto LineDrawerWithEffects<LineDrawerPolicy>::SetLineThickness(
    const uint8_t thickness) noexcept -> void
{
  m_lineDrawer.SetLineThickness(thickness);
}

template<class LineDrawerPolicy>
inline auto LineDrawerWithEffects<LineDrawerPolicy>::DrawLine(const Point2dInt& point1,
                                                              const Point2dInt& point2,
                                                              const MultiplePixels& colors) noexcept
    -> void
{
  m_lineDrawer.DrawLine(point1, point2, colors);
}

using LineDrawerWithMovingNoiseEffect = LineDrawerWithEffects<LineDrawerMovingNoise>;

} // namespace GOOM::DRAW::SHAPE_DRAWERS
