#pragma once

#include "color/colorutils.h"
#include "goom_graphic.h"

#include <cstdint>
#include <vector>

namespace GOOM
{

namespace DRAW
{
class IGoomDraw;
}

namespace VISUAL_FX::IFS
{

class IfsPoint;
class Colorizer;

enum class BlurrerColorMode
{
  SMOOTH_WITH_NEIGHBOURS,
  SMOOTH_NO_NEIGHBOURS,
  SIMI_WITH_NEIGHBOURS,
  SIMI_NO_NEIGHBOURS,
  SINGLE_WITH_NEIGHBOURS,
  SINGLE_NO_NEIGHBOURS,
  _num // unused and must be last
};

class LowDensityBlurrer
{
public:
  LowDensityBlurrer() noexcept = delete;
  LowDensityBlurrer(DRAW::IGoomDraw& draw, uint32_t width, const Colorizer* colorizer) noexcept;

  [[nodiscard]] auto GetWidth() const -> uint32_t;
  void SetWidth(uint32_t val);

  void SetColorMode(BlurrerColorMode colorMode);
  void SetSingleColor(const Pixel& color);

  void SetNeighbourMixFactor(float neighbourMixFactor);

  void DoBlur(std::vector<IfsPoint>& lowDensityPoints, uint32_t maxLowDensityCount) const;

private:
  DRAW::IGoomDraw& m_draw;
  uint32_t m_width;
  const Colorizer* const m_colorizer{};
  float m_neighbourMixFactor = 1.0;
  BlurrerColorMode m_colorMode{};
  Pixel m_singleColor{};

  void SetPointColor(IfsPoint& point,
                     float t,
                     float logMaxLowDensityCount,
                     const std::vector<Pixel>& neighbours) const;
  [[nodiscard]] auto GetMixedPointColor(const Pixel& baseColor,
                                        const IfsPoint& point,
                                        const std::vector<Pixel>& neighbours,
                                        const float brightness,
                                        const float logAlpha) const -> Pixel;

  static constexpr float GAMMA = 2.2F;
  static constexpr float GAMMA_BRIGHTNESS_THRESHOLD = 0.01F;
  const COLOR::GammaCorrection m_gammaCorrect{GAMMA, GAMMA_BRIGHTNESS_THRESHOLD};
};

inline auto LowDensityBlurrer::GetWidth() const -> uint32_t
{
  return m_width;
}

inline void LowDensityBlurrer::SetColorMode(const BlurrerColorMode colorMode)
{
  m_colorMode = colorMode;
}

inline void LowDensityBlurrer::SetSingleColor(const Pixel& color)
{
  m_singleColor = color;
}

inline void LowDensityBlurrer::SetNeighbourMixFactor(const float neighbourMixFactor)
{
  m_neighbourMixFactor = neighbourMixFactor;
}

} // namespace VISUAL_FX::IFS
} // namespace GOOM
