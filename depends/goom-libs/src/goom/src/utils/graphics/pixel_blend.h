#pragma once

#include "color/color_maps.h"
#include "color/color_utils.h"
#include "goom/goom_graphic.h"
#include "goom/math20.h"
#include "utils/graphics/pixel_utils.h"

#include <algorithm>
#include <cstdint>

namespace GOOM::UTILS::GRAPHICS
{

// NOTE: Tried reverse add color (where oldColor is multiplied by intensity),
//       but the resulting black pixels don't look good.
[[nodiscard]] constexpr auto GetColorAddPixelBlend(const Pixel& bgndColor,
                                                   uint32_t fgndIntBuffIntensity,
                                                   const Pixel& fgndColor,
                                                   PixelChannelType newAlpha) -> Pixel;

[[nodiscard]] constexpr auto GetDarkenOnlyPixelBlend(const Pixel& bgndColor,
                                                     uint32_t fgndIntBuffIntensity,
                                                     const Pixel& fgndColor,
                                                     PixelChannelType newAlpha) -> Pixel;

[[nodiscard]] constexpr auto GetLightenOnlyPixelBlend(const Pixel& bgndColor,
                                                      uint32_t fgndIntBuffIntensity,
                                                      const Pixel& fgndColor,
                                                      PixelChannelType newAlpha) -> Pixel;

[[nodiscard]] constexpr auto GetSameLumaMixPixelBlend(float lumaMixT,
                                                      const Pixel& bgndColor,
                                                      uint32_t fgndIntBuffIntensity,
                                                      const Pixel& fgndColor,
                                                      PixelChannelType newAlpha) -> Pixel;

[[nodiscard]] constexpr auto GetColorMultiplyPixelBlend(const Pixel& bgndColor,
                                                        uint32_t fgndIntBuffIntensity,
                                                        const Pixel& fgndColor,
                                                        PixelChannelType newAlpha) -> Pixel;

[[nodiscard]] constexpr auto GetColorAlphaBlend(const Pixel& bgndColor,
                                                const Pixel& fgndColor,
                                                PixelChannelType newAlpha) -> Pixel;

[[nodiscard]] constexpr auto GetColorAlphaAndAddBlend(const Pixel& bgndColor,
                                                      uint32_t fgndIntBuffIntensity,
                                                      const Pixel& fgndColor,
                                                      PixelChannelType newAlpha) -> Pixel;

constexpr auto GetColorAddPixelBlend(const Pixel& bgndColor,
                                     const uint32_t fgndIntBuffIntensity,
                                     const Pixel& fgndColor,
                                     const PixelChannelType newAlpha) -> Pixel
{
  return GetColorAdd(
      bgndColor, COLOR::GetBrighterColorInt(fgndIntBuffIntensity, fgndColor), newAlpha);
}

constexpr auto GetColorMultiplyPixelBlend(const Pixel& bgndColor,
                                          const uint32_t fgndIntBuffIntensity,
                                          const Pixel& fgndColor,
                                          const PixelChannelType newAlpha) -> Pixel
{
  constexpr auto BRIGHTNESS_INCREASE = 2.0F;
  constexpr auto BLACK_THRESHOLD     = 30U;

  if (COLOR::IsCloseToBlack(fgndColor, BLACK_THRESHOLD))
  {
    return COLOR::GetBrighterColor(BRIGHTNESS_INCREASE, bgndColor);
  }

  if (COLOR::IsCloseToBlack(bgndColor, BLACK_THRESHOLD))
  {
    return COLOR::GetBrighterColor(BRIGHTNESS_INCREASE, fgndColor);
  }

  return GetColorAdd(GetColorMultiply(bgndColor, fgndColor, newAlpha),
                     COLOR::GetBrighterColorInt(fgndIntBuffIntensity, fgndColor),
                     newAlpha);
}

constexpr auto GetDarkenOnlyPixelBlend(const Pixel& bgndColor,
                                       const uint32_t fgndIntBuffIntensity,
                                       const Pixel& fgndColor,
                                       const PixelChannelType newAlpha) -> Pixel
{
  constexpr auto BRIGHTNESS_INCREASE = 2.0F;
  constexpr auto BLACK_THRESHOLD     = 30U;

  if (COLOR::IsCloseToBlack(fgndColor, BLACK_THRESHOLD))
  {
    return COLOR::GetBrighterColor(BRIGHTNESS_INCREASE, bgndColor);
  }

  if (COLOR::IsCloseToBlack(bgndColor, BLACK_THRESHOLD))
  {
    return COLOR::GetBrighterColor(BRIGHTNESS_INCREASE, fgndColor);
  }

  return GetColorMin(
      bgndColor, COLOR::GetBrighterColorInt(fgndIntBuffIntensity, fgndColor), newAlpha);
}

constexpr auto GetLightenOnlyPixelBlend(const Pixel& bgndColor,
                                        const uint32_t fgndIntBuffIntensity,
                                        const Pixel& fgndColor,
                                        const PixelChannelType newAlpha) -> Pixel
{
  return GetColorMax(
      bgndColor, COLOR::GetBrighterColorInt(fgndIntBuffIntensity, fgndColor), newAlpha);
}

constexpr auto GetSameLumaMixPixelBlend(const float lumaMixT,
                                        const Pixel& bgndColor,
                                        const uint32_t fgndIntBuffIntensity,
                                        const Pixel& fgndColor,
                                        const PixelChannelType newAlpha) -> Pixel
{
  constexpr auto SMALL_LUMA = 0.1F;

  const auto fgndColorLuma = COLOR::GetLuma(fgndColor);
  if (fgndColorLuma < SMALL_LUMA)
  {
    return GetColorAdd(bgndColor, fgndColor, newAlpha);
  }

  const auto bgndColorLuma = COLOR::GetLuma(bgndColor);
  if (bgndColorLuma < SMALL_LUMA)
  {
    return GetColorAdd(bgndColor, fgndColor, newAlpha);
  }

  const auto buffIntensity =
      static_cast<float>(fgndIntBuffIntensity) / channel_limits<float>::max();

  const auto brightness = std::min((bgndColorLuma + (buffIntensity * fgndColorLuma)) /
                                       STD20::lerp(bgndColorLuma, fgndColorLuma, lumaMixT),
                                   COLOR::MAX_BRIGHTNESS);

  const auto finalFgndColor = COLOR::ColorMaps::GetColorMix(bgndColor, fgndColor, lumaMixT);
  return COLOR::GetBrighterColor(brightness, finalFgndColor);
}

constexpr auto GetColorAlphaBlend(const Pixel& bgndColor,
                                  const Pixel& fgndColor,
                                  const PixelChannelType newAlpha) -> Pixel
{
  const auto fgndR = static_cast<int32_t>(fgndColor.R());
  const auto fgndG = static_cast<int32_t>(fgndColor.G());
  const auto fgndB = static_cast<int32_t>(fgndColor.B());
  const auto fgndA = static_cast<int32_t>(fgndColor.A());
  const auto bgndR = static_cast<int32_t>(bgndColor.R());
  const auto bgndG = static_cast<int32_t>(bgndColor.G());
  const auto bgndB = static_cast<int32_t>(bgndColor.B());

  const auto newR = static_cast<PixelChannelType>(bgndR + ((fgndA * (fgndR - bgndR)) / MAX_ALPHA));
  const auto newG = static_cast<PixelChannelType>(bgndG + ((fgndA * (fgndG - bgndG)) / MAX_ALPHA));
  const auto newB = static_cast<PixelChannelType>(bgndB + ((fgndA * (fgndB - bgndB)) / MAX_ALPHA));

  return Pixel{newR, newG, newB, newAlpha};
}

constexpr auto GetColorAlphaAndAddBlend(const Pixel& bgndColor,
                                        const uint32_t fgndIntBuffIntensity,
                                        const Pixel& fgndColor,
                                        const PixelChannelType newAlpha) -> Pixel
{
  if (fgndColor.A() == MAX_ALPHA)
  {
    return GetColorAddPixelBlend(bgndColor, fgndIntBuffIntensity, fgndColor, newAlpha);
  }

  return GetColorAddPixelBlend(bgndColor,
                               fgndIntBuffIntensity,
                               GetColorAlphaBlend(bgndColor, fgndColor, newAlpha),
                               newAlpha);
}

} // namespace GOOM::UTILS::GRAPHICS
