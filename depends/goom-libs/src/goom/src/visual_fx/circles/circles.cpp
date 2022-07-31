#include "circles.h"

#include "color/random_color_maps.h"
#include "point2d.h"
#include "utils/graphics/image_bitmaps.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/paths.h"

#include <algorithm>

namespace GOOM::VISUAL_FX::CIRCLES
{

using COLOR::RandomColorMaps;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::OscillatingFunction;
using UTILS::MATH::OscillatingPath;

static constexpr auto LINE_DOT_DIAMETER = BitmapGetter::MIN_DOT_DIAMETER;
static constexpr auto MIN_DOT_DIAMETER  = BitmapGetter::MIN_DOT_DIAMETER + 4;
static constexpr auto MAX_DOT_DIAMETER  = BitmapGetter::MAX_DOT_DIAMETER;

Circles::Circles(const FxHelper& fxHelper,
                 const SmallImageBitmaps& smallBitmaps,
                 const uint32_t numCircles,
                 const std::vector<Circle::Params>& circleParams) noexcept
  : m_goomRand{fxHelper.GetGoomRand()},
    m_goomInfo{fxHelper.GetGoomInfo()},
    m_bitmapGetter{fxHelper.GetGoomRand(), smallBitmaps},
    m_numCircles{numCircles},
    m_circles{GetCircles(fxHelper,
                         {LINE_DOT_DIAMETER, MIN_DOT_DIAMETER, MAX_DOT_DIAMETER, m_bitmapGetter},
                         GetPathParams(),
                         m_numCircles,
                         circleParams)}
{
}

auto Circles::GetCircles(const FxHelper& fxHelper,
                         const Circle::Helper& helper,
                         const OscillatingFunction::Params& pathParams,
                         const uint32_t numCircles,
                         const std::vector<Circle::Params>& circleParams) noexcept
    -> std::vector<Circle>
{
  auto circles = std::vector<Circle>{};
  circles.reserve(numCircles);

  for (auto i = 0U; i < numCircles; ++i)
  {
    circles.emplace_back(fxHelper, helper, circleParams[i], pathParams);
  }

  return circles;
}

auto Circles::SetWeightedColorMaps(
    const std::shared_ptr<const RandomColorMaps> weightedMaps,
    const std::shared_ptr<const RandomColorMaps> weightedLowMaps) noexcept -> void
{
  std::for_each(begin(m_circles),
                end(m_circles),
                [&weightedMaps, &weightedLowMaps](Circle& circle)
                { circle.SetWeightedColorMaps(weightedMaps, weightedLowMaps); });

  m_bitmapGetter.ChangeCurrentBitmap();

  static constexpr auto MIN_BLANK_TIME = 30U;
  static constexpr auto MAX_BLANK_TIME = 100U;
  const auto newBlankTime = m_goomRand.GetRandInRange(MIN_BLANK_TIME, MAX_BLANK_TIME + 1);
  std::for_each(begin(m_circles),
                end(m_circles),
                [&newBlankTime](Circle& circle) { circle.SetBlankTime(newBlankTime); });
}

auto Circles::SetNewTargetPoints() noexcept -> void
{
  if (not m_circles.front().HasPositionTJustHitABoundary())
  {
    return;
  }

  const auto lerpTFromFixedTarget = m_goomRand.GetRandInRange(0.0F, 0.7F);

  m_circles.front().SetMovingTargetPoint(GetCentreCircleTargetPoint(), lerpTFromFixedTarget);

  std::for_each(begin(m_circles) + 1,
                end(m_circles),
                [this, &lerpTFromFixedTarget](Circle& circle)
                { circle.SetMovingTargetPoint(m_zoomMidpoint, lerpTFromFixedTarget); });
}

inline auto Circles::GetCentreCircleTargetPoint() const noexcept -> Point2dInt
{
  if (static constexpr float PROB_FOLLOW_GIVEN_ZOOM_MIDPOINT = 0.2F;
      m_goomRand.ProbabilityOf(PROB_FOLLOW_GIVEN_ZOOM_MIDPOINT))
  {
    return m_zoomMidpoint;
  }

  return m_circles.at(m_goomRand.GetRandInRange(1U, static_cast<uint32_t>(m_circles.size())))
      .GetCircleCentreFixedTarget();
}

auto Circles::Start() noexcept -> void
{
  std::for_each(begin(m_circles), end(m_circles), [](Circle& circle) { circle.Start(); });
}

auto Circles::UpdateAndDraw() noexcept -> void
{
  SetNewTargetPoints();
  UpdateAndDrawCircles();
  UpdatePositionSpeed();
  UpdateCirclePathParams();
}

inline auto Circles::UpdateAndDrawCircles() noexcept -> void
{
  std::for_each(begin(m_circles), end(m_circles), [](Circle& circle) { circle.UpdateAndDraw(); });
}

auto Circles::UpdatePositionSpeed() noexcept -> void
{
  if (static constexpr auto PROB_NO_SPEED_CHANGE = 0.7F;
      m_goomRand.ProbabilityOf(PROB_NO_SPEED_CHANGE))
  {
    return;
  }

  static constexpr auto MIN_POSITION_STEPS = 100U;
  static constexpr auto MAX_POSITION_STEPS = 1000U;
  const auto newNumSteps                   = std::min(
      MIN_POSITION_STEPS + m_goomInfo.GetSoundEvents().GetTimeSinceLastGoom(), MAX_POSITION_STEPS);

  std::for_each(begin(m_circles),
                end(m_circles),
                [&newNumSteps](Circle& circle) { circle.UpdatePositionSpeed(newNumSteps); });
}

inline auto Circles::UpdateCirclePathParams() noexcept -> void
{
  if (m_goomInfo.GetSoundEvents().GetTimeSinceLastGoom() > 0)
  {
    return;
  }

  std::for_each(begin(m_circles),
                end(m_circles),
                [this](Circle& circle) { circle.SetPathParams(GetPathParams()); });
}

inline auto Circles::GetPathParams() const noexcept -> OscillatingFunction::Params
{
  static constexpr auto MIN_PATH_AMPLITUDE = 90.0F;
  static constexpr auto MAX_PATH_AMPLITUDE = 110.0F;
  static constexpr auto MIN_PATH_X_FREQ    = 0.9F;
  static constexpr auto MAX_PATH_X_FREQ    = 2.0F;
  static constexpr auto MIN_PATH_Y_FREQ    = 0.9F;
  static constexpr auto MAX_PATH_Y_FREQ    = 2.0F;

  return {
      m_goomRand.GetRandInRange(MIN_PATH_AMPLITUDE, MAX_PATH_AMPLITUDE),
      m_goomRand.GetRandInRange(MIN_PATH_X_FREQ, MAX_PATH_X_FREQ),
      m_goomRand.GetRandInRange(MIN_PATH_Y_FREQ, MAX_PATH_Y_FREQ),
  };
}


} // namespace GOOM::VISUAL_FX::CIRCLES
