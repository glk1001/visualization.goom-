#pragma once

#include "../fx_utils/dot_drawer.h"
#include "color/colormaps.h"
#include "color/random_colormaps.h"
#include "goom_graphic.h"
#include "tentacle_colorizer.h"
#include "tentacle_layout.h"
#include "tentacles3d.h"
#include "utils/mathutils.h"
#include "v2d.h"

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
                 const UTILS::SmallImageBitmaps& smallBitmaps) noexcept;

  void Init(COLOR::ColorMapGroup initialColorMapGroup, const ITentacleLayout& layout);

  void SetColorMode(ColorModes colorMode);

  void SetWeightedColorMaps(const std::shared_ptr<COLOR::RandomColorMaps>& weightedMaps);

  void StartIterating();

  void FreshStart();
  void SetReverseColorMix(bool value);
  void MultiplyIterZeroYValWaveFreq(float value);
  void SetTentacleAngle(float value);
  void SetProjectionDistance(float value);
  void SetCameraDistance(float value);
  void SetDominantColors(const Pixel& dominantColor, const Pixel& dominantLowColor);

  void Update();

private:
  DRAW::IGoomDraw& m_draw;
  UTILS::IGoomRand& m_goomRand;
  const int32_t m_halfScreenWidth;
  const int32_t m_halfScreenHeight;
  Tentacles3D m_tentacles{};
  ColorModes m_colorMode = ColorModes::ONE_GROUP_FOR_ALL;
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
    [[nodiscard]] auto GetNext(float t) const -> IterationParams;
  };
  const std::vector<IterParamsGroup> m_iterParamsGroups;

  void UpdateTentaclesLayout(const ITentacleLayout& layout);

  std::shared_ptr<COLOR::RandomColorMaps> m_colorMaps{};
  std::vector<std::shared_ptr<ITentacleColorizer>> m_colorizers{};

  size_t m_updateNum = 0;
  size_t m_numTentacles = 0;
  std::vector<IterationParams> m_tentacleParams{};
  static const size_t CHANGE_CURRENT_COLOR_MAP_GROUP_EVERY_N_UPDATES;
  [[nodiscard]] auto GetNextColorMapGroups() const -> std::vector<COLOR::ColorMapGroup>;

  void AddTentacle(size_t id, const IterationParams& params);
  auto CreateNewTentacle2D(size_t id, const IterationParams& params) -> std::unique_ptr<Tentacle2D>;
  void AddColorizer(COLOR::ColorMapGroup initialColorMapGroup, const IterationParams& params);

  void CheckForTimerEvents();
  [[nodiscard]] auto ChangeCurrentColorMapEvent() const -> bool;

  float m_tentacleAngle = 0.0F;
  float m_projectionDistance = 0.0F;
  float m_cameraDistance = 0.0F;
  Pixel m_dominantColor{};
  Pixel m_dominantLowColor{};
  FX_UTILS::DotDrawer m_dotDrawer;
  static constexpr uint32_t MIN_STEPS_BETWEEN_NODES = 1;
  static constexpr uint32_t MAX_STEPS_BETWEEN_NODES = 6;
  uint32_t m_numNodesBetweenDots = (MIN_STEPS_BETWEEN_NODES + MAX_STEPS_BETWEEN_NODES) / 2;

  void Plot3D(const Tentacle3D& tentacle);
  void DrawNode(const Tentacle3D& tentacle,
                size_t nodeNum,
                int32_t x0,
                int32_t y0,
                int32_t x1,
                int32_t y1,
                float brightness);
  void DrawNodeLine(
      int32_t x0, int32_t y0, int32_t x1, int32_t y1, const std::vector<Pixel>& colors);
  void DrawNodeDot(size_t nodeNum, int32_t x, int32_t y, const std::vector<Pixel>& colors);

  [[nodiscard]] auto GetMixedColors(const Tentacle3D& tentacle,
                                    float brightness,
                                    size_t nodeNum) const -> std::vector<Pixel>;
  [[nodiscard]] auto GetBrightness(const Tentacle3D& tentacle) const -> float;
  [[nodiscard]] auto GetBrightnessCut(const Tentacle3D& tentacle) const -> float;
  [[nodiscard]] auto GetCameraPosition() const -> V3dFlt;
  [[nodiscard]] auto Get2DTentaclePoints(const Tentacle3D& tentacle) const -> std::vector<V2dInt>;
  [[nodiscard]] auto GetTentacleAngleAboutY(const Tentacle3D& tentacle) const -> float;
  [[nodiscard]] static auto GetTransformedPoints(const std::vector<V3dFlt>& points,
                                                 const V3dFlt& translate,
                                                 float angle) -> std::vector<V3dFlt>;
  [[nodiscard]] auto GetPerspectiveProjection(const std::vector<V3dFlt>& points3D) const
      -> std::vector<V2dInt>;
  static void RotateAboutYAxis(float sinAngle, float cosAngle, const V3dFlt& vSrc, V3dFlt& vDest);
  static void Translate(const V3dFlt& vAdd, V3dFlt& vInOut);
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

inline void TentacleDriver::SetTentacleAngle(const float value)
{
  m_tentacleAngle = value;
}

inline void TentacleDriver::SetProjectionDistance(const float value)
{
  m_projectionDistance = value;
}

inline void TentacleDriver::SetCameraDistance(const float value)
{
  m_cameraDistance = value;
}

inline void TentacleDriver::SetDominantColors(const Pixel& dominantColor,
                                              const Pixel& dominantLowColor)
{
  m_dominantColor = dominantColor;
  m_dominantLowColor = dominantLowColor;
}

} // namespace TENTACLES
} // namespace VISUAL_FX
} // namespace GOOM
