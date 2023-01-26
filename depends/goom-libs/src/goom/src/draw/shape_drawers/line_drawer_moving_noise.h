#pragma once

#include "draw/goom_draw.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "goom_types.h"
#include "line_drawer_noisy_pixels.h"
#include "point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/t_values.h"

#include <memory>
#include <vector>

namespace GOOM::DRAW::SHAPE_DRAWERS
{

class LineDrawerMovingNoise
{
public:
  LineDrawerMovingNoise(IGoomDraw& draw,
                        const UTILS::MATH::IGoomRand& goomRand,
                        const MinMaxValues<uint8_t>& minMaxNoiseRadius,
                        uint32_t numNoiseRadiusSteps,
                        const MinMaxValues<uint8_t>& minMaxNumNoisePixels,
                        uint32_t numNumPixelSteps) noexcept;

  [[nodiscard]] auto GetCurrentNoiseRadius() const noexcept -> uint8_t;
  auto SetCurrentNoiseRadius(uint8_t noiseRadius) noexcept -> void;

  auto SetMinNoiseRadius(uint8_t minNoiseRadius) noexcept -> void;
  auto SetMaxNoiseRadius(uint8_t maxNoiseRadius) noexcept -> void;
  auto SetMinMaxNoiseRadius(const MinMaxValues<uint8_t>& minMaxNoiseRadius) noexcept -> void;

  [[nodiscard]] auto GetCurrentNumNoisePixelsPerPixel() const noexcept -> uint8_t;
  auto SetCurrentNumNoisePixelsPerPixel(uint8_t numNoisePixelsPerPixel) noexcept -> void;

  auto SetMinNumNoisePixelsPerPixel(uint8_t minNumNoisePixels) noexcept -> void;
  auto SetMaxNumNoisePixelsPerPixel(uint8_t maxNumNoisePixels) noexcept -> void;
  auto SetMinMaxNumNoisePixelsPerPixel(
      const MinMaxValues<uint8_t>& minMaxNumNoisePixelsPerPixel) noexcept -> void;
  auto SetNumNoisePixelsPerPixel(const MinMaxValues<uint8_t>& minMaxNumNoisePixels) noexcept
      -> void;

  auto SetLineThickness(uint8_t thickness) noexcept -> void;
  auto SetBrightnessFactor(float brightnessFactor) noexcept -> void;
  auto SetNoiseColors(const MultiplePixels& colors) noexcept -> void;
  auto SetUseMainColorsForNoise(bool value) noexcept -> void;
  auto SetUseMainPointWithoutNoise(bool useMainPointWithoutNoise) noexcept -> void;

  auto IncrementNoise() noexcept -> void;

  auto DrawLine(const Point2dInt& point1, const Point2dInt& point2, const Pixel& color) noexcept
      -> void;
  auto DrawLine(const Point2dInt& point1,
                const Point2dInt& point2,
                const std::vector<Pixel>& colors) noexcept -> void;

private:
  LineDrawerNoisyPixels m_lineDrawer;

  UTILS::IncrementedValue<uint8_t> m_noiseRadius;
  UTILS::IncrementedValue<uint8_t> m_numNoisePixelsPerPixel;

