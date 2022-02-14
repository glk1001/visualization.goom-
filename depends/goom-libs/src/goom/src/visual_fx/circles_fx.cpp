#include "circles_fx.h"

#include "color/colormaps.h"
#include "draw/goom_draw.h"
#include "fx_helper.h"
#include "goom/spimpl.h"
#include "utils/goom_rand_base.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "visual_fx/circles/circles.h"

#include <memory>

namespace GOOM::VISUAL_FX
{

using CIRCLES::Circle;
using COLOR::IColorMap;
using DRAW::IGoomDraw;
using UTILS::GetHalf;
using UTILS::GetQuarter;
using UTILS::GetThreeQuarters;
using UTILS::IGoomRand;
using UTILS::GRAPHICS::SmallImageBitmaps;

class CirclesFx::CirclesFxImpl
{
public:
  CirclesFxImpl(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps);

  void SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps,
                            std::shared_ptr<COLOR::RandomColorMaps> weightedLowMaps);

  void SetZoomMidpoint(const Point2dInt& zoomMidpoint);

  void Start();
  void ApplyMultiple();

private:
  static constexpr uint32_t NUM_CIRCLES = 5;
  [[nodiscard]] static auto GetCircleParams(const PluginInfo& goomInfo)
      -> std::vector<Circle::Params>;
  CIRCLES::Circles m_circles;
};

CirclesFx::CirclesFx(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept
  : m_fxImpl{spimpl::make_unique_impl<CirclesFxImpl>(fxHelper, smallBitmaps)}
{
}

void CirclesFx::SetWeightedColorMaps(const std::shared_ptr<COLOR::RandomColorMaps> weightedMaps,
                                     const std::shared_ptr<COLOR::RandomColorMaps> weightedLowMaps)
{
  m_fxImpl->SetWeightedColorMaps(weightedMaps, weightedLowMaps);
}

void CirclesFx::SetZoomMidpoint(const Point2dInt& zoomMidpoint)
{
  m_fxImpl->SetZoomMidpoint(zoomMidpoint);
}

void CirclesFx::Start()
{
  m_fxImpl->Start();
}

void CirclesFx::Finish()
{
  // nothing to do
}

auto CirclesFx::GetFxName() const -> std::string
{
  return "circles";
}

void CirclesFx::ApplyMultiple()
{
  m_fxImpl->ApplyMultiple();
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

  constexpr float RADIUS_MARGIN = 10.0F;
  const float maxRadius = 0.5F * static_cast<float>(std::min(goomInfo.GetScreenInfo().width,
                                                             goomInfo.GetScreenInfo().height));
  constexpr float RADIUS_REDUCER = 0.95F;
  const float radius0 = maxRadius - RADIUS_MARGIN;

  circleParams[0].circleRadius = radius0;
  circleParams[1].circleRadius = RADIUS_REDUCER * circleParams[0].circleRadius;
  circleParams[2].circleRadius = RADIUS_REDUCER * circleParams[1].circleRadius;
  circleParams[3].circleRadius = RADIUS_REDUCER * circleParams[2].circleRadius;
  circleParams[4].circleRadius = RADIUS_REDUCER * circleParams[3].circleRadius;

  circleParams[0].circleCentreTarget = {1 * goomInfo.GetScreenInfo().width / 2,
                                        1 * goomInfo.GetScreenInfo().height / 2};
  circleParams[1].circleCentreTarget = {1 * goomInfo.GetScreenInfo().width / 10,
                                        1 * goomInfo.GetScreenInfo().height / 10};
  circleParams[2].circleCentreTarget = {9 * goomInfo.GetScreenInfo().width / 10,
                                        1 * goomInfo.GetScreenInfo().height / 10};
  circleParams[3].circleCentreTarget = {9 * goomInfo.GetScreenInfo().width / 10,
                                        9 * goomInfo.GetScreenInfo().height / 10};
  circleParams[4].circleCentreTarget = {1 * goomInfo.GetScreenInfo().width / 10,
                                        9 * goomInfo.GetScreenInfo().height / 10};

  return circleParams;
}

inline void CirclesFx::CirclesFxImpl::SetWeightedColorMaps(
    const std::shared_ptr<COLOR::RandomColorMaps> weightedMaps,
    const std::shared_ptr<COLOR::RandomColorMaps> weightedLowMaps)
{
  m_circles.SetWeightedColorMaps(weightedMaps, weightedLowMaps);
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
