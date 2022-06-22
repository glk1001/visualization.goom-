#include "color_adjustment.h"

#include "color_utils.h"
#include "goom_config.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201) // glm: nonstandard extension used: nameless struct/union
#pragma warning(disable : 4242) // possible loss of data
#pragma warning(disable : 4244) // possible loss of data
#endif
#include <vivid/vivid.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace GOOM::COLOR
{

using UTILS::MATH::FloatsEqual;

auto ColorAdjustment::GetAdjustment(const float brightness, const Pixel& color) const -> Pixel
{
  Expects(brightness >= 0.0F);

  if (brightness < m_ignoreThreshold)
  {
    return GetBrighterColor(brightness, color);
  }

  Pixel adjustedColor = color;

  if (m_doAlterChroma)
  {
    adjustedColor = GetAlteredChroma(m_alterChromaFactor, adjustedColor);
  }

  const auto newR = static_cast<uint32_t>(std::round(
      channel_limits<float>::max() * std::pow(brightness * adjustedColor.RFlt(), m_gamma)));
  const auto newG = static_cast<uint32_t>(std::round(
      channel_limits<float>::max() * std::pow(brightness * adjustedColor.GFlt(), m_gamma)));
  const auto newB = static_cast<uint32_t>(std::round(
      channel_limits<float>::max() * std::pow(brightness * adjustedColor.BFlt(), m_gamma)));
  const uint32_t newA = color.A();

  return Pixel{newR, newG, newB, newA};
}

auto ColorAdjustment::GetAlteredChroma(const float lchYFactor, const Pixel& color) -> Pixel
{
  Expects(lchYFactor > 0.0F);

  if (FloatsEqual(1.0F, lchYFactor))
  {
    return color;
  }

  const vivid::col8_t rgb8 = {color.R(), color.G(), color.B()};
  vivid::lch_t lch = vivid::lch::fromSrgb(vivid::rgb::fromRgb8(rgb8));
  static constexpr float MAX_LCH_Y = 140.0F;
  lch.y = std::min(lch.y * lchYFactor, MAX_LCH_Y);
  const vivid::col8_t newRgb8 = vivid::rgb8::fromRgb(vivid::srgb::fromLch(lch));
  return Pixel{
      {newRgb8.r, newRgb8.g, newRgb8.b, MAX_ALPHA}
  };
}

} // namespace GOOM::COLOR
