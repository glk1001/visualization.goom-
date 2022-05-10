#pragma once

#include "goom/goom_graphic.h"
#include "utils/math/misc.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>

namespace GOOM::COLOR
{

[[nodiscard]] auto GetColorAverage(const Pixel& color1, const Pixel& color2) -> Pixel;
template<typename T>
[[nodiscard]] auto GetColorAverage(size_t num, const T& colors) -> Pixel;

[[nodiscard]] auto GetColorBlend(const Pixel& fgnd, const Pixel& bgnd) -> Pixel;
[[nodiscard]] auto GetColorMultiply(const Pixel& srce, const Pixel& dest) -> Pixel;
[[nodiscard]] auto GetColorAdd(const Pixel& color1, const Pixel& color2) -> Pixel;
[[nodiscard]] auto GetBrighterColorInt(uint32_t brightness, const Pixel& color) -> Pixel;
[[nodiscard]] auto GetBrighterColorInt(float brightness, const Pixel&) -> Pixel = delete;

[[nodiscard]] auto GetBrighterColor(float brightness, const Pixel& color) -> Pixel;
[[nodiscard]] auto GetBrighterColor(uint32_t brightness, const Pixel&) -> Pixel = delete;

[[nodiscard]] auto GetLightenedColor(const Pixel& oldColor, float power) -> Pixel;
[[nodiscard]] auto GetEvolvedColor(const Pixel& baseColor) -> Pixel;

[[nodiscard]] auto GetRgbColorChannelLerp(int32_t ch1, int32_t ch2, int32_t intT) -> uint32_t;
[[nodiscard]] auto GetRgbColorLerp(const Pixel& color1, const Pixel& color2, float t) -> Pixel;

[[nodiscard]] auto GetLuma(const Pixel& color) -> float;
static constexpr float INCREASED_CHROMA_FACTOR = 2.0F;
static constexpr float DECREASED_CHROMA_FACTOR = 0.5F;
[[nodiscard]] auto GetAlteredChroma(float lchYFactor, const Pixel& color) -> Pixel;
[[nodiscard]] auto GetIncreasedChroma(const Pixel& color) -> Pixel;
[[nodiscard]] auto GetDecreasedChroma(const Pixel& color) -> Pixel;

[[nodiscard]] auto IsCloseToBlack(const Pixel& color, uint32_t threshold = 10) -> bool;

enum class SimpleColors
{
  BLEUBLANC = 0,
  RED,
  ORANGE_V,
  ORANGE_J,
  VERT,
  BLEU,
  BLACK,
  _num // unused and must be last
};
[[nodiscard]] auto GetSimpleColor(SimpleColors simpleColor) -> Pixel;

class ColorCorrection
{
public:
  static constexpr float GOOD_INCREASED_CHROMA_FACTOR = INCREASED_CHROMA_FACTOR;

  ColorCorrection(float gamma, float alterChromaFactor = 1.0F);

  [[nodiscard]] auto GetIgnoreThreshold() const -> float;
  auto SetIgnoreThreshold(float val) -> void;

  [[nodiscard]] auto GetGamma() const -> float;
  auto SetGamma(float val) -> void;

  [[nodiscard]] auto GetAlterChromaFactor() const -> float;
  auto SetAlterChromaFactor(float val) -> void;

