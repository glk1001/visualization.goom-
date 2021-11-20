#pragma once

#include "../utils/mathutils.h"
#include "goom/goom_graphic.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace COLOR
{
#else
namespace GOOM::COLOR
{
#endif

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
[[nodiscard]] auto GetRgbColorLerp(const Pixel& colA, const Pixel& colB, float t) -> Pixel;

[[nodiscard]] auto GetLuma(const Pixel& color) -> float;
constexpr float INCREASED_CHROMA_FACTOR = 2.0F;
constexpr float DECREASED_CHROMA_FACTOR = 0.5F;
[[nodiscard]] auto GetAlteredChroma(float lchYFactor, const Pixel& color) -> Pixel;
[[nodiscard]] auto GetIncreasedChroma(const Pixel& color) -> Pixel;
[[nodiscard]] auto GetDecreasedChroma(const Pixel& color) -> Pixel;

class GammaCorrection
{
public:
  GammaCorrection(float gamma, float threshold);

  [[nodiscard]] auto GetThreshold() const -> float;
  void SetThreshold(float val);
  [[nodiscard]] auto GetGamma() const -> float;
  void SetGamma(float val);

  [[nodiscard]] auto GetCorrection(float brightness, const Pixel& color) const -> Pixel;

private:
  float m_gamma;
  float m_threshold;
};


inline auto ColorChannelMultiply(const PixelChannelType ch1, const PixelChannelType ch2) -> uint32_t
{
  return MultiplyColorChannels(ch1, ch2);
}

inline auto ColorChannelAdd(const PixelChannelType ch1, const PixelChannelType ch2) -> uint32_t
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


inline auto GetBrighterChannelColor(const uint32_t brightness, const PixelChannelType channelVal)
    -> uint32_t
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
  assert(brightness >= 0.0F && brightness <= 50.0F);
  const auto intBrightness = static_cast<uint32_t>(std::round((brightness * 256.0F) + 0.0001F));
  return GetBrighterColorInt(intBrightness, color);
}


inline auto GetRgbColorChannelLerp(const int32_t ch1, const int32_t ch2, const int32_t intT)
    -> uint32_t
{
  constexpr auto MAX_COL_VAL_32 = static_cast<int32_t>(MAX_COLOR_VAL);
  return static_cast<uint32_t>(((MAX_COL_VAL_32 * ch1) + (intT * (ch2 - ch1))) / MAX_COL_VAL_32);
}

inline auto GetRgbColorLerp(const Pixel& colA, const Pixel& colB, float t) -> Pixel
{
  t = stdnew::clamp(t, 0.0F, 1.0F);
  const auto intT = static_cast<int32_t>(t * static_cast<float>(MAX_COLOR_VAL));

  const auto colA_R = static_cast<int32_t>(colA.R());
  const auto colA_G = static_cast<int32_t>(colA.G());
  const auto colA_B = static_cast<int32_t>(colA.B());
  const auto colA_A = static_cast<int32_t>(colA.A());
  const auto colB_R = static_cast<int32_t>(colB.R());
  const auto colB_G = static_cast<int32_t>(colB.G());
  const auto colB_B = static_cast<int32_t>(colB.B());
  const auto colB_A = static_cast<int32_t>(colB.A());

  const uint32_t newR = GetRgbColorChannelLerp(colA_R, colB_R, intT);
  const uint32_t newG = GetRgbColorChannelLerp(colA_G, colB_G, intT);
  const uint32_t newB = GetRgbColorChannelLerp(colA_B, colB_B, intT);
  const uint32_t newA = GetRgbColorChannelLerp(colA_A, colB_A, intT);

  return Pixel{newR, newG, newB, newA};
}


// RGB -> Luma conversion formula.
//
// Photometric/digital ITU BT.709:
//
//     Y = 0.2126 R + 0.7152 G + 0.0722 B
//
constexpr float LUMA_RED_COMPONENT = 0.2126F;
constexpr float LUMA_GREEN_COMPONENT = 0.7152F;
constexpr float LUMA_BLUE_COMPONENT = 0.0722F;

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

inline GammaCorrection::GammaCorrection(const float gamma, const float threshold)
  : m_gamma{gamma}, m_threshold{threshold}
{
}

inline auto GammaCorrection::GetThreshold() const -> float
{
  return m_threshold;
}

inline void GammaCorrection::SetThreshold(const float val)
{
  m_threshold = val;
}

inline auto GammaCorrection::GetGamma() const -> float
{
  return m_gamma;
}

inline void GammaCorrection::SetGamma(const float val)
{
  m_gamma = val;
}

#if __cplusplus <= 201402L
} // namespace COLOR
} // namespace GOOM
#else
} // namespace GOOM::COLOR
#endif

