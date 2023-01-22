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
  LineDrawerNoisyPixels(IGoomDraw& draw,
                        const UTILS::MATH::IGoomRand& goomRand,
                        uint8_t noiseRadius,
                        uint8_t numNoisePixels) noexcept;

  [[nodiscard]] auto GetNoiseRadius() const noexcept -> uint8_t;
  auto SetNoiseRadius(uint8_t noiseRadius) noexcept -> void;

  [[nodiscard]] auto GetNumNoisePixelsPerPixel() const noexcept -> uint8_t;
  auto SetNumNoisePixelsPerPixel(uint8_t numNoisePixels) noexcept -> void;

  auto SetLineThickness(uint8_t thickness) noexcept -> void;
  auto SetBrightnessFactor(float brightnessFactor) noexcept -> void;
  auto SetNoiseColors(const MultiplePixels& colors) noexcept -> void;
  auto SetUseMainColorsForNoise(bool value) noexcept -> void;

  auto DrawLine(const Point2dInt& point1, const Point2dInt& point2, const Pixel& color) noexcept
      -> void;
  auto DrawLine(const Point2dInt& point1,
                const Point2dInt& point2,
                const std::vector<Pixel>& colors) noexcept -> void;

private:
  class NoisyPixelDrawer
  {
  public:
    NoisyPixelDrawer(IGoomDraw& draw,
                     const UTILS::MATH::IGoomRand& goomRand,
                     uint8_t noiseRadius,
                     uint8_t numNoisePixels) noexcept;

    auto SetBrightnessFactor(float brightnessFactor) noexcept -> void;
    auto SetNoiseColors(const MultiplePixels& colors) noexcept -> void;
    auto SetUseMainColorsForNoise(bool useMainColorsForNoise) noexcept -> void;

    [[nodiscard]] auto GetNoiseRadius() const noexcept -> uint8_t;
    auto SetNoiseRadius(uint8_t noiseRadius) noexcept -> void;

    [[nodiscard]] auto GetNumNoisePixelsPerPixel() const noexcept -> uint8_t;
    auto SetNumNoisePixelsPerPixel(uint8_t numNoisePixels) noexcept -> void;

    auto DrawPixels(const Point2dInt& point, float brightness, MultiplePixels colors) noexcept
        -> void;

  private:
    IGoomDraw& m_draw;
    const UTILS::MATH::IGoomRand& m_goomRand;
    int32_t m_noiseRadius           = 0;
    int32_t m_numNoisePixels        = 1;
    float m_brightnessReducer       = 1.0F;
    float m_brightnessFactor        = 1.0F;
    float m_overallBrightnessFactor = 1.0F;
    bool m_useMainColorsForNoise    = true;
    MultiplePixels m_noiseColors{};
  };

  LineDrawer<NoisyPixelDrawer> m_lineDrawer;
};

inline auto LineDrawerNoisyPixels::SetLineThickness(const uint8_t thickness) noexcept -> void
{
  m_lineDrawer.SetLineThickness(thickness);
}

inline auto LineDrawerNoisyPixels::SetBrightnessFactor(const float brightnessFactor) noexcept
    -> void
{
  m_lineDrawer.GetDrawPixelPolicy().SetBrightnessFactor(brightnessFactor);
}

inline auto LineDrawerNoisyPixels::SetNoiseColors(const MultiplePixels& colors) noexcept -> void
{
  m_lineDrawer.GetDrawPixelPolicy().SetNoiseColors(colors);
}

inline auto LineDrawerNoisyPixels::SetUseMainColorsForNoise(const bool value) noexcept -> void
{
  m_lineDrawer.GetDrawPixelPolicy().SetUseMainColorsForNoise(value);
}

inline auto LineDrawerNoisyPixels::GetNoiseRadius() const noexcept -> uint8_t
{
  return m_lineDrawer.GetDrawPixelPolicy().GetNoiseRadius();
}

inline auto LineDrawerNoisyPixels::SetNoiseRadius(const uint8_t noiseRadius) noexcept -> void
{
  m_lineDrawer.GetDrawPixelPolicy().SetNoiseRadius(noiseRadius);
}

inline auto LineDrawerNoisyPixels::GetNumNoisePixelsPerPixel() const noexcept -> uint8_t
{
  return m_lineDrawer.GetDrawPixelPolicy().GetNumNoisePixelsPerPixel();
}

inline auto LineDrawerNoisyPixels::SetNumNoisePixelsPerPixel(const uint8_t numNoisePixels) noexcept
    -> void
{
  m_lineDrawer.GetDrawPixelPolicy().SetNumNoisePixelsPerPixel(numNoisePixels);
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
  m_lineDrawer.DrawLine(point1, point2, colors);
}

inline auto LineDrawerNoisyPixels::NoisyPixelDrawer::SetBrightnessFactor(
    const float brightnessFactor) noexcept -> void
{
  m_brightnessFactor        = brightnessFactor;
  m_overallBrightnessFactor = m_brightnessFactor * m_brightnessReducer;
}

inline auto LineDrawerNoisyPixels::NoisyPixelDrawer::SetNoiseColors(
    const MultiplePixels& colors) noexcept -> void
{
  m_noiseColors = colors;
}

inline auto LineDrawerNoisyPixels::NoisyPixelDrawer::SetUseMainColorsForNoise(
    const bool useMainColorsForNoise) noexcept -> void
{
  m_useMainColorsForNoise = useMainColorsForNoise;
}

inline auto LineDrawerNoisyPixels::NoisyPixelDrawer::GetNoiseRadius() const noexcept -> uint8_t
{
  return static_cast<uint8_t>(m_noiseRadius);
}

inline auto LineDrawerNoisyPixels::NoisyPixelDrawer::SetNoiseRadius(
    const uint8_t noiseRadius) noexcept -> void
{
  m_noiseRadius = noiseRadius;
}

inline auto LineDrawerNoisyPixels::NoisyPixelDrawer::GetNumNoisePixelsPerPixel() const noexcept
    -> uint8_t
{
  return static_cast<uint8_t>(m_numNoisePixels);
}

inline auto LineDrawerNoisyPixels::NoisyPixelDrawer::SetNumNoisePixelsPerPixel(
    const uint8_t numNoisePixels) noexcept -> void
{
  Expects(numNoisePixels >= 1U);
  m_numNoisePixels          = numNoisePixels;
  m_brightnessReducer       = 1.0F / static_cast<float>(m_numNoisePixels);
  m_overallBrightnessFactor = m_brightnessFactor * m_brightnessReducer;
}

} // namespace GOOM::DRAW::SHAPE_DRAWERS
