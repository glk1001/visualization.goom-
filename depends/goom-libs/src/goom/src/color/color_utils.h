#pragma once

#include "goom/goom_graphic.h"
#include "goom_config.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace GOOM::COLOR
{

[[nodiscard]] constexpr auto GetColorAverage(const Pixel& color1, const Pixel& color2) -> Pixel;
template<typename T>
[[nodiscard]] constexpr auto GetColorAverage(size_t num, const T& colors) -> Pixel;

[[nodiscard]] constexpr auto GetColorBlend(const Pixel& fgnd, const Pixel& bgnd) -> Pixel;
[[nodiscard]] constexpr auto ColorChannelMultiply(PixelChannelType ch1, PixelChannelType ch2)
    -> uint32_t;
[[nodiscard]] constexpr auto GetColorMultiply(const Pixel& srce, const Pixel& dest) -> Pixel;
[[nodiscard]] constexpr auto ColorChannelAdd(PixelChannelType ch1, PixelChannelType ch2)
    -> uint32_t;
[[nodiscard]] constexpr auto GetColorAdd(const Pixel& color1, const Pixel& color2) -> Pixel;

[[nodiscard]] constexpr auto GetBrighterColorInt(uint32_t brightness, const Pixel& color) -> Pixel;
[[nodiscard, maybe_unused]] constexpr auto GetBrighterColorInt(float brightness, const Pixel&)
    -> Pixel = delete;
[[nodiscard]] constexpr auto GetBrighterChannelColor(uint32_t brightness,
                                                     PixelChannelType channelVal) -> uint32_t;
[[nodiscard]] auto GetBrighterColor(float brightness, const Pixel& color) -> Pixel;
[[nodiscard]] auto GetBrighterColor(uint32_t brightness, const Pixel&) -> Pixel = delete;

[[nodiscard]] constexpr auto GetRgbColorChannelLerp(int32_t ch1, int32_t ch2, int32_t intT)
    -> uint32_t;
[[nodiscard]] constexpr auto GetRgbColorLerp(const Pixel& color1, const Pixel& color2, float t)
    -> Pixel;

[[nodiscard]] constexpr auto GetLuma(const Pixel& color) -> float;

[[nodiscard]] constexpr auto IsCloseToBlack(const Pixel& color, uint32_t threshold = 10) -> bool;

[[nodiscard]] auto GetLightenedColor(const Pixel& oldColor, float power) -> Pixel;
[[nodiscard]] auto GetEvolvedColor(const Pixel& baseColor) -> Pixel;

enum class SimpleColors
{
  BLEUBLANC = 0,
  RED,
  ORANGE_V,
  ORANGE_J,
  VERT,
  BLEU,
  BLACK,
  _num // unused, and marks the enum end
};
[[nodiscard]] constexpr auto GetSimpleColor(SimpleColors simpleColor) -> Pixel;

template<typename T>
constexpr auto GetColorAverage(const size_t num, const T& colors) -> Pixel
{
  Expects(num > 0);

  auto newR = 0U;
  auto newG = 0U;
  auto newB = 0U;
  auto newA = 0U;

  for (auto i = 0U; i < num; ++i)
  {
    newR += static_cast<uint32_t>(colors[i].R());
    newG += static_cast<uint32_t>(colors[i].G());
    newB += static_cast<uint32_t>(colors[i].B());
    newA += static_cast<uint32_t>(colors[i].A());
  }

  newR /= static_cast<uint32_t>(num);
  newG /= static_cast<uint32_t>(num);
  newB /= static_cast<uint32_t>(num);
  newA /= static_cast<uint32_t>(num);

  return Pixel{newR, newG, newB, newA};
}

constexpr auto GetColorAverage(const Pixel& color1, const Pixel& color2) -> Pixel
{
  const auto newR = ColorChannelAdd(color1.R(), color2.R()) / 2;
  const auto newG = ColorChannelAdd(color1.G(), color2.G()) / 2;
  const auto newB = ColorChannelAdd(color1.B(), color2.B()) / 2;
  const auto newA = ColorChannelAdd(color1.A(), color2.A()) / 2;

  return Pixel{newR, newG, newB, newA};
}

constexpr auto GetColorBlend(const Pixel& fgnd, const Pixel& bgnd) -> Pixel
{
  const auto fgndR = static_cast<int>(fgnd.R());
  const auto fgndG = static_cast<int>(fgnd.G());
  const auto fgndB = static_cast<int>(fgnd.B());
  const auto fgndA = static_cast<int>(fgnd.A());
  const auto bgndR = static_cast<int>(bgnd.R());
  const auto bgndG = static_cast<int>(bgnd.G());
  const auto bgndB = static_cast<int>(bgnd.B());
  const auto bgndA = static_cast<int>(bgnd.A());

  const auto newR =
      static_cast<uint32_t>(bgndR + ((fgndA * (fgndR - bgndR)) / channel_limits<int32_t>::max()));
  const auto newG =
      static_cast<uint32_t>(bgndG + ((fgndA * (fgndG - bgndG)) / channel_limits<int32_t>::max()));
  const auto newB =
      static_cast<uint32_t>(bgndB + ((fgndA * (fgndB - bgndB)) / channel_limits<int32_t>::max()));
  const auto newA = static_cast<uint32_t>(std::min(channel_limits<int32_t>::max(), fgndA + bgndA));

  return Pixel{newR, newG, newB, newA};
}

constexpr auto GetColorMultiply(const Pixel& srce, const Pixel& dest) -> Pixel
{
  const auto newR = ColorChannelMultiply(srce.R(), dest.R());
  const auto newG = ColorChannelMultiply(srce.G(), dest.G());
  const auto newB = ColorChannelMultiply(srce.B(), dest.B());
  const auto newA = ColorChannelMultiply(srce.A(), dest.A());

  return Pixel{newR, newG, newB, newA};
}

constexpr auto ColorChannelMultiply(const PixelChannelType ch1, const PixelChannelType ch2)
    -> uint32_t
{
  return MultiplyColorChannels(ch1, ch2);
}

constexpr auto GetColorAdd(const Pixel& color1, const Pixel& color2) -> Pixel
{
  const auto newR = ColorChannelAdd(color1.R(), color2.R());
  const auto newG = ColorChannelAdd(color1.G(), color2.G());
  const auto newB = ColorChannelAdd(color1.B(), color2.B());
  const auto newA = ColorChannelAdd(color1.A(), color2.A());

  return Pixel{newR, newG, newB, newA};
}

constexpr auto ColorChannelAdd(const PixelChannelType ch1, const PixelChannelType ch2) -> uint32_t
{
  return static_cast<uint32_t>(ch1) + static_cast<uint32_t>(ch2);
}

constexpr auto GetBrighterColorInt(const uint32_t brightness, const Pixel& color) -> Pixel
{
  const auto newR = GetBrighterChannelColor(brightness, color.R());
  const auto newG = GetBrighterChannelColor(brightness, color.G());
  const auto newB = GetBrighterChannelColor(brightness, color.B());
  const auto newA = color.A();

  return Pixel{newR, newG, newB, newA};
}

constexpr auto GetBrighterChannelColor(const uint32_t brightness, const PixelChannelType channelVal)
    -> uint32_t
{
  return MultiplyChannelColorByScalar(brightness, channelVal);
}

inline auto GetBrighterColor(const float brightness, const Pixel& color) -> Pixel
{
  static constexpr auto MAX_BRIGHTNESS = 50.0F;
  Expects((brightness >= 0.0F) && (brightness <= MAX_BRIGHTNESS));
  UNUSED_FOR_NDEBUG(MAX_BRIGHTNESS);

  const auto intBrightness = static_cast<uint32_t>(std::round((brightness * 256.0F) + 0.0001F));
  return GetBrighterColorInt(intBrightness, color);
}

constexpr auto GetRgbColorChannelLerp(const int32_t ch1, const int32_t ch2, const int32_t intT)
    -> uint32_t
{
  constexpr auto MAX_COL_VAL_32 = static_cast<int32_t>(MAX_COLOR_VAL);
  return static_cast<uint32_t>(((MAX_COL_VAL_32 * ch1) + (intT * (ch2 - ch1))) / MAX_COL_VAL_32);
}

constexpr auto GetRgbColorLerp(const Pixel& color1, const Pixel& color2, float t) -> Pixel
{
  t               = std::clamp(t, 0.0F, 1.0F);
  const auto intT = static_cast<int32_t>(t * static_cast<float>(MAX_COLOR_VAL));

  const auto color1Red   = static_cast<int32_t>(color1.R());
  const auto color1Green = static_cast<int32_t>(color1.G());
  const auto color1Blue  = static_cast<int32_t>(color1.B());
  const auto color1Alpha = static_cast<int32_t>(color1.A());
  const auto color2Red   = static_cast<int32_t>(color2.R());
  const auto color2Green = static_cast<int32_t>(color2.G());
  const auto color2Blue  = static_cast<int32_t>(color2.B());
  const auto color2Alpha = static_cast<int32_t>(color2.A());

  const auto newR = GetRgbColorChannelLerp(color1Red, color2Red, intT);
  const auto newG = GetRgbColorChannelLerp(color1Green, color2Green, intT);
  const auto newB = GetRgbColorChannelLerp(color1Blue, color2Blue, intT);
  const auto newA = GetRgbColorChannelLerp(color1Alpha, color2Alpha, intT);

  return Pixel{newR, newG, newB, newA};
}

constexpr auto IsCloseToBlack(const Pixel& color, const uint32_t threshold) -> bool
{
  return (color.R() < threshold) && (color.G() < threshold) && (color.B() < threshold);
}


// RGB -> Luma conversion formula.
//
// Photometric/digital ITU BT.709:
//
//     Y = 0.2126 R + 0.7152 G + 0.0722 B
//
static constexpr auto LUMA_RED_COMPONENT   = 0.2126F;
static constexpr auto LUMA_GREEN_COMPONENT = 0.7152F;
static constexpr auto LUMA_BLUE_COMPONENT  = 0.0722F;

constexpr auto GetLuma(const Pixel& color) -> float
{
  return (LUMA_RED_COMPONENT * color.RFlt()) + (LUMA_GREEN_COMPONENT * color.GFlt()) +
         (LUMA_BLUE_COMPONENT * color.BFlt());
}

constexpr auto GetSimpleColor(const SimpleColors simpleColor) -> Pixel
{
  constexpr auto RED       = Pixel::RGB{230, 120, 18, MAX_ALPHA};
  constexpr auto ORANGE_J  = Pixel::RGB{120, 252, 18, MAX_ALPHA};
  constexpr auto ORANGE_V  = Pixel::RGB{160, 236, 40, MAX_ALPHA};
  constexpr auto BLEUBLANC = Pixel::RGB{40, 220, 140, MAX_ALPHA};
  constexpr auto VERT      = Pixel::RGB{200, 80, 18, MAX_ALPHA};
  constexpr auto BLEU      = Pixel::RGB{250, 30, 80, MAX_ALPHA};
  constexpr auto BLACK     = Pixel::RGB{16, 16, 16, MAX_ALPHA};

  switch (simpleColor)
  {
    case SimpleColors::RED:
      return Pixel{RED};
    case SimpleColors::ORANGE_J:
      return Pixel{ORANGE_J};
    case SimpleColors::ORANGE_V:
      return Pixel{ORANGE_V};
    case SimpleColors::BLEUBLANC:
      return Pixel{BLEUBLANC};
    case SimpleColors::VERT:
      return Pixel{VERT};
    case SimpleColors::BLEU:
      return Pixel{BLEU};
    case SimpleColors::BLACK:
      return Pixel{BLACK};
    default:
      FailFast();
      return Pixel{BLACK};
  }
}

} // namespace GOOM::COLOR
