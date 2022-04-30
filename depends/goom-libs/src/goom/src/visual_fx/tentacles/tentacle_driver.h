#pragma once

#include "color/random_colormaps.h"
#include "goom_graphic.h"
#include "tentacle3d.h"
#include "tentacle_layout.h"
#include "tentacle_plotter.h"
#include "utils/math/misc.h"

#include <memory>
#include <vector>

namespace GOOM
{

namespace DRAW
{
class IGoomDraw;
}

namespace UTILS
{
class IGoomRand;
}

namespace UTILS::GRAPHICS
{
class SmallImageBitmaps;
}

namespace VISUAL_FX::TENTACLES
{

class TentacleDriver
{
public:
  TentacleDriver() noexcept = delete;
  TentacleDriver(DRAW::IGoomDraw& draw,
                 const UTILS::MATH::IGoomRand& goomRand,
                 const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps,
                 const ITentacleLayout& tentacleLayout) noexcept;

  auto SetWeightedColorMaps(const std::shared_ptr<COLOR::RandomColorMaps>& weightedMaps) -> void;
  auto TentaclesColorMapsChanged() -> void;

  auto StartIterating() -> void;

  auto SetReverseColorMix(bool value) -> void;
  auto MultiplyIterZeroYValWaveFreq(float value) -> void;
  auto SetProjectionDistance(float value) -> void;
  auto SetCameraPosition(float cameraDistance, float tentacleAngle) -> void;
  auto SetDominantColors(const Pixel& dominantMainColor,
                         const Pixel& dominantLowColor,
                         const Pixel& dominantDotColor) -> void;

  auto Update() -> void;

private:
  DRAW::IGoomDraw& m_draw;
  const UTILS::MATH::IGoomRand& m_goomRand;
  const ITentacleLayout& m_tentacleLayout;

  struct IterationParams
  {
    size_t numNodes;
    float prevYWeight;
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
  std::vector<IterationParams> m_tentacleParams;
  [[nodiscard]] static auto GetTentacleParams(size_t numTentacles,
                                              const std::vector<IterParamsGroup>& iterParamsGroups)
      -> std::vector<IterationParams>;

  std::vector<Tentacle3D> m_tentacles;
  [[nodiscard]] static auto GetTentacles(const UTILS::MATH::IGoomRand& goomRand,
                                         const std::vector<IterationParams>& tentacleParams,
                                         const ITentacleLayout& tentacleLayout)
      -> std::vector<Tentacle3D>;
  [[nodiscard]] static auto CreateNewTentacle2D(const UTILS::MATH::IGoomRand& goomRand,
                                                size_t id,
                                                const IterationParams& params)
      -> std::unique_ptr<Tentacle2D>;
  static auto UpdateTentaclesLayout(std::vector<Tentacle3D>& tentacles,
                                    const ITentacleLayout& tentacleLayout) -> void;

  size_t m_updateNum = 0;
  static const size_t CHANGE_CURRENT_COLOR_MAP_GROUP_EVERY_N_UPDATES;
  auto CheckForTimerEvents() -> void;

  TentaclePlotter m_tentaclePlotter;
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

inline auto TentacleDriver::SetDominantColors(const Pixel& dominantMainColor,
                                              const Pixel& dominantLowColor,
                                              const Pixel& dominantDotColor) -> void
{
  m_tentaclePlotter.SetDominantColors(dominantMainColor, dominantLowColor, dominantDotColor);
}

} // namespace VISUAL_FX::TENTACLES
} // namespace GOOM
