#pragma once

#include "color/colormaps.h"
#include "color/random_colormaps.h"
#include "goom_graphic.h"
#include "tentacle3d.h"
#include "tentacle_colorizer.h"
#include "tentacle_layout.h"
#include "tentacle_plotter.h"
#include "utils/mathutils.h"

#include <cstdint>
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
} // namespace UTILS

namespace VISUAL_FX
{
namespace TENTACLES
{

class TentacleDriver
{
public:
  enum class ColorModes
  {
    MINIMAL,
    ONE_GROUP_FOR_ALL,
    MULTI_GROUPS,
  };

  TentacleDriver() noexcept = delete;
  TentacleDriver(DRAW::IGoomDraw& draw,
                 UTILS::IGoomRand& goomRand,
                 const UTILS::SmallImageBitmaps& smallBitmaps,
                 COLOR::ColorMapGroup initialColorMapGroup,
                 const ITentacleLayout& tentacleLayout) noexcept;

  void SetColorMode(ColorModes colorMode);
  void SetWeightedColorMaps(const std::shared_ptr<COLOR::RandomColorMaps>& weightedMaps);

  void StartIterating();

  void FreshStart();
  void SetReverseColorMix(bool value);
  void MultiplyIterZeroYValWaveFreq(float value);
  void SetProjectionDistance(float value);
  void SetCameraPosition(float cameraDistance, float tentacleAngle);
  void SetDominantColors(const Pixel& dominantColor, const Pixel& dominantLowColor);

  void Update();

private:
  DRAW::IGoomDraw& m_draw;
  UTILS::IGoomRand& m_goomRand;
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
    UTILS::IGoomRand& goomRand;
    IterationParams first{};
    IterationParams last{};
    [[nodiscard]] auto GetNextIterationParams(float t) const -> IterationParams;
  };
  const std::vector<IterParamsGroup> m_iterParamsGroups;
  std::vector<IterationParams> m_tentacleParams;
  [[nodiscard]] static auto GetTentacleParams(size_t numTentacles,
                                              const std::vector<IterParamsGroup>& iterParamsGroups)
      -> std::vector<IterationParams>;

  std::shared_ptr<COLOR::RandomColorMaps> m_colorMaps{};
  std::vector<std::shared_ptr<ITentacleColorizer>> m_colorizers;
  [[nodiscard]] static auto GetColorizers(UTILS::IGoomRand& goomRand,
                                          const std::vector<IterationParams>& tentacleParams,
                                          COLOR::ColorMapGroup initialColorMapGroup)
      -> std::vector<std::shared_ptr<ITentacleColorizer>>;

  std::vector<Tentacle3D> m_tentacles;
  [[nodiscard]] static auto GetTentacles(
      UTILS::IGoomRand& goomRand,
      const std::vector<IterationParams>& tentacleParams,
      const ITentacleLayout& tentacleLayout,
      const std::vector<std::shared_ptr<ITentacleColorizer>>& colorizers)
      -> std::vector<Tentacle3D>;
  [[nodiscard]] static auto CreateNewTentacle2D(UTILS::IGoomRand& goomRand,
                                                size_t id,
                                                const IterationParams& params)
      -> std::unique_ptr<Tentacle2D>;
  static void UpdateTentaclesLayout(std::vector<Tentacle3D>& tentacles,
                                    const ITentacleLayout& tentacleLayout);

  ColorModes m_colorMode = ColorModes::ONE_GROUP_FOR_ALL;
  static const size_t CHANGE_CURRENT_COLOR_MAP_GROUP_EVERY_N_UPDATES;
  [[nodiscard]] auto GetNextColorMapGroups() const -> std::vector<COLOR::ColorMapGroup>;
  void TentaclesColorMapsChanged();

  size_t m_updateNum = 0;
  void CheckForTimerEvents();
  [[nodiscard]] auto ChangeCurrentColorMapEvent() const -> bool;

  TentaclePlotter m_tentaclePlotter;
};

class TentacleColorMapColorizer : public ITentacleColorizer
{
public:
  TentacleColorMapColorizer(COLOR::ColorMapGroup cmg,
                            size_t numNodes,
                            UTILS::IGoomRand& goomRand) noexcept;
  TentacleColorMapColorizer(const TentacleColorMapColorizer&) noexcept = delete;
  TentacleColorMapColorizer(TentacleColorMapColorizer&&) noexcept = delete;
  ~TentacleColorMapColorizer() noexcept override = default;
  auto operator=(const TentacleColorMapColorizer&) -> TentacleColorMapColorizer& = delete;
  auto operator=(TentacleColorMapColorizer&&) -> TentacleColorMapColorizer& = delete;

  void SetColorMapGroup(COLOR::ColorMapGroup colorMapGroup) override;
  void ChangeColorMap() override;
  auto GetColor(size_t nodeNum) const -> Pixel override;

private:
  size_t m_numNodes = 0;
  COLOR::ColorMapGroup m_currentColorMapGroup{};
  const COLOR::RandomColorMaps m_colorMaps;
  std::shared_ptr<const COLOR::IColorMap> m_colorMap{};
  std::shared_ptr<const COLOR::IColorMap> m_prevColorMap{};
  static constexpr uint32_t MAX_COUNT_SINCE_COLORMAP_CHANGE = 100;
  static constexpr float TRANSITION_STEP =
      1.0F / static_cast<float>(MAX_COUNT_SINCE_COLORMAP_CHANGE);
  mutable float m_tTransition = 0.0F;
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

} // namespace TENTACLES
} // namespace VISUAL_FX
} // namespace GOOM
