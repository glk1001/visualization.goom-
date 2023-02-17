#include "color_utils.h"

#include "goom_graphic.h"

#include <cmath>

namespace GOOM::COLOR
{

namespace
{

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

} // namespace

auto GetLightenedColor(const Pixel& oldColor, const float power) -> Pixel
{
  auto pixel = oldColor;

  pixel.SetR(Lighten(pixel.R(), power));
  pixel.SetG(Lighten(pixel.G(), power));
  pixel.SetB(Lighten(pixel.B(), power));

  return pixel;
}

} // namespace GOOM::COLOR
