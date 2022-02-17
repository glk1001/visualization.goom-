#pragma once

#include "color/random_colormaps.h"
#include "goom_graphic.h"
#include "tentacle3d.h"
#include "tentacle_layout.h"
#include "tentacle_plotter.h"
#include "utils/math/mathutils.h"

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

  void SetWeightedColorMaps(const std::shared_ptr<COLOR::RandomColorMaps>& weightedMaps);
  void TentaclesColorMapsChanged();

  void StartIterating();

  void SetReverseColorMix(bool value);
  void MultiplyIterZeroYValWaveFreq(float value);
  void SetProjectionDistance(float value);
  void SetCameraPosition(float cameraDistance, float tentacleAngle);
  void SetDominantColors(const Pixel& dominantColor, const Pixel& dominantLowColor);

  void Update();

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
  static void UpdateTentaclesLayout(std::vector<Tentacle3D>& tentacles,
                                    const ITentacleLayout& tentacleLayout);

  size_t m_updateNum = 0;
  static const size_t CHANGE_CURRENT_COLOR_MAP_GROUP_EVERY_N_UPDATES;
  void CheckForTimerEvents();

  TentaclePlotter m_tentaclePlotter;
};

inline void TentacleDriver::SetProjectionDistance(const float value)
{
  m_tentaclePlotter.SetProjectionDistance(value);
}

inline void TentacleDriver::SetCameraPosition(const float cameraDistance, const float tentacleAngle)
{
  m_tentaclePlotter.SetCameraPosition(cameraDistance, tentacleAngle);
}

inline void TentacleDriver::SetDominantColors(const Pixel& dominantColor,
                                              const Pixel& dominantLowColor)
{
  m_tentaclePlotter.SetDominantColors(dominantColor, dominantLowColor);
}

} // namespace VISUAL_FX::TENTACLES
} // namespace GOOM
