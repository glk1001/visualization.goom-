module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>

export module Goom.Color.ColorUtils;

import Goom.Utils.EnumUtils;
import Goom.Utils.Graphics.PixelUtils;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;

export namespace GOOM::COLOR
{

[[nodiscard]] constexpr auto GetColorAverage(const Pixel& color1, const Pixel& color2) -> Pixel;
template<typename T>
[[nodiscard]] constexpr auto GetColorAverage(size_t num, const T& colors) -> Pixel;

[[nodiscard]] constexpr auto GetBrighterColorInt(uint32_t brightness, const Pixel& color) -> Pixel;
[[nodiscard, maybe_unused]] constexpr auto GetBrighterColorInt(float brightness,
                                                               const Pixel&) -> Pixel = delete;
[[nodiscard]] constexpr auto GetBrighterChannelColor(uint32_t brightness,
                                                     PixelChannelType channelVal) -> uint32_t;
inline constexpr auto MIN_BRIGHTNESS = 0.0F;
inline constexpr auto MAX_BRIGHTNESS = 50.0F;
[[nodiscard]] auto GetBrighterColor(float brightness, const Pixel& color) -> Pixel;
[[nodiscard]] auto GetBrighterColor(uint32_t brightness, const Pixel&) -> Pixel = delete;

[[nodiscard]] constexpr auto GetRgbColorChannelLerp(int32_t ch1,
                                                    int32_t ch2,
                                                    int32_t intT) -> PixelChannelType;
[[nodiscard]] inline auto GetRgbColorLerp(const Pixel& color1,
                                          const Pixel& color2,
                                          float t) -> Pixel;

[[nodiscard]] constexpr auto GetLuma(const Pixel& color) -> float;

[[nodiscard]] constexpr auto IsCloseToBlack(const Pixel& color, uint32_t threshold = 10) -> bool;

[[nodiscard]] auto GetLightenedColor(const Pixel& oldColor, float power) -> Pixel;

enum class SimpleColors : UnderlyingEnumType
{
  PURE_RED,
  PURE_LIME,
  PURE_BLUE,
  PURE_YELLOW,
  PURE_AQUA,
  PURE_MAGENTA,
  TORCH_RED,
  ORANGE,
  PIZAZZ,
  TANGO,
  TIA_MARIA,
  SHAMROCK,
  CURIOUS_BLUE,
  LIGHT_ORCHID,
  GOLDEN_BELL,
  BLACK,
};
[[nodiscard]] constexpr auto GetSimpleColor(SimpleColors simpleColor,
                                            PixelChannelType alpha) -> Pixel;

} // namespace GOOM::COLOR

namespace GOOM::COLOR
{

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
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-constant-array-index)
    newR += static_cast<uint32_t>(colors[i].R());
    newG += static_cast<uint32_t>(colors[i].G());
    newB += static_cast<uint32_t>(colors[i].B());
    newA += static_cast<uint32_t>(colors[i].A());
    // NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)
  }

  // NOLINTBEGIN(clang-analyzer-core.DivideZero)
  newR /= static_cast<uint32_t>(num);
  newG /= static_cast<uint32_t>(num);
  newB /= static_cast<uint32_t>(num);
  newA /= static_cast<uint32_t>(num);
  // NOLINTEND(clang-analyzer-core.DivideZero)

  return Pixel{static_cast<PixelChannelType>(newR),
               static_cast<PixelChannelType>(newG),
               static_cast<PixelChannelType>(newB),
               static_cast<PixelChannelType>(newA)};
}

constexpr auto GetColorAverage(const Pixel& color1, const Pixel& color2) -> Pixel
{
  using UTILS::GRAPHICS::GetColorChannelAdd;
  const auto newR = static_cast<PixelChannelType>(GetColorChannelAdd(color1.R(), color2.R()) / 2);
  const auto newG = static_cast<PixelChannelType>(GetColorChannelAdd(color1.G(), color2.G()) / 2);
  const auto newB = static_cast<PixelChannelType>(GetColorChannelAdd(color1.B(), color2.B()) / 2);
  const auto newA = static_cast<PixelChannelType>(GetColorChannelAdd(color1.A(), color2.A()) / 2);

  return Pixel{newR, newG, newB, newA};
}

constexpr auto GetBrighterColorInt(const uint32_t brightness, const Pixel& color) -> Pixel
{
  const auto newR = GetBrighterChannelColor(brightness, color.R());
  const auto newG = GetBrighterChannelColor(brightness, color.G());
  const auto newB = GetBrighterChannelColor(brightness, color.B());
  const auto newA = color.A();

  return UTILS::GRAPHICS::MakePixel(newR, newG, newB, newA);
}

