#include "colorutils.h"

#include "goom/goom_graphic.h"

#include <cmath>
#include <cstdint>
#include <vivid/vivid.h>

namespace GOOM::COLOR
{

auto GammaCorrection::GetCorrection(const float brightness, const Pixel& color) const -> Pixel
{
  if (brightness < m_threshold)
  {
    return GetBrighterColor(brightness, color);
  }
  // OLD  return GetBrighterColor(std::pow(brightness, m_gamma), color);

  auto newR = static_cast<uint32_t>(
      std::round(channel_limits<float>::max() * std::pow(brightness * color.RFlt(), m_gamma)));
  auto newG = static_cast<uint32_t>(
      std::round(channel_limits<float>::max() * std::pow(brightness * color.GFlt(), m_gamma)));
  auto newB = static_cast<uint32_t>(
      std::round(channel_limits<float>::max() * std::pow(brightness * color.BFlt(), m_gamma)));
  const uint32_t newA = color.A();

  return Pixel{newR, newG, newB, newA};
}

auto GetAlteredChroma(const float lchYFactor, const Pixel& color) -> Pixel
{
  const vivid::col8_t rgb8 = {color.R(), color.G(), color.B()};
  vivid::lch_t lch = vivid::lch::fromSrgb(vivid::rgb::fromRgb8(rgb8));
  constexpr float MAX_LCH_Y = 140.0F;
  lch.y = std::min(lch.y * lchYFactor, MAX_LCH_Y);
  const vivid::col8_t newRgb8 = vivid::rgb8::fromRgb(vivid::srgb::fromLch(lch));
  return Pixel{{newRgb8.r, newRgb8.g, newRgb8.b, MAX_ALPHA}};
}

inline auto Lighten(const PixelChannelType value, const float power) -> PixelChannelType
{
  const auto t = (static_cast<float>(value) * std::log10(power)) / 2.0F;
  if (t <= 0.0F)
  {
    return 0;
  }

  // (32.0f * log (t));
  return static_cast<PixelChannelType>(
      std::clamp(static_cast<int>(t), channel_limits<int>::min(), channel_limits<int>::max()));
}

auto GetLightenedColor(const Pixel& oldColor, const float power) -> Pixel
{
  Pixel pixel = oldColor;

  pixel.SetR(Lighten(pixel.R(), power));
  pixel.SetG(Lighten(pixel.G(), power));
  pixel.SetB(Lighten(pixel.B(), power));

  return pixel;
}

inline auto EvolvedColor(const Pixel& src,
                         const Pixel& dest,
                         const PixelIntType mask,
                         const PixelIntType incr) -> Pixel
{
  struct RGBChannels
  {
    PixelChannelType r = 0;
    PixelChannelType g = 0;
    PixelChannelType b = 0;
    PixelChannelType a = 0;
  };
  union RGBColor
  {
    RGBChannels channels{};
    PixelIntType intVal;
  };

  const RGBColor srcColor{{src.R(), src.G(), src.B(), src.A()}};
  PixelIntType iMaskedSrc = srcColor.intVal & mask;

  const RGBColor destColor{{dest.R(), dest.G(), dest.B(), dest.A()}};
  const PixelIntType iMaskedDest = destColor.intVal & mask;

  if ((iMaskedSrc != mask) && (iMaskedSrc < iMaskedDest))
  {
    iMaskedSrc += incr;
  }
  if (iMaskedSrc > iMaskedDest)
  {
    iMaskedSrc -= incr;
  }

  const PixelIntType color = srcColor.intVal & (~mask);

  RGBColor finalColor;
  finalColor.intVal = (iMaskedSrc & mask) | static_cast<PixelIntType>(color);

  return Pixel{
      {finalColor.channels.r, finalColor.channels.g, finalColor.channels.b, finalColor.channels.a}};
}

auto GetEvolvedColor(const Pixel& baseColor) -> Pixel
{
  Pixel newColor = baseColor;

  newColor = EvolvedColor(newColor, baseColor, 0xFFU, 0x01U);
  newColor = EvolvedColor(newColor, baseColor, 0xFF00U, 0x0100U);
  newColor = EvolvedColor(newColor, baseColor, 0xFF0000U, 0x010000U);
  newColor = EvolvedColor(newColor, baseColor, 0xFF000000U, 0x01000000U);

  constexpr float LIGHTENED_COLOR_POWER = (10.0F * 2.0F) + 2.0F;
  newColor = GetLightenedColor(newColor, LIGHTENED_COLOR_POWER);

  return newColor;
}

} // namespace GOOM::COLOR
