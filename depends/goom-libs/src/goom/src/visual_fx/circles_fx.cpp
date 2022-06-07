#include "circles_fx.h"

#include "color/colormaps.h"
#include "draw/goom_draw.h"
#include "fx_helper.h"
#include "goom/spimpl.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "visual_fx/circles/circles.h"

#include <array>
#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX
{

using CIRCLES::Circle;
using COLOR::IColorMap;
using DRAW::IGoomDraw;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::Fraction;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::U_HALF;

class CirclesFx::CirclesFxImpl
{
public:
  CirclesFxImpl(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps);

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;

  void SetZoomMidpoint(const Point2dInt& zoomMidpoint);

  void Start();
  void ApplyMultiple();

private:
  static constexpr uint32_t NUM_CIRCLES = 5;
  [[nodiscard]] static auto GetCircleParams(const PluginInfo& goomInfo)
      -> std::vector<Circle::Params>;
  [[nodiscard]] static auto GetCircleCentreTargets(const Point2dInt& screenMidPoint)
      -> std::array<Point2dInt, NUM_CIRCLES>;
  CIRCLES::Circles m_circles;
};

CirclesFx::CirclesFx(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept
  : m_pimpl{spimpl::make_unique_impl<CirclesFxImpl>(fxHelper, smallBitmaps)}
{
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
                                        const SmallImageBitmaps& smallBitmaps)
  : m_circles{fxHelper, smallBitmaps, NUM_CIRCLES, GetCircleParams(fxHelper.GetGoomInfo())}
{
}

auto CirclesFx::CirclesFxImpl::GetCircleParams(const PluginInfo& goomInfo)
    -> std::vector<Circle::Params>
{
  static_assert(5 == NUM_CIRCLES);

  std::vector<Circle::Params> circleParams(NUM_CIRCLES);

  static constexpr float RADIUS_MARGIN = 10.0F;
  const float maxRadius = 0.5F * static_cast<float>(std::min(goomInfo.GetScreenInfo().width,
                                                             goomInfo.GetScreenInfo().height));
  static constexpr float RADIUS_REDUCER = 1.0F;
  const float radius0 = maxRadius - RADIUS_MARGIN;

  circleParams[0].circleRadius = radius0;
  for (size_t i = 1; i < NUM_CIRCLES; ++i)
  {
    circleParams[i].circleRadius = RADIUS_REDUCER * circleParams[i - 1].circleRadius;
  }

  const Point2dInt screenMidPoint =
      MidpointFromOrigin({goomInfo.GetScreenInfo().width, goomInfo.GetScreenInfo().height});
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
  circleCentreTargets[0] = screenMidPoint;
  circleCentreTargets[1] = {SMALL_FRAC * width, SMALL_FRAC * height};
  circleCentreTargets[2] = {LARGE_FRAC * width, SMALL_FRAC * height};
  circleCentreTargets[3] = {LARGE_FRAC * width, LARGE_FRAC * height};
  circleCentreTargets[4] = {SMALL_FRAC * width, LARGE_FRAC * height};

  return circleCentreTargets;
}

inline auto CirclesFx::CirclesFxImpl::SetWeightedColorMaps(
    const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  Expects(weightedColorMaps.mainColorMaps != nullptr);
  Expects(weightedColorMaps.lowColorMaps != nullptr);

  m_circles.SetWeightedColorMaps(weightedColorMaps.mainColorMaps, weightedColorMaps.lowColorMaps);
}

inline void CirclesFx::CirclesFxImpl::SetZoomMidpoint(const Point2dInt& zoomMidpoint)
{
  m_circles.SetZoomMidpoint(zoomMidpoint);
}

inline void CirclesFx::CirclesFxImpl::Start()
{
  m_circles.Start();
}

inline void CirclesFx::CirclesFxImpl::ApplyMultiple()
{
  m_circles.UpdateAndDraw();
}

} // namespace GOOM::VISUAL_FX
