#pragma once

#include "color/colormaps.h"
#include "color/random_colormaps.h"
#include "goom_graphic.h"
#include "tentacles.h"
#include "utils/goom_rand_base.h"
#include "utils/graphics/image_bitmaps.h"
#include "utils/graphics/small_image_bitmaps.h"
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
} // namespace DRAW

namespace TENTACLES
{

class IterTimer
{
public:
  explicit IterTimer(const size_t startCnt) : m_startCount{startCnt} {}

  void Start() { m_count = m_startCount; }
  void Next()
  {
    if (m_count > 0)
    {
      --m_count;
    }
  }

private:
  const size_t m_startCount;
  size_t m_count = 0;
};

class ITentacleLayout
{
public:
  ITentacleLayout() noexcept = default;
  ITentacleLayout(const ITentacleLayout&) noexcept = default;
  ITentacleLayout(ITentacleLayout&&) noexcept = delete;
  virtual ~ITentacleLayout() noexcept = default;
  auto operator=(const ITentacleLayout&) -> ITentacleLayout& = delete;
  auto operator=(ITentacleLayout&&) -> ITentacleLayout& = delete;

  [[nodiscard]] virtual auto GetNumPoints() const -> size_t = 0;
  [[nodiscard]] virtual auto GetPoints() const -> const std::vector<V3dFlt>& = 0;
};

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

  void Init(COLOR::ColorMapGroup initialColorMapGroup, const ITentacleLayout& l);

  void SetColorMode(ColorModes m);

  void SetWeightedColorMaps(const std::shared_ptr<COLOR::RandomColorMaps>& weightedMaps);

  void StartIterating();

  void FreshStart();
  void Update(
      float angle, float distance, float distance2, const Pixel& color, const Pixel& lowColor);

  void SetReverseColorMix(bool val);
  void MultiplyIterZeroYValWaveFreq(float val);

private:
  DRAW::IGoomDraw& m_draw;
  UTILS::IGoomRand& m_goomRand;
  Tentacles3D m_tentacles;
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

  auto CreateNewTentacle2D(size_t id, const IterationParams& iterationParams)
      -> std::unique_ptr<Tentacle2D>;
  const std::vector<IterTimer*> m_iterTimers{};
  void UpdateIterTimers();
  void CheckForTimerEvents();
  [[nodiscard]] auto ChangeCurrentColorMapEvent() const -> bool;
  void Plot3D(const Tentacle3D& tentacle,
              const Pixel& dominantColor,
              const Pixel& dominantLowColor,
              float angle,
              float distance,
              float distance2) const;
  static constexpr size_t MIN_IMAGE_DOT_SIZE = 3;
  static constexpr size_t MAX_IMAGE_DOT_SIZE = 15;
  static_assert(MAX_IMAGE_DOT_SIZE <= UTILS::SmallImageBitmaps::MAX_IMAGE_SIZE,
                "Max dot size mismatch.");
  size_t m_currentDotSize = MIN_IMAGE_DOT_SIZE;
  bool m_beadedLook = false;
  static constexpr uint32_t MIN_STEPS_BETWEEN_NODES = 1;
  static constexpr uint32_t MAX_STEPS_BETWEEN_NODES = 6;
  uint32_t m_numNodesBetweenDots = (MIN_STEPS_BETWEEN_NODES + MAX_STEPS_BETWEEN_NODES) / 2;
  const UTILS::Weights<size_t> m_dotSizesMin;
  const UTILS::Weights<size_t> m_dotSizes;
  const UTILS::SmallImageBitmaps& m_smallBitmaps;
  [[nodiscard]] auto GetNextDotSize(size_t maxSize) const -> size_t;
  [[nodiscard]] auto GetImageBitmap(size_t size) const -> const UTILS::ImageBitmap&;
  void DrawDots(const V2dInt& pt, const std::vector<Pixel>& colors) const;
  [[nodiscard]] auto ProjectV3DOntoV2D(const std::vector<V3dFlt>& v3, float distance) const
      -> std::vector<V2dInt>;
  static void RotateV3DAboutYAxis(float sinAngle,
                                  float cosAngle,
                                  const V3dFlt& vSrc,
                                  V3dFlt& vDest);
  static void TranslateV3D(const V3dFlt& vAdd, V3dFlt& vInOut);
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

  auto GetColorMapGroup() const -> COLOR::ColorMapGroup override;
  void SetColorMapGroup(COLOR::ColorMapGroup c) override;
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

class CirclesTentacleLayout : public ITentacleLayout
{
public:
  CirclesTentacleLayout(float radiusMin,
                        float radiusMax,
                        const std::vector<size_t>& numCircleSamples,
                        float zConst);
  // Order of points is outer circle to inner.
  [[nodiscard]] auto GetNumPoints() const -> size_t override;
  [[nodiscard]] auto GetPoints() const -> const std::vector<V3dFlt>& override;

private:
  std::vector<V3dFlt> m_points{};
};

} // namespace TENTACLES
} // namespace GOOM
