#include "circles_fx.h"

//#undef NO_LOGGING

#include "draw/goom_draw.h"
#include "fx_helper.h"
#include "goom/spimpl.h"
#include "logging.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/misc.h"
#include "utils/timer.h"
#include "visual_fx/circles/circles.h"

#include <array>
#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX
{

using CIRCLES::Circle;
using CIRCLES::Circles;
using UTILS::Logging; // NOLINT(misc-unused-using-decls)
using UTILS::Timer;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::Fraction;
using UTILS::MATH::U_HALF;

class CirclesFx::CirclesFxImpl
{
public:
  CirclesFxImpl(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>;
  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;

  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;

  auto Start() noexcept -> void;
  auto ApplyMultiple() noexcept -> void;

private:
  const FxHelper& m_fxHelper;
  const SmallImageBitmaps& m_smallBitmaps;
  const uint32_t m_screenWidth      = m_fxHelper.GetGoomInfo().GetScreenInfo().width;
  const uint32_t m_screenHeight     = m_fxHelper.GetGoomInfo().GetScreenInfo().height;
  const Point2dInt m_screenMidPoint = MidpointFromOrigin({m_screenWidth, m_screenHeight});

  static constexpr uint32_t NUM_CIRCLES = 5;
  Point2dInt m_lastCircleCentreStart{m_screenMidPoint};
  Point2dInt m_lastZoomMidpoint{
      static_cast<int32_t>(U_HALF * m_fxHelper.GetDraw().GetScreenWidth()),
      static_cast<int32_t>(U_HALF* m_fxHelper.GetDraw().GetScreenHeight())};
  std::unique_ptr<Circles> m_circles{MakeCircles()};
  [[nodiscard]] auto MakeCircles() const noexcept -> std::unique_ptr<Circles>;
  [[nodiscard]] auto GetCircleParams() const noexcept -> std::vector<Circle::Params>;
  [[nodiscard]] auto GetCircleRadius0() const noexcept -> float;
  [[nodiscard]] auto GetCircleRadiusReducer() const noexcept -> float;
  [[nodiscard]] auto GetAllCirclesCentreStart() const noexcept -> Point2dInt;
  [[nodiscard]] auto GetCircleCentreTargets() const -> std::array<Point2dInt, NUM_CIRCLES>;

  auto UpdateAndDraw() noexcept -> void;

  bool m_fullCirclesReset        = false;
  bool m_resetCircleCentreStarts = false;
  WeightedColorMaps m_lastWeightedColorMaps{};
  static constexpr uint32_t LERP_TO_NEW_CIRCLE_CENTRE_TIME = 20;
  Timer m_lerpToNewCircleCentreTimer{LERP_TO_NEW_CIRCLE_CENTRE_TIME, true};
  Point2dInt m_newCircleCentreStart{m_lastCircleCentreStart};
  float m_newRadius0 = 0.0F;
  auto ResetCircles() noexcept -> void;
  auto CheckCircleCentreStartsReset() noexcept -> void;
  auto CheckFullCirclesReset() noexcept -> void;

  static constexpr uint32_t MIN_BLANK_AT_START_TIME = 1;
  static constexpr uint32_t MAX_BLANK_AT_START_TIME = 5;
  uint32_t m_blankAtStartTime =
      m_fxHelper.GetGoomRand().GetRandInRange(MIN_BLANK_AT_START_TIME, MAX_BLANK_AT_START_TIME + 1);
  Timer m_blankAtStartTimer{m_blankAtStartTime, true};
};

CirclesFx::CirclesFx(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept
  : m_pimpl{spimpl::make_unique_impl<CirclesFxImpl>(fxHelper, smallBitmaps)}
{
}

auto CirclesFx::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return m_pimpl->GetCurrentColorMapsNames();
}

auto CirclesFx::SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  m_pimpl->SetWeightedColorMaps(weightedColorMaps);
}

auto CirclesFx::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  m_pimpl->SetZoomMidpoint(zoomMidpoint);
}

auto CirclesFx::Start() noexcept -> void
{
  m_pimpl->Start();
}

auto CirclesFx::Finish() noexcept -> void
{
  // nothing to do
}

auto CirclesFx::GetFxName() const noexcept -> std::string
{
  return "circles";
}