  [[nodiscard]] auto GetCorrection(float brightness, const Pixel& color) const -> Pixel;

private:
  float m_gamma;
  float m_alterChromaFactor;
  bool m_doAlterChroma = not UTILS::MATH::FloatsEqual(1.0F, m_alterChromaFactor);
  static constexpr float DEFAULT_GAMMA_BRIGHTNESS_THRESHOLD = 0.01F;
  float m_ignoreThreshold = DEFAULT_GAMMA_BRIGHTNESS_THRESHOLD;
};


[[nodiscard]] inline auto ColorChannelMultiply(const PixelChannelType ch1,
                                               const PixelChannelType ch2) -> uint32_t
{
  return MultiplyColorChannels(ch1, ch2);
}

[[nodiscard]] inline auto ColorChannelAdd(const PixelChannelType ch1, const PixelChannelType ch2)
    -> uint32_t
{
  return static_cast<uint32_t>(ch1) + static_cast<uint32_t>(ch2);
}

template<typename T>
inline auto GetColorAverage(const size_t num, const T& colors) -> Pixel
{
  assert(num > 0);

  uint32_t newR = 0;
  uint32_t newG = 0;
  uint32_t newB = 0;
  uint32_t newA = 0;

  for (size_t i = 0; i < num; ++i)
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

inline auto GetColorAverage(const Pixel& color1, const Pixel& color2) -> Pixel
{
  const uint32_t newR = ColorChannelAdd(color1.R(), color2.R()) / 2;
  const uint32_t newG = ColorChannelAdd(color1.G(), color2.G()) / 2;
  const uint32_t newB = ColorChannelAdd(color1.B(), color2.B()) / 2;
  const uint32_t newA = ColorChannelAdd(color1.A(), color2.A()) / 2;

  return Pixel{newR, newG, newB, newA};
}

inline auto GetColorBlend(const Pixel& fgnd, const Pixel& bgnd) -> Pixel
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

inline auto GetColorMultiply(const Pixel& srce, const Pixel& dest) -> Pixel
{
  const uint32_t newR = ColorChannelMultiply(srce.R(), dest.R());
  const uint32_t newG = ColorChannelMultiply(srce.G(), dest.G());
  const uint32_t newB = ColorChannelMultiply(srce.B(), dest.B());
  const uint32_t newA = ColorChannelMultiply(srce.A(), dest.A());

  return Pixel{newR, newG, newB, newA};
}

inline auto GetColorAdd(const Pixel& color1, const Pixel& color2) -> Pixel
{
  const uint32_t newR = ColorChannelAdd(color1.R(), color2.R());
  const uint32_t newG = ColorChannelAdd(color1.G(), color2.G());
  const uint32_t newB = ColorChannelAdd(color1.B(), color2.B());
  const uint32_t newA = ColorChannelAdd(color1.A(), color2.A());

  return Pixel{newR, newG, newB, newA};
}


[[nodiscard]] inline auto GetBrighterChannelColor(const uint32_t brightness,
                                                  const PixelChannelType channelVal) -> uint32_t
{
  return MultiplyChannelColorByScalar(brightness, channelVal);
}

inline auto GetBrighterColorInt(const uint32_t brightness, const Pixel& color) -> Pixel
{
  const uint32_t newR = GetBrighterChannelColor(brightness, color.R());
  const uint32_t newG = GetBrighterChannelColor(brightness, color.G());
  const uint32_t newB = GetBrighterChannelColor(brightness, color.B());
  const uint32_t newA = color.A();

  return Pixel{newR, newG, newB, newA};
}

inline auto GetBrighterColor(const float brightness, const Pixel& color) -> Pixel
{
  static constexpr float MAX_BRIGHTNESS = 50.0F;
  assert(brightness >= 0.0F && brightness <= MAX_BRIGHTNESS);
  UNUSED_FOR_NDEBUG(MAX_BRIGHTNESS);

  const auto intBrightness = static_cast<uint32_t>(std::round((brightness * 256.0F) + 0.0001F));
  return GetBrighterColorInt(intBrightness, color);
}

inline auto GetRgbColorChannelLerp(const int32_t ch1, const int32_t ch2, const int32_t intT)
    -> uint32_t
{
  static constexpr auto MAX_COL_VAL_32 = static_cast<int32_t>(MAX_COLOR_VAL);
  return static_cast<uint32_t>(((MAX_COL_VAL_32 * ch1) + (intT * (ch2 - ch1))) / MAX_COL_VAL_32);
}

inline auto GetRgbColorLerp(const Pixel& color1, const Pixel& color2, float t) -> Pixel
{
  t = std::clamp(t, 0.0F, 1.0F);
  const auto intT = static_cast<int32_t>(t * static_cast<float>(MAX_COLOR_VAL));

  const auto color1Red = static_cast<int32_t>(color1.R());
  const auto color1Green = static_cast<int32_t>(color1.G());
  const auto color1Blue = static_cast<int32_t>(color1.B());
  const auto color1Alpha = static_cast<int32_t>(color1.A());
  const auto color2Red = static_cast<int32_t>(color2.R());
  const auto color2Green = static_cast<int32_t>(color2.G());
  const auto color2Blue = static_cast<int32_t>(color2.B());
  const auto color2Alpha = static_cast<int32_t>(color2.A());

  const uint32_t newR = GetRgbColorChannelLerp(color1Red, color2Red, intT);
  const uint32_t newG = GetRgbColorChannelLerp(color1Green, color2Green, intT);
  const uint32_t newB = GetRgbColorChannelLerp(color1Blue, color2Blue, intT);
  const uint32_t newA = GetRgbColorChannelLerp(color1Alpha, color2Alpha, intT);

  return Pixel{newR, newG, newB, newA};
}

inline auto IsCloseToBlack(const Pixel& color, const uint32_t threshold) -> bool
{
  return (color.R() < threshold) && (color.G() < threshold) && (color.B() < threshold);
}


// RGB -> Luma conversion formula.
//
// Photometric/digital ITU BT.709:
//
//     Y = 0.2126 R + 0.7152 G + 0.0722 B
//
static constexpr float LUMA_RED_COMPONENT = 0.2126F;
static constexpr float LUMA_GREEN_COMPONENT = 0.7152F;
static constexpr float LUMA_BLUE_COMPONENT = 0.0722F;

inline auto GetLuma(const Pixel& color) -> float
{
  return (LUMA_RED_COMPONENT * color.RFlt()) + (LUMA_GREEN_COMPONENT * color.GFlt()) +
         (LUMA_BLUE_COMPONENT * color.BFlt());
}

inline auto GetIncreasedChroma(const Pixel& color) -> Pixel
{
  return GetAlteredChroma(INCREASED_CHROMA_FACTOR, color);
}

inline auto GetDecreasedChroma(const Pixel& color) -> Pixel
{
  return GetAlteredChroma(DECREASED_CHROMA_FACTOR, color);
}

inline auto GetSimpleColor(const SimpleColors simpleColor) -> Pixel
{
  static constexpr Pixel::RGB RED{230, 120, 18, MAX_ALPHA};
  static constexpr Pixel::RGB ORANGE_J{120, 252, 18, MAX_ALPHA};
  static constexpr Pixel::RGB ORANGE_V{160, 236, 40, MAX_ALPHA};
  static constexpr Pixel::RGB BLEUBLANC{40, 220, 140, MAX_ALPHA};
  static constexpr Pixel::RGB VERT{200, 80, 18, MAX_ALPHA};
  static constexpr Pixel::RGB BLEU{250, 30, 80, MAX_ALPHA};
  static constexpr Pixel::RGB BLACK{16, 16, 16, MAX_ALPHA};

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
      throw std::logic_error("Unknown simple color enum.");
  }
}

inline ColorCorrection::ColorCorrection(const float gamma, const float alterChromaFactor)
  : m_gamma{gamma}, m_alterChromaFactor{alterChromaFactor}
{
}

inline auto ColorCorrection::GetGamma() const -> float
{
  return m_gamma;
}

inline auto ColorCorrection::SetGamma(const float val) -> void
{
  m_gamma = val;
}

inline auto ColorCorrection::GetAlterChromaFactor() const -> float
{
  return m_alterChromaFactor;
}

inline auto ColorCorrection::SetAlterChromaFactor(const float val) -> void
{
  m_alterChromaFactor = val;
  m_doAlterChroma = not UTILS::MATH::FloatsEqual(1.0F, m_alterChromaFactor);
}

inline auto ColorCorrection::GetIgnoreThreshold() const -> float
{
  return m_ignoreThreshold;
}

inline auto ColorCorrection::SetIgnoreThreshold(const float val) -> void
{
  m_ignoreThreshold = val;
}

} // namespace GOOM::COLOR
