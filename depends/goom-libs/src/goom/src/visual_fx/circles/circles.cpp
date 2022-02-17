#include "circles.h"

#include "color/random_colormaps.h"
#include "point2d.h"
#include "utils/graphics/image_bitmaps.h"
#include "utils/graphics/small_image_bitmaps.h"

#include <algorithm>

namespace GOOM::VISUAL_FX::CIRCLES
{

using COLOR::GammaCorrection;
using COLOR::GetBrighterColor;
using COLOR::RandomColorMaps;
using UTILS::GRAPHICS::ImageBitmap;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::IGoomRand;

Circles::Circles(const FxHelper& fxHelper,
                 const SmallImageBitmaps& smallBitmaps,
                 const uint32_t numCircles,
                 const std::vector<Circle::Params>& circleParams)
  : m_bitmapGetter{fxHelper.GetGoomRand(), smallBitmaps},
    m_numCircles{numCircles},
    m_circles{GetCircles(fxHelper,
                         {BitmapGetter::MIN_DOT_DIAMETER, BitmapGetter::MAX_DOT_DIAMETER,
                          m_bitmapGetter, m_gammaCorrect},
                         m_numCircles,
                         circleParams)}
{
}

auto Circles::GetCircles(const FxHelper& fxHelper,
                         const Circle::Helper& helper,
                         const uint32_t numCircles,
                         const std::vector<Circle::Params>& circleParams) -> std::vector<Circle>
{
  std::vector<Circle> circles{};
  circles.reserve(numCircles);

  for (size_t i = 0; i < numCircles; ++i)
  {
    circles.emplace_back(fxHelper, helper, circleParams[i]);
  }

  return circles;
}

void Circles::SetWeightedColorMaps(const std::shared_ptr<RandomColorMaps> weightedMaps,
                                   const std::shared_ptr<RandomColorMaps> weightedLowMaps)
{
  std::for_each(begin(m_circles), end(m_circles),
                [&weightedMaps, &weightedLowMaps](Circle& circle)
                { circle.SetWeightedColorMaps(weightedMaps, weightedLowMaps); });

  m_bitmapGetter.ChangeCurrentBitmap();
}

void Circles::SetZoomMidpoint(const Point2dInt& zoomMidpoint)
{
  std::for_each(begin(m_circles), end(m_circles),
                [&zoomMidpoint](Circle& circle) { circle.SetZoomMidpoint(zoomMidpoint); });
}

void Circles::Start()
{
  std::for_each(begin(m_circles), end(m_circles), [](Circle& circle) { circle.Start(); });
}

void Circles::UpdateAndDraw()
{
  std::for_each(begin(m_circles), end(m_circles), [](Circle& circle) { circle.UpdateAndDraw(); });
}

} // namespace GOOM::VISUAL_FX::CIRCLES