auto CirclesFx::ApplyMultiple() noexcept -> void
{
  m_pimpl->ApplyMultiple();
}

CirclesFx::CirclesFxImpl::CirclesFxImpl(const FxHelper& fxHelper,
                                        const SmallImageBitmaps& smallBitmaps) noexcept
  : m_fxHelper{fxHelper}, m_smallBitmaps{smallBitmaps}
{
}

inline auto CirclesFx::CirclesFxImpl::MakeCircles() const noexcept -> std::unique_ptr<Circles>
{
  return std::make_unique<Circles>(m_fxHelper, m_smallBitmaps, NUM_CIRCLES, GetCircleParams());
}

auto CirclesFx::CirclesFxImpl::GetCircleParams() const noexcept -> std::vector<Circle::Params>
{
  auto circleParams = std::vector<Circle::Params>(NUM_CIRCLES);

  const auto radiusReducer = GetCircleRadiusReducer();

  circleParams[0].circleRadius = GetCircleRadius0();
  for (auto i = 1U; i < NUM_CIRCLES; ++i)
  {
    circleParams[i].circleRadius = radiusReducer * circleParams[i - 1].circleRadius;
  }

  auto circleCentreTargets = GetCircleCentreTargets();
  for (auto i = 0U; i < NUM_CIRCLES; ++i)
  {
    circleParams[i].circleCentreStart  = m_lastCircleCentreStart;
    circleParams[i].circleCentreTarget = circleCentreTargets.at(i);
  }

  return circleParams;
}

inline auto CirclesFx::CirclesFxImpl::GetCircleRadius0() const noexcept -> float
{
  static constexpr auto MIN_RADIUS_MARGIN = 10.0F;
  static constexpr auto MAX_RADIUS_MARGIN = 50.01F;
  const auto radiusMargin =
      m_fxHelper.GetGoomRand().GetRandInRange(MIN_RADIUS_MARGIN, MAX_RADIUS_MARGIN);

  const auto maxRadius = 0.5F * static_cast<float>(std::min(m_screenWidth, m_screenHeight));

  return maxRadius - radiusMargin;
}

inline auto CirclesFx::CirclesFxImpl::GetCircleRadiusReducer() const noexcept -> float
{
  if (static constexpr auto PROB_SAME_RADIUS = 0.6F;
      m_fxHelper.GetGoomRand().ProbabilityOf(PROB_SAME_RADIUS))
  {
    return 1.0F;
  }

  static constexpr auto MIN_RADIUS_REDUCER = 0.90F;
  static constexpr auto MAX_RADIUS_REDUCER = 1.01F;
  return m_fxHelper.GetGoomRand().GetRandInRange(MIN_RADIUS_REDUCER, MAX_RADIUS_REDUCER);
}

auto CirclesFx::CirclesFxImpl::GetAllCirclesCentreStart() const noexcept -> Point2dInt
{
  static constexpr auto MAX_CLOSE_TO_ZOOM_POINT_T = 0.75F;
  const auto t = m_fxHelper.GetGoomRand().GetRandInRange(0.0F, MAX_CLOSE_TO_ZOOM_POINT_T);
  return lerp(m_screenMidPoint, m_lastZoomMidpoint, t);
}

auto CirclesFx::CirclesFxImpl::GetCircleCentreTargets() const -> std::array<Point2dInt, NUM_CIRCLES>
{
  auto circleCentreTargets = std::array<Point2dInt, NUM_CIRCLES>{};

  const auto width                 = 2 * m_screenMidPoint.x;
  const auto height                = 2 * m_screenMidPoint.y;
  static constexpr auto SMALL_FRAC = Fraction{1, 10};
  static constexpr auto LARGE_FRAC = 1 - SMALL_FRAC;
  static_assert(5 == NUM_CIRCLES);
  circleCentreTargets[0] = m_screenMidPoint;
  circleCentreTargets[1] = {SMALL_FRAC * width, SMALL_FRAC * height};
  circleCentreTargets[2] = {LARGE_FRAC * width, SMALL_FRAC * height};
  circleCentreTargets[3] = {LARGE_FRAC * width, LARGE_FRAC * height};
  circleCentreTargets[4] = {SMALL_FRAC * width, LARGE_FRAC * height};

  return circleCentreTargets;
}

