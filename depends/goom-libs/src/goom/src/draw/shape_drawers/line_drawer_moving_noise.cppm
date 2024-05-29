module;

#include "goom/goom_config.h"

#include <cstdint>

export module Goom.Draw.ShaperDrawers.LineDrawerMovingNoise;

import Goom.Draw.GoomDrawBase;
import Goom.Draw.ShaperDrawers.LineDrawerNoisyPixels;
import Goom.Utils.Math.GoomRandBase;
import Goom.Utils.Math.IncrementedValues;
import Goom.Utils.Math.Misc;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

export namespace GOOM::DRAW::SHAPE_DRAWERS
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

  [[nodiscard]] auto GetCurrentNumNoisePixelsPerPixel() const noexcept -> uint8_t;
  auto SetCurrentNumNoisePixelsPerPixel(uint8_t numNoisePixelsPerPixel) noexcept -> void;

  auto SetMinMaxNoiseValues(const MinMaxValues<uint8_t>& minMaxNoiseRadius,
                            const MinMaxValues<uint8_t>& minMaxNumNoisePixelsPerPixel) noexcept
      -> void;

  auto SetLineThickness(uint8_t thickness) noexcept -> void;
  auto SetBrightnessFactor(float brightnessFactor) noexcept -> void;
  auto SetNoiseColors(const MultiplePixels& colors) noexcept -> void;
  auto SetUseMainColorsForNoise(bool value) noexcept -> void;
  auto SetUseMainPointWithoutNoise(bool useMainPointWithoutNoise) noexcept -> void;

  auto IncrementNoise() noexcept -> void;

  auto DrawLine(const Point2dInt& point1,
                const Point2dInt& point2,
                const MultiplePixels& colors) noexcept -> void;

private:
  LineDrawerNoisyPixels m_lineDrawer;

  UTILS::MATH::IncrementedValue<uint8_t> m_noiseRadius;
  UTILS::MATH::IncrementedValue<uint8_t> m_numNoisePixelsPerPixel;

  auto UpdateLineDrawerNoise() noexcept -> void;
};

} // namespace GOOM::DRAW::SHAPE_DRAWERS

namespace GOOM::DRAW::SHAPE_DRAWERS
{

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

inline auto LineDrawerMovingNoise::SetMinMaxNoiseValues(
    const MinMaxValues<uint8_t>& minMaxNoiseRadius,
    const MinMaxValues<uint8_t>& minMaxNumNoisePixelsPerPixel) noexcept -> void
{
  m_noiseRadius.SetValues(minMaxNoiseRadius.minValue, minMaxNoiseRadius.maxValue);
  m_numNoisePixelsPerPixel.SetValues(minMaxNumNoisePixelsPerPixel.minValue,
                                     minMaxNumNoisePixelsPerPixel.maxValue);
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

inline auto LineDrawerMovingNoise::IncrementNoise() noexcept -> void
{
  m_noiseRadius.Increment();
  m_numNoisePixelsPerPixel.Increment();
  UpdateLineDrawerNoise();
}

inline auto LineDrawerMovingNoise::DrawLine(const Point2dInt& point1,
                                            const Point2dInt& point2,
                                            const MultiplePixels& colors) noexcept -> void
{
  m_lineDrawer.DrawLine(point1, point2, colors);
}

inline auto LineDrawerMovingNoise::UpdateLineDrawerNoise() noexcept -> void
{
  m_lineDrawer.SetNoiseParams({m_noiseRadius(), m_numNoisePixelsPerPixel()});
}

} // namespace GOOM::DRAW::SHAPE_DRAWERS
