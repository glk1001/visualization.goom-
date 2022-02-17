#include "low_density_blurrer.h"

#include "color/colormaps.h"
#include "colorizer.h"
#include "draw/goom_draw.h"
#include "fractal.h"
#include "goom_graphic.h"
#include "utils/math/misc.h"

#include <cmath>
#include <cstdint>
#include <vector>

namespace GOOM::VISUAL_FX::IFS
{

using COLOR::GetBrighterColor;
using COLOR::GetColorAverage;
using COLOR::IColorMap;
using DRAW::IGoomDraw;
using UTILS::MATH::U_HALF;

LowDensityBlurrer::LowDensityBlurrer(IGoomDraw& draw,
                                     const uint32_t width,
                                     const Colorizer* const colorizer) noexcept
  : m_draw{draw}, m_width{width}, m_colorizer{colorizer}
{
}

void LowDensityBlurrer::SetWidth(const uint32_t val)
{
  constexpr std::array VALID_WIDTHS{3, 5, 7};

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4389) // '==' mismatch. Not sure why?
#endif
  if (std::find(cbegin(VALID_WIDTHS), cend(VALID_WIDTHS), val) == cend(VALID_WIDTHS))
  {
    throw std::logic_error(std20::format("Invalid blur width {}.", val));
  }
#ifdef _MSC_VER
#pragma warning(pop)
#endif

  m_width = val;
}

void LowDensityBlurrer::DoBlur(std::vector<IfsPoint>& lowDensityPoints,
                               const uint32_t maxLowDensityCount) const
{
  std::vector<Pixel> neighbours(static_cast<size_t>(m_width) * static_cast<size_t>(m_width));
  const float logMaxLowDensityCount = std::log(static_cast<float>(maxLowDensityCount));

  float t = 0.0;
  const float tStep = 1.0F / static_cast<float>(lowDensityPoints.size());
  const uint32_t halfWidth = U_HALF * m_width;
  for (auto& point : lowDensityPoints)
  {
    if ((point.GetX() < halfWidth) || (point.GetY() < halfWidth) ||
        (point.GetX() >= (m_draw.GetScreenWidth() - halfWidth)) ||
        (point.GetY() >= (m_draw.GetScreenHeight() - halfWidth)))
    {
      point.SetCount(0); // just signal that no need to set buff
      continue;
    }

    size_t n = 0;
    auto neighY = static_cast<int32_t>(point.GetY() - (m_width / 2));
    for (size_t i = 0; i < m_width; ++i)
    {
      auto neighX = static_cast<int32_t>(point.GetX() - (m_width / 2));
      for (size_t j = 0; j < m_width; ++j)
      {
        neighbours[n] = m_draw.GetPixel(neighX, neighY);
        ++n;
        ++neighX;
      }
      ++neighY;
    }

    SetPointColor(point, t, logMaxLowDensityCount, neighbours);

    t += tStep;
  }

  for (const auto& point : lowDensityPoints)
  {
    if (0 == point.GetCount())
    {
      continue;
    }
    const std::vector<Pixel> colors{point.GetColor(), point.GetColor()};
    // TODO bitmap here
    m_draw.DrawPixels(static_cast<int32_t>(point.GetX()), static_cast<int32_t>(point.GetY()),
                      colors);
  }
}

void LowDensityBlurrer::SetPointColor(IfsPoint& point,
                                      const float t,
                                      const float logMaxLowDensityCount,
                                      const std::vector<Pixel>& neighbours) const
{
  const float logAlpha = point.GetCount() <= 1 ? 1.0F
                                               : (std::log(static_cast<float>(point.GetCount())) /
                                                     logMaxLowDensityCount);
  constexpr float BRIGHTNESS = 0.5F;

  switch (m_colorMode)
  {
    case BlurrerColorMode::SINGLE_NO_NEIGHBOURS:
      point.SetColor(m_singleColor);
      break;
    case BlurrerColorMode::SINGLE_WITH_NEIGHBOURS:
      point.SetColor(IColorMap::GetColorMix(
          m_singleColor, GetColorAverage(neighbours.size(), neighbours), m_neighbourMixFactor));
      break;
    case BlurrerColorMode::SIMI_NO_NEIGHBOURS:
      point.SetColor(point.GetSimi()->GetColor());
      break;
    case BlurrerColorMode::SIMI_WITH_NEIGHBOURS:
    {
      const float fx =
          static_cast<float>(point.GetX()) / static_cast<float>(m_draw.GetScreenWidth());
      const float fy =
          static_cast<float>(point.GetY()) / static_cast<float>(m_draw.GetScreenHeight());
      point.SetColor(m_colorizer->GetMixedColor(
          IColorMap::GetColorMix(point.GetSimi()->GetColor(),
                                 GetColorAverage(neighbours.size(), neighbours),
                                 m_neighbourMixFactor),
          point.GetCount(), BRIGHTNESS, logAlpha, fx, fy));
      break;
    }
    case BlurrerColorMode::SMOOTH_NO_NEIGHBOURS:
      point.SetColor(point.GetSimi()->GetColorMap()->GetColor(t));
      break;
    case BlurrerColorMode::SMOOTH_WITH_NEIGHBOURS:
    {
      const float fx =
          static_cast<float>(point.GetX()) / static_cast<float>(m_draw.GetScreenWidth());
      const float fy =
          static_cast<float>(point.GetY()) / static_cast<float>(m_draw.GetScreenHeight());
      point.SetColor(m_colorizer->GetMixedColor(
          IColorMap::GetColorMix(point.GetSimi()->GetColorMap()->GetColor(t),
                                 GetColorAverage(neighbours.size(), neighbours),
                                 m_neighbourMixFactor),
          point.GetCount(), BRIGHTNESS, logAlpha, fx, fy));
      break;
    }
    default:
      break;
  }

  point.SetColor(GetGammaCorrection(BRIGHTNESS * logAlpha, point.GetColor()));
}

inline auto LowDensityBlurrer::GetGammaCorrection(const float brightness, const Pixel& color) const
    -> Pixel
{
  if constexpr (1.0F == GAMMA)
  {
    return GetBrighterColor(brightness, color);
  }
  return m_gammaCorrect.GetCorrection(brightness, color);
}

} // namespace GOOM::VISUAL_FX::IFS
