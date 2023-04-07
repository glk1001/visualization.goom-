#pragma once

#include "color/color_maps.h"
#include "color/color_utils.h"
#include "goom_graphic.h"
#include "utils/graphics/pixel_utils.h"
#include "utils/math/misc.h"

#include <cstdint>

namespace GOOM::UTILS::GRAPHICS
{

// NOTE: Tried reverse add color (where oldColor is multiplied by intensity),
//       but the resulting black pixels don't look good.
[[nodiscard]] constexpr auto GetColorAddPixelBlend(const Pixel& oldColor,
                                                   const Pixel& newColor,
                                                   uint32_t intBuffIntensity) -> Pixel;

[[nodiscard]] constexpr auto GetDarkenOnlyPixelBlend(const Pixel& oldColor,
                                                     const Pixel& newColor,
                                                     uint32_t intBuffIntensity) -> Pixel;

[[nodiscard]] constexpr auto GetLightenOnlyPixelBlend(const Pixel& oldColor,
                                                      const Pixel& newColor,
                                                      uint32_t intBuffIntensity) -> Pixel;

[[nodiscard]] constexpr auto GetSameLumaMixPixelBlend(float lumaMixT,
                                                      const Pixel& oldColor,
                                                      const Pixel& newColor,
                                                      uint32_t intBuffIntensity) -> Pixel;

[[nodiscard]] constexpr auto GetColorMultiplyPixelBlend(const Pixel& oldColor,
                                                        const Pixel& newColor,
                                                        uint32_t intBuffIntensity) -> Pixel;

constexpr auto GetColorAddPixelBlend(const Pixel& oldColor,
                                     const Pixel& newColor,
                                     const uint32_t intBuffIntensity) -> Pixel
{
  return GetColorAdd(oldColor, COLOR::GetBrighterColorInt(intBuffIntensity, newColor));
}

constexpr auto GetColorMultiplyPixelBlend(const Pixel& oldColor,
                                          const Pixel& newColor,
                                          const uint32_t intBuffIntensity) -> Pixel
{
  constexpr auto BRIGHTNESS_INCREASE = 2.0F;
  constexpr auto BLACK_THRESHOLD     = 30U;

  if (COLOR::IsCloseToBlack(newColor, BLACK_THRESHOLD))
  {
    return COLOR::GetBrighterColor(BRIGHTNESS_INCREASE, oldColor);
  }

  if (COLOR::IsCloseToBlack(oldColor, BLACK_THRESHOLD))
  {
    return COLOR::GetBrighterColor(BRIGHTNESS_INCREASE, newColor);
  }

  return GetColorAdd(GetColorMultiply(oldColor, newColor),
                     COLOR::GetBrighterColorInt(intBuffIntensity, newColor));
}

constexpr auto GetDarkenOnlyPixelBlend(const Pixel& oldColor,
                                       const Pixel& newColor,
                                       const uint32_t intBuffIntensity) -> Pixel
{
  constexpr auto BRIGHTNESS_INCREASE = 2.0F;
  constexpr auto BLACK_THRESHOLD     = 30U;

  if (COLOR::IsCloseToBlack(newColor, BLACK_THRESHOLD))
  {
    return COLOR::GetBrighterColor(BRIGHTNESS_INCREASE, oldColor);
  }

  if (COLOR::IsCloseToBlack(oldColor, BLACK_THRESHOLD))
  {
    return COLOR::GetBrighterColor(BRIGHTNESS_INCREASE, newColor);
  }

  return GetColorMin(oldColor, COLOR::GetBrighterColorInt(intBuffIntensity, newColor));
}

constexpr auto GetLightenOnlyPixelBlend(const Pixel& oldColor,
                                        const Pixel& newColor,
                                        const uint32_t intBuffIntensity) -> Pixel
{
  return GetColorMax(oldColor, COLOR::GetBrighterColorInt(intBuffIntensity, newColor));
}

constexpr auto GetSameLumaMixPixelBlend(const float lumaMixT,
                                        const Pixel& oldColor,
                                        const Pixel& newColor,
                                        const uint32_t intBuffIntensity) -> Pixel
{
  constexpr auto SMALL_LUMA = 0.1F;

  const auto newColorLuma = COLOR::GetLuma(newColor);
  if (newColorLuma < SMALL_LUMA)
  {
    return GetColorAdd(oldColor, newColor);
  }

  const auto oldColorLuma = COLOR::GetLuma(oldColor);
  if (oldColorLuma < SMALL_LUMA)
  {
    return GetColorAdd(oldColor, newColor);
  }

  const auto buffIntensity = static_cast<float>(intBuffIntensity) / channel_limits<float>::max();

  const auto brightness = std::min((oldColorLuma + (buffIntensity * newColorLuma)) /
                                       STD20::lerp(oldColorLuma, newColorLuma, lumaMixT),
                                   COLOR::MAX_BRIGHTNESS);

  const auto finalNewColor = COLOR::IColorMap::GetColorMix(oldColor, newColor, lumaMixT);
  return COLOR::GetBrighterColor(brightness, finalNewColor);
}

} // namespace GOOM::UTILS::GRAPHICS
