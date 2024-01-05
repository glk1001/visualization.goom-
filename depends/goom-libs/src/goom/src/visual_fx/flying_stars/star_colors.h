#pragma once

#include "color/color_adjustment.h"
#include "color/color_maps.h"
#include "draw/goom_draw.h"
#include "goom/goom_config.h"
#include "goom/goom_graphic.h"
#include "goom/goom_types.h"

#include <memory>

namespace GOOM::VISUAL_FX::FLYING_STARS
{

class StarColors
{
public:
  struct ColorMapsSet
  {
    COLOR::ColorMapSharedPtr currentMainColorMapPtr  = nullptr;
    COLOR::ColorMapSharedPtr currentLowColorMapPtr   = nullptr;
    COLOR::ColorMapSharedPtr dominantMainColorMapPtr = nullptr;
    COLOR::ColorMapSharedPtr dominantLowColorMapPtr  = nullptr;
  };
  enum class ColorMode : UnderlyingEnumType
  {
    MIX_COLORS,
    REVERSE_MIX_COLORS,
    SINE_MIX_COLORS,
  };
  struct ColorProperties
  {
    ColorMapsSet colorMapsSet;
    ColorMode colorMode{};
    bool reverseWithinClusterMix{};
    bool similarLowColors{};
  };

  StarColors(const std::shared_ptr<const ColorProperties>& colorProperties,
             float withinClusterT) noexcept;

  struct MixedColorsParams
  {
    float brightness{};
    float lengthT{};
  };
  [[nodiscard]] auto GetMixedColors(const MixedColorsParams& mixedColorsParams) const noexcept
      -> DRAW::MultiplePixels;

private:
  std::shared_ptr<const ColorProperties> m_colorProperties;
  Pixel m_withinClusterMainColor;
  Pixel m_withinClusterLowColor;

  static constexpr auto GAMMA = 1.0F / 2.0F;
  COLOR::ColorAdjustment m_colorAdjust{
      {GAMMA, COLOR::ColorAdjustment::INCREASED_CHROMA_FACTOR}
  };
  [[nodiscard]] auto GetColorCorrection(float brightness, const Pixel& color) const noexcept
      -> Pixel;

  [[nodiscard]] auto GetColors(float t) const noexcept -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetReversedMixColors(float t) const noexcept -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetSineMixColors() const noexcept -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetFinalMixedColors(const MixedColorsParams& mixedColorsParams,
                                         const DRAW::MultiplePixels& colors) const noexcept
      -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetFinalTMix(float lengthT) const noexcept -> float;
};

} //namespace GOOM::VISUAL_FX::FLYING_STARS
