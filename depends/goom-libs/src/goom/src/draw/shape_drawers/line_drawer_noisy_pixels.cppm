module;

#include "goom/goom_config.h"

#include <array>
#include <cstdint>
#include <vector>

export module Goom.Draw.ShaperDrawers.LineDrawerNoisyPixels;

import Goom.Draw.GoomDrawBase;
import Goom.Draw.ShaperDrawers.LineDrawer;
import Goom.Utils.Math.GoomRandBase;
import Goom.Lib.Point2d;

export namespace GOOM::DRAW::SHAPE_DRAWERS
{

class LineDrawerNoisyPixels
{
public:
  struct NoiseParams
  {
    uint8_t noiseRadius;
    uint8_t numNoisePixelsPerPixel;
  };

  LineDrawerNoisyPixels(IGoomDraw& draw,
                        const UTILS::MATH::IGoomRand& goomRand,
                        const NoiseParams& noiseParams) noexcept;

  auto SetNoiseParams(const NoiseParams& noiseParams) noexcept -> void;
  auto SetLineThickness(uint8_t thickness) noexcept -> void;
  auto SetBrightnessFactor(float brightnessFactor) noexcept -> void;
  auto SetNoiseColors(const MultiplePixels& colors) noexcept -> void;
  auto SetUseMainColorsForNoise(bool value) noexcept -> void;
  auto SetUseMainPointWithoutNoise(bool useMainPointWithoutNoise) noexcept -> void;

  auto DrawLine(const Point2dInt& point1,
                const Point2dInt& point2,
                const MultiplePixels& colors) noexcept -> void;

private:
  class NoisyPixelDrawer
  {
  public:
    NoisyPixelDrawer(IGoomDraw& draw,
                     const UTILS::MATH::IGoomRand& goomRand,
                     const NoiseParams& noiseParams) noexcept;

    auto SetNoiseParams(const NoiseParams& noiseParams) noexcept -> void;
    auto SetBrightnessFactor(float brightnessFactor) noexcept -> void;
    auto SetNoiseColors(const MultiplePixels& colors) noexcept -> void;
    auto SetUseMainColorsForNoise(bool useMainColorsForNoise) noexcept -> void;
    auto SetUseMainPointWithoutNoise(bool useMainPointWithoutNoise) noexcept -> void;

    auto DrawPixels(const Point2dInt& point, float brightness, MultiplePixels colors) noexcept
        -> void;

  private:
    IGoomDraw* m_draw;
    const UTILS::MATH::IGoomRand* m_goomRand;
    float m_brightnessReducer       = 1.0F;
    float m_brightnessFactor        = 1.0F;
    float m_overallBrightnessFactor = 1.0F;
    bool m_useMainColorsForNoise    = true;
    bool m_useMainPointWithoutNoise = true;
    MultiplePixels m_noiseColors{};

    int32_t m_noiseRadius;
    int32_t m_numNoisePixelsPerPixel;
    static constexpr auto PROB_PURE_NOISE = 0.5F;
    bool m_usePureNoise                   = m_goomRand->ProbabilityOf(PROB_PURE_NOISE);
    auto SetBrightnessValues() noexcept -> void;
    auto DrawMainPoint(const Point2dInt& point, float brightness, MultiplePixels& colors) noexcept
        -> void;
    auto DrawNoisePoints(const Point2dInt& point, float brightness, MultiplePixels& colors) noexcept
        -> void;
    auto DrawPureNoisePoints(const Point2dInt& point, const MultiplePixels& colors) noexcept
        -> void;

    using NoisePerPixelList                         = std::vector<int32_t>;
    static constexpr auto NUM_NOISE_PER_PIXEL_LISTS = 5U;
    std::array<NoisePerPixelList, NUM_NOISE_PER_PIXEL_LISTS> m_noisePerPixelList;
    uint32_t m_currentNoisePerPixelIndex = 0U;
    auto SetNoisePerPixel() noexcept -> void;
    auto IncrementCurrentNoisePerPixelIndex() noexcept -> void;
    auto DrawPatternedNoisePoints(const Point2dInt& point, const MultiplePixels& colors) noexcept
        -> void;
  };

  LineDrawer<NoisyPixelDrawer> m_lineDrawer;
};

} // namespace GOOM::DRAW::SHAPE_DRAWERS

namespace GOOM::DRAW::SHAPE_DRAWERS
{

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

inline auto LineDrawerNoisyPixels::SetUseMainPointWithoutNoise(
    const bool useMainPointWithoutNoise) noexcept -> void
{
  m_lineDrawer.GetDrawPixelPolicy().SetUseMainPointWithoutNoise(useMainPointWithoutNoise);
}

inline auto LineDrawerNoisyPixels::SetNoiseParams(const NoiseParams& noiseParams) noexcept -> void
{
  m_lineDrawer.GetDrawPixelPolicy().SetNoiseParams(noiseParams);
}

inline auto LineDrawerNoisyPixels::DrawLine(const Point2dInt& point1,
                                            const Point2dInt& point2,
                                            const MultiplePixels& colors) noexcept -> void
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

inline auto LineDrawerNoisyPixels::NoisyPixelDrawer::SetUseMainPointWithoutNoise(
    const bool useMainPointWithoutNoise) noexcept -> void
{
  m_useMainPointWithoutNoise = useMainPointWithoutNoise;
}

inline auto LineDrawerNoisyPixels::NoisyPixelDrawer::SetNoiseParams(
    const NoiseParams& noiseParams) noexcept -> void
{
  if ((noiseParams.noiseRadius == m_noiseRadius) and
      (noiseParams.numNoisePixelsPerPixel == m_numNoisePixelsPerPixel))
  {
    return;
  }

  Expects(noiseParams.numNoisePixelsPerPixel >= 1U);

  m_noiseRadius            = noiseParams.noiseRadius;
  m_numNoisePixelsPerPixel = noiseParams.numNoisePixelsPerPixel;
  m_usePureNoise           = m_goomRand->ProbabilityOf(PROB_PURE_NOISE);

  SetBrightnessValues();
  SetNoisePerPixel();
}

inline auto LineDrawerNoisyPixels::NoisyPixelDrawer::SetBrightnessValues() noexcept -> void
{
  m_brightnessReducer       = 1.0F / static_cast<float>(m_numNoisePixelsPerPixel);
  m_overallBrightnessFactor = m_brightnessFactor * m_brightnessReducer;
}

} // namespace GOOM::DRAW::SHAPE_DRAWERS
