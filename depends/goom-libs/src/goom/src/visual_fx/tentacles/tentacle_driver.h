#pragma once

#include "color/random_colormaps.h"
#include "goom_graphic.h"
#include "tentacle3d.h"
#include "tentacle_layout.h"
#include "tentacle_plotter.h"
#include "utils/mathutils.h"

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
class SmallImageBitmaps;
}

namespace VISUAL_FX::TENTACLES
{

class TentacleDriver
{
public:
  TentacleDriver() noexcept = delete;
  TentacleDriver(DRAW::IGoomDraw& draw,
                 const UTILS::IGoomRand& goomRand,
                 const UTILS::SmallImageBitmaps& smallBitmaps,
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
  const UTILS::IGoomRand& m_goomRand;
  const ITentacleLayout& m_tentacleLayout;

  struct IterationParams
  {
    size_t numNodes = 200;
    float prevYWeight = 0.770F;
    float iterZeroYValWaveFreq = 1.0F;
    UTILS::SineWaveMultiplier iterZeroYValWave{};
    float length = 50.0F;
  };
  struct IterParamsGroup
  {
    const UTILS::IGoomRand& goomRand;
    IterationParams first{};
    IterationParams last{};
    [[nodiscard]] auto GetNextIterationParams(float t) const -> IterationParams;
  };
  const std::vector<IterParamsGroup> m_iterParamsGroups;
  std::vector<IterationParams> m_tentacleParams;
  [[nodiscard]] static auto GetTentacleParams(size_t numTentacles,
                                              const std::vector<IterParamsGroup>& iterParamsGroups)
      -> std::vector<IterationParams>;

  std::vector<Tentacle3D> m_tentacles;
  [[nodiscard]] static auto GetTentacles(const UTILS::IGoomRand& goomRand,
                                         const std::vector<IterationParams>& tentacleParams,
                                         const ITentacleLayout& tentacleLayout)
      -> std::vector<Tentacle3D>;
  [[nodiscard]] static auto CreateNewTentacle2D(const UTILS::IGoomRand& goomRand,
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