  auto UpdateLineDrawerNoise() noexcept -> void;
};

inline auto LineDrawerMovingNoise::SetLineThickness(const uint8_t thickness) noexcept -> void
{
  m_lineDrawer.SetLineThickness(thickness);
}

inline auto LineDrawerMovingNoise::SetBrightnessFactor(const float brightnessFactor) noexcept
    -> void
{
  m_lineDrawer.SetBrightnessFactor(brightnessFactor);
}

inline auto LineDrawerMovingNoise::SetNoiseColors(const MultiplePixels& colors) noexcept -> void
{
  m_lineDrawer.SetNoiseColors(colors);
}

inline auto LineDrawerMovingNoise::SetUseMainColorsForNoise(const bool value) noexcept -> void
{
  m_lineDrawer.SetUseMainColorsForNoise(value);
}

inline auto LineDrawerMovingNoise::SetUseMainPointWithoutNoise(
    const bool useMainPointWithoutNoise) noexcept -> void
{
  m_lineDrawer.SetUseMainPointWithoutNoise(useMainPointWithoutNoise);
}

inline auto LineDrawerMovingNoise::GetCurrentNoiseRadius() const noexcept -> uint8_t
{
  return m_noiseRadius();
}

inline auto LineDrawerMovingNoise::SetCurrentNoiseRadius(const uint8_t noiseRadius) noexcept -> void
{
  m_noiseRadius.ResetCurrentValue(noiseRadius);
}

inline auto LineDrawerMovingNoise::SetMinNoiseRadius(const uint8_t minNoiseRadius) noexcept -> void
{
  m_noiseRadius.SetValue1(minNoiseRadius);
  UpdateLineDrawerNoise();
}

inline auto LineDrawerMovingNoise::SetMaxNoiseRadius(const uint8_t maxNoiseRadius) noexcept -> void
{
  m_noiseRadius.SetValue2(maxNoiseRadius);
  UpdateLineDrawerNoise();
}

inline auto LineDrawerMovingNoise::SetMinMaxNoiseRadius(
    const MinMaxValues<uint8_t>& minMaxNoiseRadius) noexcept -> void
{
  m_noiseRadius.SetValues(minMaxNoiseRadius.minValue, minMaxNoiseRadius.maxValue);
  UpdateLineDrawerNoise();
}

inline auto LineDrawerMovingNoise::GetCurrentNumNoisePixelsPerPixel() const noexcept -> uint8_t
{
  return m_numNoisePixelsPerPixel();
}

inline auto LineDrawerMovingNoise::SetCurrentNumNoisePixelsPerPixel(
    const uint8_t numNoisePixelsPerPixel) noexcept -> void
{
  m_numNoisePixelsPerPixel.ResetCurrentValue(numNoisePixelsPerPixel);
}

inline auto LineDrawerMovingNoise::SetMinNumNoisePixelsPerPixel(
    const uint8_t minNumNoisePixels) noexcept -> void
{
  m_numNoisePixelsPerPixel.SetValue1(minNumNoisePixels);
  UpdateLineDrawerNoise();
}

inline auto LineDrawerMovingNoise::SetMaxNumNoisePixelsPerPixel(
    const uint8_t maxNumNoisePixels) noexcept -> void
{
  m_numNoisePixelsPerPixel.SetValue2(maxNumNoisePixels);
  UpdateLineDrawerNoise();
}

inline auto LineDrawerMovingNoise::SetMinMaxNumNoisePixelsPerPixel(
    const MinMaxValues<uint8_t>& minMaxNumNoisePixelsPerPixel) noexcept -> void
{
  m_numNoisePixelsPerPixel.SetValues(minMaxNumNoisePixelsPerPixel.minValue,
                                     minMaxNumNoisePixelsPerPixel.maxValue);
}

inline auto LineDrawerMovingNoise::SetNumNoisePixelsPerPixel(
    const MinMaxValues<uint8_t>& minMaxNumNoisePixels) noexcept -> void
{
  m_numNoisePixelsPerPixel.SetValues(minMaxNumNoisePixels.minValue, minMaxNumNoisePixels.maxValue);
  UpdateLineDrawerNoise();
}

inline auto LineDrawerMovingNoise::IncrementNoise() noexcept -> void
{
  m_noiseRadius.Increment();
  m_numNoisePixelsPerPixel.Increment();
  UpdateLineDrawerNoise();
}

inline auto LineDrawerMovingNoise::UpdateLineDrawerNoise() noexcept -> void
{
  m_lineDrawer.SetNoiseRadius(m_noiseRadius());
  m_lineDrawer.SetNumNoisePixelsPerPixel(m_numNoisePixelsPerPixel());
}

inline auto LineDrawerMovingNoise::DrawLine(const Point2dInt& point1,
                                            const Point2dInt& point2,
                                            const Pixel& color) noexcept -> void
{
  DrawLine(point1, point2, std::vector<Pixel>{color});
}

inline auto LineDrawerMovingNoise::DrawLine(const Point2dInt& point1,
                                            const Point2dInt& point2,
                                            const std::vector<Pixel>& colors) noexcept -> void
{
  m_lineDrawer.DrawLine(point1, point2, colors);
}

} // namespace GOOM::DRAW::SHAPE_DRAWERS
