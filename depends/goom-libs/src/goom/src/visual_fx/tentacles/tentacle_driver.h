#pragma once

#include "../goom_visual_fx.h"
#include "color/random_color_maps.h"
#include "draw/goom_draw.h"
#include "goom_graphic.h"
#include "point2d.h"
#include "tentacle3d.h"
#include "tentacle_layout.h"
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
                 const ITentacleLayout& tentacleLayout) noexcept;

  auto SetWeightedColorMaps(const IVisualFx::WeightedColorMaps& weightedColorMaps) noexcept -> void;
  auto ChangeTentacleColorMaps() -> void;

  auto StartIterating() -> void;

  auto SetAllTentaclesStartCentrePos(const Point2dInt& val) noexcept -> void;
  auto SetAllTentaclesEndCentrePos(const Point2dInt& val) noexcept -> void;

  auto MultiplyIterZeroYValWaveFreq(float value) -> void;
  auto SetProjectionDistance(float value) -> void;
  auto SetCameraPosition(float cameraDistance, float tentacleAngle) -> void;
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
  const ITentacleLayout& m_tentacleLayout;

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
  struct IterParamsGroup
  {
    const UTILS::MATH::IGoomRand& goomRand;
    IterationParams first;
    IterationParams last;
    [[nodiscard]] auto GetNextIterationParams(float t) const -> IterationParams;
  };
  static const IterationParams ITER_PARAM_GROUP1_FIRST;
  static const IterationParams ITER_PARAM_GROUP1_LAST;
  static const IterationParams ITER_PARAM_GROUP2_FIRST;
  static const IterationParams ITER_PARAM_GROUP2_LAST;
  static const IterationParams ITER_PARAM_GROUP3_FIRST;
  static const IterationParams ITER_PARAM_GROUP3_LAST;
  const std::vector<IterParamsGroup> m_iterParamsGroups;
  std::vector<IterationParams> m_tentacleParams{
      GetTentacleParams(m_tentacleLayout.GetNumPoints(), m_iterParamsGroups)};
  [[nodiscard]] static auto GetTentacleParams(size_t numTentacles,
                                              const std::vector<IterParamsGroup>& iterParamsGroups)
      -> std::vector<IterationParams>;

  std::vector<Tentacle3D> m_tentacles = GetTentacles(m_goomRand, m_tentacleParams);
  const std::vector<size_t> m_sortedIndexesLongestFirst =
      GetSortedIndexesLongestTentaclesFirst(m_tentacles);
  [[nodiscard]] static auto GetSortedIndexesLongestTentaclesFirst(
      const std::vector<Tentacle3D>& tentacles) -> std::vector<size_t>;
  [[nodiscard]] static auto GetTentacles(const UTILS::MATH::IGoomRand& goomRand,
                                         const std::vector<IterationParams>& tentacleParams)
      -> std::vector<Tentacle3D>;
  [[nodiscard]] static auto CreateNewTentacle2D(const UTILS::MATH::IGoomRand& goomRand,
                                                const IterationParams& params)
      -> std::unique_ptr<Tentacle2D>;
  auto UpdateTentaclePositions(std::vector<Tentacle3D>& tentacles,
                               const ITentacleLayout& tentacleLayout) const noexcept -> void;
  [[nodiscard]] static auto GetStartPos(const V3dFlt& tentacleLayoutStartPos) -> V3dFlt;
  static auto ShortenTentacleIfNearVerticalCentre(Tentacle3D& tentacle) -> void;

  size_t m_updateNum                                                     = 0;
  static constexpr size_t CHANGE_CURRENT_COLOR_MAP_GROUP_EVERY_N_UPDATES = 400U;
  auto CheckForTimerEvents() -> void;

  TentaclePlotter m_tentaclePlotter;
  auto UpdateTentaclePlotter() -> void;
};

inline auto TentacleDriver::SetProjectionDistance(const float value) -> void
{
  m_tentaclePlotter.SetProjectionDistance(value);
}

inline auto TentacleDriver::SetCameraPosition(const float cameraDistance, const float tentacleAngle)
    -> void
{
  m_tentaclePlotter.SetCameraPosition(cameraDistance, tentacleAngle);
}

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
