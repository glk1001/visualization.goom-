#undef NO_LOGGING

#include "line_drawer_noisy_pixels.h"

#include "drawer_types.h"
#include "goom_config.h"
#include "goom_logger.h"

namespace GOOM::DRAW::SHAPE_DRAWERS
{

using UTILS::MATH::IGoomRand;

class LineDrawerNoisyPixels::NoisyPixelDrawer
{
public:
  NoisyPixelDrawer(IGoomDraw& draw, const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto SetNoiseWidth(uint8_t noiseWidth) noexcept -> void;
  auto SetNumNoisePixelsPerPixel(uint8_t numNoisePixels) noexcept -> void;

  auto DrawPixels(const Point2dInt& point, const MultiplePixels& colors) noexcept -> void;

private:
  IGoomDraw& m_draw;
  const UTILS::MATH::IGoomRand& m_goomRand;
  int32_t m_noiseWidth     = 0;
  int32_t m_numNoisePixels = 1;
};

LineDrawerNoisyPixels::NoisyPixelDrawer::NoisyPixelDrawer(IGoomDraw& draw,
                                                          const IGoomRand& goomRand) noexcept
  : m_draw{draw}, m_goomRand{goomRand}
{
}

inline auto LineDrawerNoisyPixels::NoisyPixelDrawer::SetNoiseWidth(
    const uint8_t noiseWidth) noexcept -> void
{
  m_noiseWidth = noiseWidth;
}

inline auto LineDrawerNoisyPixels::NoisyPixelDrawer::SetNumNoisePixelsPerPixel(
    const uint8_t numNoisePixels) noexcept -> void
{
  Expects(numNoisePixels >= 1U);
  m_numNoisePixels = numNoisePixels;
}

auto LineDrawerNoisyPixels::NoisyPixelDrawer::DrawPixels(const Point2dInt& point,
                                                         const MultiplePixels& colors) noexcept
    -> void
{
  if (0 == m_noiseWidth)
  {
    //TODO(glk) Can this be optimized?
    for (auto i = 0; i < m_numNoisePixels; ++i)
    {
      m_draw.DrawClippedPixels(point, colors);
    }
    return;
  }

  for (auto i = 0; i < m_numNoisePixels; ++i)
  {
    auto noisyPoint = point;
    noisyPoint.x += m_goomRand.GetRandInRange(-m_noiseWidth, m_noiseWidth + 1);
    noisyPoint.y += m_goomRand.GetRandInRange(-m_noiseWidth, m_noiseWidth + 1);
    m_draw.DrawClippedPixels(noisyPoint, colors);
  }
}

LineDrawerNoisyPixels::LineDrawerNoisyPixels(IGoomDraw& draw, const IGoomRand& goomRand) noexcept
  : m_draw{draw}, m_goomRand{goomRand}
{
}

LineDrawerNoisyPixels::LineDrawerNoisyPixels(LineDrawerNoisyPixels&&) noexcept = default;

LineDrawerNoisyPixels::~LineDrawerNoisyPixels() noexcept = default;

auto LineDrawerNoisyPixels::Init() noexcept -> void
{
  if (m_noisyPixelDrawer != nullptr)
  {
    return;
  }

  m_noisyPixelDrawer = std::make_unique<NoisyPixelDrawer>(m_draw, m_goomRand);
  m_lineDrawer =
      std::make_unique<LineDrawer>([this](const Point2dInt& point, const MultiplePixels& colors)
                                   { m_noisyPixelDrawer->DrawPixels(point, colors); });
}

auto LineDrawerNoisyPixels::SetNoiseWidth(const uint8_t noiseWidth) noexcept -> void
{
  Expects(m_lineDrawer != nullptr);
  m_noisyPixelDrawer->SetNoiseWidth(noiseWidth);
}

auto LineDrawerNoisyPixels::SetNumNoisePixelsPerPixel(const uint8_t numNoisePixels) noexcept -> void
{
  Expects(m_lineDrawer != nullptr);
  m_noisyPixelDrawer->SetNumNoisePixelsPerPixel(numNoisePixels);
}

} // namespace GOOM::DRAW::SHAPE_DRAWERS
