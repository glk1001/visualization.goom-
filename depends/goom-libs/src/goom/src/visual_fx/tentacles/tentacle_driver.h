#pragma once

#include "../goom_visual_fx.h"
#include "circles_tentacle_layout.h"
#include "color/random_color_maps.h"
#include "draw/goom_draw.h"
#include "goom_graphic.h"
#include "point2d.h"
#include "tentacle3d.h"
#include "tentacle_plotter.h"
#include "utils/math/misc.h"

#include <memory>
#include <vector>

namespace GOOM
{

namespace UTILS::MATH
{
class IGoomRand;
}
namespace UTILS::GRAPHICS
{
class SmallImageBitmaps;
}

} // namespace GOOM

namespace GOOM::VISUAL_FX::TENTACLES
{

class TentacleDriver
{
public:
  TentacleDriver() noexcept = delete;
  TentacleDriver(DRAW::IGoomDraw& draw,
                 const UTILS::MATH::IGoomRand& goomRand,
                 const CirclesTentacleLayout& tentacleLayout) noexcept;

  auto SetWeightedColorMaps(const IVisualFx::WeightedColorMaps& weightedColorMaps) noexcept -> void;
  auto ChangeTentacleColorMaps() -> void;

  auto StartIterating() -> void;

  auto SetAllTentaclesEndCentrePos(const Point2dInt& val) noexcept -> void;
  [[nodiscard]] auto GetAcceptableCentrePosOffset(
      const Point2dInt& requestedCentrePos) const noexcept -> Point2dInt;
  [[nodiscard]] static auto GetNewRadiusCentrePosOffset(
      float radiusScale, const V3dFlt& endPosFlt, const Point2dInt& oldCentreEndPosOffset) noexcept
      -> V3dFlt;

  auto MultiplyIterZeroYValWaveFreq(float value) -> void;
  auto SetDominantColorMaps(const std::shared_ptr<const COLOR::IColorMap>& dominantMainColorMap,
                            const std::shared_ptr<const COLOR::IColorMap>& dominantLowColorMap)
      -> void;

  auto Update() -> void;

private:
  DRAW::IGoomDraw& m_draw;
  const UTILS::MATH::IGoomRand& m_goomRand;
  const Point2dInt m_screenMidpoint =
      MidpointFromOrigin({m_draw.GetScreenWidth(), m_draw.GetScreenHeight()});

  std::shared_ptr<const COLOR::IColorMap> m_dominantMainColorMap{};
  std::shared_ptr<const COLOR::IColorMap> m_dominantLowColorMap{};

  struct IterationParams
  {
    uint32_t numNodes;
    float length;
    float iterZeroYValWaveFreq;
    UTILS::MATH::SineWaveMultiplier iterZeroYValWave;
  };
  IterationParams m_tentacleParams;
  TentaclePlotter m_tentaclePlotter;

  std::vector<Tentacle3D> m_tentacles;
  [[nodiscard]] auto GetTentacles(const CirclesTentacleLayout& tentacleLayout) const noexcept
      -> std::vector<Tentacle3D>;
  [[nodiscard]] auto CreateNewTentacle2D() const noexcept -> std::unique_ptr<Tentacle2D>;
  uint32_t m_tentacleGroupSize                      = static_cast<uint32_t>(m_tentacles.size());
  static constexpr uint32_t MIN_TENTACLE_GROUP_SIZE = 10U;

  static constexpr float MIN_RADIUS_FACTOR = 0.5F;
  static constexpr float MAX_RADIUS_FACTOR = 1.0F;

  size_t m_updateNum                                                     = 0U;
  static constexpr size_t CHANGE_CURRENT_COLOR_MAP_GROUP_EVERY_N_UPDATES = 400U;
  auto CheckForTimerEvents() -> void;
};

inline auto TentacleDriver::SetDominantColorMaps(
    const std::shared_ptr<const COLOR::IColorMap>& dominantMainColorMap,
    const std::shared_ptr<const COLOR::IColorMap>& dominantLowColorMap) -> void
{
  m_dominantMainColorMap = dominantMainColorMap;
  m_dominantLowColorMap  = dominantLowColorMap;
}

} // namespace GOOM::VISUAL_FX::TENTACLES