inline auto CirclesFx::CirclesFxImpl::GetCurrentColorMapsNames() const noexcept
    -> std::vector<std::string>
{
  return {};
}

inline auto CirclesFx::CirclesFxImpl::SetWeightedColorMaps(
    const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  Expects(weightedColorMaps.mainColorMaps != nullptr);
  Expects(weightedColorMaps.lowColorMaps != nullptr);

  m_circles->SetWeightedColorMaps(weightedColorMaps.mainColorMaps, weightedColorMaps.lowColorMaps);

  m_blankAtStartTime =
      m_fxHelper.GetGoomRand().GetRandInRange(MIN_BLANK_AT_START_TIME, MAX_BLANK_AT_START_TIME + 1);

  m_lastWeightedColorMaps   = weightedColorMaps;
  m_fullCirclesReset        = true;
  m_resetCircleCentreStarts = true;
  m_lerpToNewCircleCentreTimer.SetToFinished();
}

inline auto CirclesFx::CirclesFxImpl::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept
    -> void
{
  m_circles->SetZoomMidpoint(zoomMidpoint);
  m_lastZoomMidpoint = zoomMidpoint;
}

inline auto CirclesFx::CirclesFxImpl::Start() noexcept -> void
{
  m_blankAtStartTimer.SetToFinished();

  m_circles->Start();
}

inline auto CirclesFx::CirclesFxImpl::ApplyMultiple() noexcept -> void
{
  UpdateAndDraw();
  ResetCircles();
}

inline auto CirclesFx::CirclesFxImpl::UpdateAndDraw() noexcept -> void
{
  m_blankAtStartTimer.Increment();

  if (not m_blankAtStartTimer.Finished())
  {
    Expects(m_circles->HasPositionTJustHitStartBoundary());
    return;
  }

  m_circles->UpdateAndDraw();

  if (m_circles->HasPositionTJustHitStartBoundary())
  {
    m_blankAtStartTimer.SetTimeLimit(m_blankAtStartTime);
  }
}

inline auto CirclesFx::CirclesFxImpl::ResetCircles() noexcept -> void
{
  CheckCircleCentreStartsReset();
  CheckFullCirclesReset();
}

auto CirclesFx::CirclesFxImpl::CheckCircleCentreStartsReset() noexcept -> void
{
  if (m_resetCircleCentreStarts)
  {
    Expects(m_lerpToNewCircleCentreTimer.Finished());
    m_lerpToNewCircleCentreTimer.ResetToZero();
    m_newCircleCentreStart    = GetAllCirclesCentreStart();
    m_newRadius0              = GetCircleRadius0();
    m_resetCircleCentreStarts = false;
  }
  if (not m_lerpToNewCircleCentreTimer.Finished())
  {
    const auto t = static_cast<float>(m_lerpToNewCircleCentreTimer.GetCurrentCount()) /
                   static_cast<float>(m_lerpToNewCircleCentreTimer.GetTimeLimit());
    m_lastCircleCentreStart = lerp(m_lastCircleCentreStart, m_newCircleCentreStart, t);
    m_circles->SetNewCircleCentreAndRadius(m_lastCircleCentreStart, m_newRadius0);
    m_lerpToNewCircleCentreTimer.Increment();
  }
}

inline auto CirclesFx::CirclesFxImpl::CheckFullCirclesReset() noexcept -> void
{
  if (not m_fullCirclesReset)
  {
    return;
  }
  if (not m_circles->HasPositionTJustHitStartBoundary())
  {
    return;
  }

  Expects(not m_blankAtStartTimer.Finished());

  m_circles = MakeCircles();
  m_circles->SetWeightedColorMaps(m_lastWeightedColorMaps.mainColorMaps,
                                  m_lastWeightedColorMaps.lowColorMaps);
  m_circles->SetZoomMidpoint(m_lastZoomMidpoint);
  m_circles->Start();
  m_fullCirclesReset = false;

  m_resetCircleCentreStarts = false;
  m_lerpToNewCircleCentreTimer.SetToFinished();

  Ensures(m_circles->HasPositionTJustHitStartBoundary());
}

} // namespace GOOM::VISUAL_FX
