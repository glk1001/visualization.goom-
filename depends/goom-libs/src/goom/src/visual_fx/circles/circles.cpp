#include "circles.h"

#include "color/random_colormaps.h"
#include "point2d.h"
#include "utils/graphics/image_bitmaps.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/paths.h"

#include <algorithm>

namespace GOOM::VISUAL_FX::CIRCLES
{

using COLOR::ColorCorrection;
using COLOR::GetBrighterColor;
using COLOR::RandomColorMaps;
using UTILS::GRAPHICS::ImageBitmap;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::OscillatingFunction;
using UTILS::MATH::OscillatingPath;

static constexpr uint32_t LINE_DOT_DIAMETER = BitmapGetter::MIN_DOT_DIAMETER;
static constexpr uint32_t MIN_DOT_DIAMETER = BitmapGetter::MIN_DOT_DIAMETER + 2;
static constexpr uint32_t MAX_DOT_DIAMETER = BitmapGetter::MAX_DOT_DIAMETER;

Circles::Circles(const FxHelper& fxHelper,
                 const SmallImageBitmaps& smallBitmaps,
                 const uint32_t numCircles,
                 const std::vector<Circle::Params>& circleParams)
  : m_goomRand{fxHelper.GetGoomRand()},
    m_goomInfo{fxHelper.GetGoomInfo()},
    m_bitmapGetter{fxHelper.GetGoomRand(), smallBitmaps},
    m_numCircles{numCircles},
    m_circles{GetCircles(
        fxHelper,
        {LINE_DOT_DIAMETER, MIN_DOT_DIAMETER, MAX_DOT_DIAMETER, m_bitmapGetter, m_colorCorrect},
        GetPathParams(),
        m_numCircles,
        circleParams)}
{
}

auto Circles::GetCircles(const FxHelper& fxHelper,
                         const Circle::Helper& helper,
                         const OscillatingFunction::Params& pathParams,
                         const uint32_t numCircles,
                         const std::vector<Circle::Params>& circleParams) -> std::vector<Circle>
{
  std::vector<Circle> circles{};
  circles.reserve(numCircles);

  for (size_t i = 0; i < numCircles; ++i)
  {
    circles.emplace_back(fxHelper, helper, circleParams[i], pathParams);
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
  UpdateAndDrawCircles();
  UpdatePositionSpeed();
  UpdateCirclePathParams();
}

inline void Circles::UpdateAndDrawCircles()
{
  std::for_each(begin(m_circles), end(m_circles), [](Circle& circle) { circle.UpdateAndDraw(); });
}

void Circles::UpdatePositionSpeed()
{
  if (constexpr float PROB_NO_SPEED_CHANGE = 0.7F; m_goomRand.ProbabilityOf(PROB_NO_SPEED_CHANGE))
  {
    return;
  }

  static constexpr uint32_t MIN_POSITION_STEPS = 100;
  static constexpr uint32_t MAX_POSITION_STEPS = 600;
  const uint32_t newNumSteps = std::min(
      MIN_POSITION_STEPS + m_goomInfo.GetSoundInfo().GetTimeSinceLastGoom(), MAX_POSITION_STEPS);

  std::for_each(begin(m_circles), end(m_circles),
                [&newNumSteps](Circle& circle) { circle.UpdatePositionSpeed(newNumSteps); });
}

inline void Circles::UpdateCirclePathParams()
{
  if (m_goomInfo.GetSoundInfo().GetTimeSinceLastGoom() > 0)
  {
    return;
  }

  std::for_each(begin(m_circles), end(m_circles),
                [this](Circle& circle) { circle.SetPathParams(GetPathParams()); });
}

inline auto Circles::GetPathParams() const -> OscillatingFunction::Params
{
  static constexpr float MIN_PATH_AMPLITUDE = 90.0F;
  static constexpr float MAX_PATH_AMPLITUDE = 110.0F;
  static constexpr float MIN_PATH_X_FREQ = 0.9F;
  static constexpr float MAX_PATH_X_FREQ = 2.0F;
  static constexpr float MIN_PATH_Y_FREQ = 0.9F;
  static constexpr float MAX_PATH_Y_FREQ = 2.0F;

  return {
      m_goomRand.GetRandInRange(MIN_PATH_AMPLITUDE, MAX_PATH_AMPLITUDE),
      m_goomRand.GetRandInRange(MIN_PATH_X_FREQ, MAX_PATH_X_FREQ),
      m_goomRand.GetRandInRange(MIN_PATH_Y_FREQ, MAX_PATH_Y_FREQ),
  };
}


} // namespace GOOM::VISUAL_FX::CIRCLES
