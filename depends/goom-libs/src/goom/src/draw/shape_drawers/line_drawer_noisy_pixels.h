#pragma once

#include "draw/goom_draw.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "line_drawer.h"
#include "point2d.h"
#include "utils/math/goom_rand_base.h"

#include <memory>
#include <vector>

namespace GOOM::DRAW::SHAPE_DRAWERS
{

class LineDrawerNoisyPixels
{
public:
  LineDrawerNoisyPixels(IGoomDraw& draw, const UTILS::MATH::IGoomRand& goomRand) noexcept;
  LineDrawerNoisyPixels(const LineDrawerNoisyPixels&) = delete;
  LineDrawerNoisyPixels(LineDrawerNoisyPixels&&) noexcept;
  ~LineDrawerNoisyPixels() noexcept;
  auto operator=(const LineDrawerNoisyPixels&) -> LineDrawerNoisyPixels& = delete;
  auto operator=(LineDrawerNoisyPixels&&) -> LineDrawerNoisyPixels&      = delete;

  auto Init() noexcept -> void;

  auto SetNoiseWidth(uint8_t noiseWidth) noexcept -> void;
  auto SetNumNoisePixelsPerPixel(uint8_t numNoisePixels) noexcept -> void;
  auto SetLineThickness(uint8_t thickness) noexcept -> void;

  auto DrawLine(const Point2dInt& point1, const Point2dInt& point2, const Pixel& color) noexcept
      -> void;
  auto DrawLine(const Point2dInt& point1,
                const Point2dInt& point2,
                const std::vector<Pixel>& colors) noexcept -> void;

private:
  IGoomDraw& m_draw;
  const UTILS::MATH::IGoomRand& m_goomRand;

  class NoisyPixelDrawer;
  std::unique_ptr<NoisyPixelDrawer> m_noisyPixelDrawer{};
  std::unique_ptr<LineDrawer> m_lineDrawer{};
};

inline auto LineDrawerNoisyPixels::SetLineThickness(const uint8_t thickness) noexcept -> void
{
  Expects(m_lineDrawer != nullptr);
  m_lineDrawer->SetLineThickness(thickness);
}

inline auto LineDrawerNoisyPixels::DrawLine(const Point2dInt& point1,
                                            const Point2dInt& point2,
                                            const Pixel& color) noexcept -> void
{
  DrawLine(point1, point2, std::vector<Pixel>{color});
}

inline auto LineDrawerNoisyPixels::DrawLine(const Point2dInt& point1,
                                            const Point2dInt& point2,
                                            const std::vector<Pixel>& colors) noexcept -> void
{
  Expects(m_lineDrawer != nullptr);
  m_lineDrawer->DrawLine(point1, point2, colors);
}

} // namespace GOOM::DRAW::SHAPE_DRAWERS
