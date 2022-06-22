#pragma once

#include "color/color_adjustment.h"
#include "draw/goom_draw.h"
#include "goom_graphic.h"

#include <cstdint>
#include <vector>

namespace GOOM
{

namespace DRAW
{
class IGoomDraw;
}

namespace UTILS::GRAPHICS
{
class ImageBitmap;
class SmallImageBitmaps;
}
namespace UTILS::MATH
{
class IGoomRand;
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
  _num // unused, and marks the enum end
};

class LowDensityBlurrer
{
public:
  LowDensityBlurrer() noexcept = delete;
  LowDensityBlurrer(DRAW::IGoomDraw& draw,
                    const UTILS::MATH::IGoomRand& goomRand,
                    uint32_t width,
                    const Colorizer* colorizer,
                    const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] auto GetWidth() const -> uint32_t;
  void SetWidth(uint32_t val);

  void SetColorMode(BlurrerColorMode colorMode);
  void SetSingleColor(const Pixel& color);

  void SetNeighbourMixFactor(float neighbourMixFactor);

  void DoBlur(std::vector<IfsPoint>& lowDensityPoints, uint32_t maxLowDensityCount) const;

private:
  DRAW::IGoomDraw& m_draw;
  const UTILS::MATH::IGoomRand& m_goomRand;
  uint32_t m_width;
  const UTILS::GRAPHICS::SmallImageBitmaps& m_smallBitmaps;
  const UTILS::GRAPHICS::ImageBitmap* m_currentImageBitmap{};
  const Colorizer* const m_colorizer;
  float m_neighbourMixFactor = 1.0;
  BlurrerColorMode m_colorMode{};
  Pixel m_singleColor{};

  [[nodiscard]] auto GetImageBitmap(bool useBitmaps) const -> const UTILS::GRAPHICS::ImageBitmap*;
  [[nodiscard]] auto GetBrightness() const -> float;
  void SetPointColor(IfsPoint& point,
                     float t,
                     float logMaxLowDensityCount,
                     const DRAW::MultiplePixels& neighbours) const;
  [[nodiscard]] auto GetMixedPointColor(const Pixel& baseColor,
                                        const IfsPoint& point,
                                        const DRAW::MultiplePixels& neighbours,
                                        float brightness,
                                        float logAlpha) const -> Pixel;

  static constexpr float GAMMA = 2.2F;
  const COLOR::ColorAdjustment m_colorAdjust{GAMMA};
};

inline auto LowDensityBlurrer::GetWidth() const -> uint32_t
{
  return m_width;
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
