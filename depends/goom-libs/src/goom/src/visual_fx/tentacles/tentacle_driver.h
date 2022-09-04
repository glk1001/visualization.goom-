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
                 const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps,
                 const CirclesTentacleLayout& tentacleLayout) noexcept;

  auto SetWeightedColorMaps(const IVisualFx::WeightedColorMaps& weightedColorMaps) noexcept -> void;
  auto ChangeTentacleColorMaps() -> void;

  auto StartIterating() -> void;

  auto SetAllTentaclesStartCentrePos(const Point2dInt& val) noexcept -> void;
  auto SetAllTentaclesEndCentrePos(const Point2dInt& val) noexcept -> void;

  auto MultiplyIterZeroYValWaveFreq(float value) -> void;
  auto SetDominantColorMaps(const std::shared_ptr<const COLOR::IColorMap>& dominantMainColorMap,
                            const std::shared_ptr<const COLOR::IColorMap>& dominantLowColorMap,
                            const std::shared_ptr<const COLOR::IColorMap>& dominantDotColorMap)
      -> void;

  auto Update() -> void;

private:
  DRAW::IGoomDraw& m_draw;
  const UTILS::MATH::IGoomRand& m_goomRand;
  const Point2dInt m_screenMidpoint =
      MidpointFromOrigin({m_draw.GetScreenWidth(), m_draw.GetScreenHeight()});
  const CirclesTentacleLayout& m_tentacleLayout;

  std::shared_ptr<const COLOR::IColorMap> m_dominantMainColorMap{};
  std::shared_ptr<const COLOR::IColorMap> m_dominantLowColorMap{};
  std::shared_ptr<const COLOR::IColorMap> m_dominantDotColorMap{};

  struct IterationParams
  {
    uint32_t numNodes;
    float length;
    float iterZeroYValWaveFreq;
    UTILS::MATH::SineWaveMultiplier iterZeroYValWave;
  };
  IterationParams m_tentacleParams;

  std::vector<Tentacle3D> m_tentacles;
  [[nodiscard]] static auto GetTentacles(const UTILS::MATH::IGoomRand& goomRand,
                                         const IterationParams& tentacleParams,
                                         const CirclesTentacleLayout& tentacleLayout)
      -> std::vector<Tentacle3D>;
  [[nodiscard]] static auto CreateNewTentacle2D(const IterationParams& params)
      -> std::unique_ptr<Tentacle2D>;

  size_t m_updateNum                                                     = 0;
  static constexpr size_t CHANGE_CURRENT_COLOR_MAP_GROUP_EVERY_N_UPDATES = 400U;
  auto CheckForTimerEvents() -> void;

  TentaclePlotter m_tentaclePlotter;
  auto UpdateTentaclePlotter() -> void;
};

inline auto TentacleDriver::SetDominantColorMaps(
    const std::shared_ptr<const COLOR::IColorMap>& dominantMainColorMap,
    const std::shared_ptr<const COLOR::IColorMap>& dominantLowColorMap,
    const std::shared_ptr<const COLOR::IColorMap>& dominantDotColorMap) -> void
{
  m_dominantMainColorMap = dominantMainColorMap;
  m_dominantLowColorMap  = dominantLowColorMap;
  m_dominantDotColorMap  = dominantDotColorMap;
}

} // namespace GOOM::VISUAL_FX::TENTACLES
