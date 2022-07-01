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
using UTILS::Logging;
using UTILS::Timer;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::Fraction;

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

  static constexpr uint32_t NUM_CIRCLES = 5;
  [[nodiscard]] auto GetCircleParams() const noexcept -> std::vector<Circle::Params>;
  [[nodiscard]] static auto GetCircleCentreTargets(const Point2dInt& screenMidPoint)
      -> std::array<Point2dInt, NUM_CIRCLES>;
  std::unique_ptr<Circles> m_circles{MakeCircles()};
  [[nodiscard]] auto MakeCircles() const noexcept -> std::unique_ptr<Circles>;

  auto UpdateAndDraw() noexcept -> void;

  bool m_resetCircles = false;
  WeightedColorMaps m_lastWeightedColorMaps{};
  Point2dInt m_lastZoomMidpoint{};
  auto ResetCircles() noexcept -> void;

  static constexpr uint32_t BLANK_TIME = 30;
  Timer m_blankTimer{BLANK_TIME, true};
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
  std::vector<Circle::Params> circleParams(NUM_CIRCLES);

  const PluginInfo::Screen& screenInfo = m_fxHelper.GetGoomInfo().GetScreenInfo();

  static constexpr float MIN_RADIUS_REDUCER = 0.90F;
  static constexpr float MAX_RADIUS_REDUCER = 1.01F;
  const float radiusReducer =
      m_fxHelper.GetGoomRand().GetRandInRange(MIN_RADIUS_REDUCER, MAX_RADIUS_REDUCER);

  static constexpr float MIN_RADIUS_MARGIN = 10.0F;
  static constexpr float MAX_RADIUS_MARGIN = 50.01F;
  const float radiusMargin =
      m_fxHelper.GetGoomRand().GetRandInRange(MIN_RADIUS_MARGIN, MAX_RADIUS_MARGIN);
  const float maxRadius = 0.5F * static_cast<float>(std::min(screenInfo.width, screenInfo.height));
  const float radius0 = maxRadius - radiusMargin;

  circleParams[0].circleRadius = radius0;
  for (size_t i = 1; i < NUM_CIRCLES; ++i)
  {
    circleParams[i].circleRadius = radiusReducer * circleParams[i - 1].circleRadius;
  }

  const Point2dInt screenMidPoint = MidpointFromOrigin({screenInfo.width, screenInfo.height});
  std::array<Point2dInt, NUM_CIRCLES> circleCentreTargets = GetCircleCentreTargets(screenMidPoint);
  for (size_t i = 0; i < NUM_CIRCLES; ++i)
  {
    circleParams[i].circleCentreTarget = circleCentreTargets.at(i);
  }

  return circleParams;
}

auto CirclesFx::CirclesFxImpl::GetCircleCentreTargets(const Point2dInt& screenMidPoint)
    -> std::array<Point2dInt, NUM_CIRCLES>
{
  std::array<Point2dInt, NUM_CIRCLES> circleCentreTargets{};

  const int32_t width = 2 * screenMidPoint.x;
  const int32_t height = 2 * screenMidPoint.y;
  static constexpr Fraction SMALL_FRAC{1, 10};
  static constexpr Fraction LARGE_FRAC = 1 - SMALL_FRAC;
  static_assert(5 == NUM_CIRCLES);
  circleCentreTargets[0] = screenMidPoint;
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

  m_lastWeightedColorMaps = weightedColorMaps;
  m_resetCircles = true;
}

inline auto CirclesFx::CirclesFxImpl::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept
    -> void
{
  m_circles->SetZoomMidpoint(zoomMidpoint);
  m_lastZoomMidpoint = zoomMidpoint;
}

inline auto CirclesFx::CirclesFxImpl::Start() noexcept -> void
{
  m_blankTimer.SetToFinished();

  m_circles->Start();
}

inline auto CirclesFx::CirclesFxImpl::ApplyMultiple() noexcept -> void
{
  UpdateAndDraw();
  ResetCircles();
}

inline auto CirclesFx::CirclesFxImpl::UpdateAndDraw() noexcept -> void
{
  m_blankTimer.Increment();

  if (not m_blankTimer.Finished())
  {
    Expects(m_circles->HasPositionTJustHitStartBoundary());
    return;
  }

  m_circles->UpdateAndDraw();

  if (m_circles->HasPositionTJustHitStartBoundary())
  {
    m_blankTimer.ResetToZero();
  }
}

inline auto CirclesFx::CirclesFxImpl::ResetCircles() noexcept -> void
{
  if ((not m_resetCircles) or (not m_circles->HasPositionTJustHitStartBoundary()))
  {
    return;
  }

  Expects(not m_blankTimer.Finished());

  m_circles = MakeCircles();
  m_circles->SetWeightedColorMaps(m_lastWeightedColorMaps.mainColorMaps,
                                  m_lastWeightedColorMaps.lowColorMaps);
  m_circles->SetZoomMidpoint(m_lastZoomMidpoint);
  m_circles->Start();
  m_resetCircles = false;

  Ensures(m_circles->HasPositionTJustHitStartBoundary());
}

} // namespace GOOM::VISUAL_FX