constexpr auto GetBrighterChannelColor(const uint32_t brightness,
                                       const PixelChannelType channelVal) -> uint32_t
{
  return UTILS::GRAPHICS::GetChannelColorMultiplyByScalar(brightness, channelVal);
}

inline auto GetBrighterColor(const float brightness, const Pixel& color) -> Pixel
{
  Expects(brightness >= MIN_BRIGHTNESS);
  Expects(brightness <= MAX_BRIGHTNESS);

  static constexpr auto MAX_COLOR_VALUE = 256.0F;
  static constexpr auto SMALL_VALUE     = 0.0001F;
  return GetBrighterColorInt(
      static_cast<uint32_t>(std::round((brightness * MAX_COLOR_VALUE) + SMALL_VALUE)), color);
}

constexpr auto GetRgbColorChannelLerp(const int32_t ch1,
                                      const int32_t ch2,
                                      const int32_t intT) -> PixelChannelType
{
  constexpr auto MAX_COL_VAL_32 = static_cast<int32_t>(MAX_COLOR_VAL);
  const auto lerpVal            = ((MAX_COL_VAL_32 * ch1) + (intT * (ch2 - ch1))) / MAX_COL_VAL_32;
  return static_cast<PixelChannelType>(lerpVal);
}

inline auto GetRgbColorLerp(const Pixel& color1, const Pixel& color2, const float t) -> Pixel
{
  const auto intT = static_cast<int32_t>(
      std::lround(std::clamp(t, 0.0F, 1.0F) * static_cast<float>(MAX_COLOR_VAL)));

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
inline constexpr auto LUMA_RED_COMPONENT   = 0.2126F;
inline constexpr auto LUMA_GREEN_COMPONENT = 0.7152F;
inline constexpr auto LUMA_BLUE_COMPONENT  = 0.0722F;

constexpr auto GetLuma(const Pixel& color) -> float
{
  return (LUMA_RED_COMPONENT * color.RFlt()) + (LUMA_GREEN_COMPONENT * color.GFlt()) +
         (LUMA_BLUE_COMPONENT * color.BFlt());
}

constexpr auto GetSimpleColor(const SimpleColors simpleColor, const PixelChannelType alpha) -> Pixel
{
  constexpr auto SIMPLE_COLORS_MAP = UTILS::EnumMap<SimpleColors, Pixel>{{{
      {SimpleColors::PURE_RED, Pixel{255, 0, 0, MAX_ALPHA}},
      {SimpleColors::PURE_LIME, Pixel{0, 255, 0, MAX_ALPHA}},
      {SimpleColors::PURE_BLUE, Pixel{0, 0, 255, MAX_ALPHA}},
      {SimpleColors::PURE_YELLOW, Pixel{255, 255, 0, MAX_ALPHA}},
      {SimpleColors::PURE_AQUA, Pixel{0, 255, 255, MAX_ALPHA}},
      {SimpleColors::PURE_MAGENTA, Pixel{255, 0, 255, MAX_ALPHA}},
      {SimpleColors::TORCH_RED, Pixel{250, 30, 80, MAX_ALPHA}},
      {SimpleColors::ORANGE, Pixel{255, 165, 0, MAX_ALPHA}},
      {SimpleColors::PIZAZZ, Pixel{255, 140, 0, MAX_ALPHA}},
      {SimpleColors::TANGO, Pixel{230, 120, 18, MAX_ALPHA}},
      {SimpleColors::TIA_MARIA, Pixel{200, 80, 18, MAX_ALPHA}},
      {SimpleColors::SHAMROCK, Pixel{40, 220, 140, MAX_ALPHA}},
      {SimpleColors::CURIOUS_BLUE, Pixel{40, 140, 220, MAX_ALPHA}},
      {SimpleColors::LIGHT_ORCHID, Pixel{220, 140, 220, MAX_ALPHA}},
      {SimpleColors::GOLDEN_BELL, Pixel{220, 140, 20, MAX_ALPHA}},
      {SimpleColors::BLACK, Pixel{16, 16, 16, MAX_ALPHA}},
  }}};

  if (alpha == MAX_ALPHA)
  {
    return SIMPLE_COLORS_MAP[simpleColor];
  }

  auto simpleColorPixel = SIMPLE_COLORS_MAP[simpleColor];
  simpleColorPixel.SetA(alpha);
  return simpleColorPixel;
}

} // namespace GOOM::COLOR
