#pragma once

#include "bitmap_getter.h"
#include "circle.h"
#include "color/color_utils.h"
#include "color/random_color_maps.h"
#include "point2d.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "visual_fx/fx_helper.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX::CIRCLES
{

class Circles
{
public:
  Circles(const FxHelper& fxHelper,
          const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps,
          uint32_t numCircles,
          const std::vector<Circle::Params>& circleParams);

  auto SetWeightedColorMaps(std::shared_ptr<const COLOR::RandomColorMaps> weightedMaps,
                            std::shared_ptr<const COLOR::RandomColorMaps> weightedLowMaps) -> void;

  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;

  auto Start() -> void;
  auto UpdateAndDraw() -> void;

private:
  const UTILS::MATH::IGoomRand& m_goomRand;
  const PluginInfo& m_goomInfo;
  BitmapGetter m_bitmapGetter;
  Point2dInt m_zoomMidpoint{};
  auto SetNewTargetPoints() -> void;

  const uint32_t m_numCircles;
  std::vector<Circle> m_circles;
  [[nodiscard]] static auto GetCircles(const FxHelper& fxHelper,
                                       const Circle::Helper& helper,
                                       const UTILS::MATH::OscillatingFunction::Params& pathParams,
                                       uint32_t numCircles,
                                       const std::vector<Circle::Params>& circleParams)
      -> std::vector<Circle>;
  auto UpdatePositionSpeed() -> void;
  auto UpdateAndDrawCircles() -> void;

  auto UpdateCirclePathParams() -> void;
  [[nodiscard]] auto GetCentreCircleTargetPoint() const noexcept -> Point2dInt;
  [[nodiscard]] auto GetPathParams() const -> UTILS::MATH::OscillatingFunction::Params;
};

inline auto Circles::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  m_zoomMidpoint = zoomMidpoint;
}

} // namespace GOOM::VISUAL_FX::CIRCLES
