#include "low_density_blurrer.h"

#include "color/colormaps.h"
#include "colorizer.h"
#include "draw/goom_draw.h"
#include "fractal.h"
#include "goom_graphic.h"
#include "utils/graphics/image_bitmaps.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/goom_rand_base.h"
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
using DRAW::MultiplePixels;
using UTILS::GRAPHICS::ImageBitmap;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::U_HALF;

LowDensityBlurrer::LowDensityBlurrer(IGoomDraw& draw,
                                     const IGoomRand& goomRand,
                                     const uint32_t width,
                                     const Colorizer* const colorizer,
                                     const SmallImageBitmaps& smallBitmaps) noexcept
  : m_draw{draw},
    m_goomRand{goomRand},
    m_width{width},
    m_smallBitmaps{smallBitmaps},
    m_colorizer{colorizer}
{
}

void LowDensityBlurrer::SetWidth(const uint32_t val)
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4389) // '==' mismatch. Not sure why?
#endif
  if (static constexpr std::array VALID_WIDTHS{3, 5, 7};
      std::find(cbegin(VALID_WIDTHS), cend(VALID_WIDTHS), val) == cend(VALID_WIDTHS))
  {
    throw std::logic_error(std20::format("Invalid blur width {}.", val));
  }
#ifdef _MSC_VER
#pragma warning(pop)
#endif

  m_width = val;
}

void LowDensityBlurrer::SetColorMode(const BlurrerColorMode colorMode)
{
  m_colorMode = colorMode;

  static constexpr float PROB_USE_BITMAPS = 0.1F;
  const bool useBitmaps = m_goomRand.ProbabilityOf(PROB_USE_BITMAPS);
  m_currentImageBitmap = GetImageBitmap(useBitmaps);
}

auto LowDensityBlurrer::GetImageBitmap(const bool useBitmaps) const -> const ImageBitmap*
{
  if (!useBitmaps)
  {
    return nullptr;
  }

  static constexpr uint32_t MIN_RES = 3;
  static constexpr uint32_t MAX_RES = 7;
  const uint32_t res = m_goomRand.GetRandInRange(MIN_RES, MAX_RES);

  return &m_smallBitmaps.GetImageBitmap(SmallImageBitmaps::ImageNames::SPHERE, res);
}

void LowDensityBlurrer::DoBlur(std::vector<IfsPoint>& lowDensityPoints,
                               const uint32_t maxLowDensityCount) const
{
  MultiplePixels neighbours(static_cast<size_t>(m_width) * static_cast<size_t>(m_width));
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
        neighbours[n] = m_draw.GetPixel({neighX, neighY});
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

    if (const Point2dInt pt = {static_cast<int32_t>(point.GetX()),
                               static_cast<int32_t>(point.GetY())};
        nullptr == m_currentImageBitmap)
    {
      const MultiplePixels colors{point.GetColor(), point.GetColor()};
      m_draw.DrawPixels(pt, colors);
    }
    else
    {
      const auto getColor =
          [&point]([[maybe_unused]] const size_t xVal, [[maybe_unused]] const size_t yVal,
                   [[maybe_unused]] const Pixel& bgnd) { return point.GetColor(); };
      m_draw.Bitmap(pt, *m_currentImageBitmap, {getColor, getColor});
    }
  }
}

void LowDensityBlurrer::SetPointColor(IfsPoint& point,
                                      const float t,
                                      const float logMaxLowDensityCount,
                                      const MultiplePixels& neighbours) const
{
  const float logAlpha =
      point.GetCount() <= 1
          ? 1.0F
          : (std::log(static_cast<float>(point.GetCount())) / logMaxLowDensityCount);

  const float brightness = GetBrightness();

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
      const Pixel simiColor = point.GetSimi()->GetColor();
      const Pixel mixedPointColor =
          GetMixedPointColor(simiColor, point, neighbours, brightness, logAlpha);
      point.SetColor(mixedPointColor);
      break;
    }
    case BlurrerColorMode::SMOOTH_NO_NEIGHBOURS:
      point.SetColor(point.GetSimi()->GetColorMap()->GetColor(t));
      break;
    case BlurrerColorMode::SMOOTH_WITH_NEIGHBOURS:
    {
      const Pixel simiSmoothColor = point.GetSimi()->GetColorMap()->GetColor(t);
      const Pixel mixedPointColor =
          GetMixedPointColor(simiSmoothColor, point, neighbours, brightness, logAlpha);
      point.SetColor(mixedPointColor);
      break;
    }
    default:
      break;
  }

  point.SetColor(m_colorAdjust.GetAdjustment(brightness * logAlpha, point.GetColor()));
}

inline auto LowDensityBlurrer::GetBrightness() const -> float
{
  static constexpr float NO_NEIGHBOUR_BRIGHTNESS = 1.5F;
  static constexpr float NEIGHBOUR_BRIGHTNESS = 0.1F;

  float brightness;

  switch (m_colorMode)
  {
    case BlurrerColorMode::SINGLE_NO_NEIGHBOURS:
    case BlurrerColorMode::SIMI_NO_NEIGHBOURS:
      brightness = NO_NEIGHBOUR_BRIGHTNESS;
      break;
    case BlurrerColorMode::SINGLE_WITH_NEIGHBOURS:
    case BlurrerColorMode::SIMI_WITH_NEIGHBOURS:
    case BlurrerColorMode::SMOOTH_NO_NEIGHBOURS:
    case BlurrerColorMode::SMOOTH_WITH_NEIGHBOURS:
      brightness = NEIGHBOUR_BRIGHTNESS;
      break;
    default:
      throw std::logic_error("Unexpected m_colorMode enum.");
  }

  if (nullptr != m_currentImageBitmap)
  {
    static constexpr float BITMAP_BRIGHTNESS_CUT = 0.5F;
    brightness *= BITMAP_BRIGHTNESS_CUT;
  }

  return brightness;
}

inline auto LowDensityBlurrer::GetMixedPointColor(const Pixel& baseColor,
                                                  const IfsPoint& point,
                                                  const MultiplePixels& neighbours,
                                                  const float brightness,
                                                  const float logAlpha) const -> Pixel
{
  const float fx = static_cast<float>(point.GetX()) / static_cast<float>(m_draw.GetScreenWidth());
  const float fy = static_cast<float>(point.GetY()) / static_cast<float>(m_draw.GetScreenHeight());

  const Pixel neighbourhoodAverageColor = GetColorAverage(neighbours.size(), neighbours);

  const Pixel baseAndNeighbourhoodMixedColor =
      IColorMap::GetColorMix(baseColor, neighbourhoodAverageColor, m_neighbourMixFactor);

  return m_colorizer->GetMixedColor(baseAndNeighbourhoodMixedColor, point.GetCount(), brightness,
                                    logAlpha, fx, fy);
}

} // namespace GOOM::VISUAL_FX::IFS
