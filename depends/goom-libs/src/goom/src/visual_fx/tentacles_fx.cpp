#include "tentacles_fx.h"

#include "color/colormaps.h"
#include "color/colorutils.h"
#include "color/random_colormaps.h"
#include "draw/goom_draw.h"
#include "fx_helpers.h"
#include "goom/spimpl.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "tentacles/circles_tentacle_layout.h"
#include "tentacles/tentacle_driver.h"
#include "utils/goom_rand_base.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/mathutils.h"
#include "utils/timer.h"

#include <array>
#undef NDEBUG
#include <cassert>
#include <memory>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
#else
namespace GOOM::VISUAL_FX
{
#endif

using COLOR::GetLightenedColor;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using DRAW::IGoomDraw;
using TENTACLES::CirclesTentacleLayout;
using TENTACLES::TentacleDriver;
using UTILS::IGoomRand;
using UTILS::m_half_pi;
using UTILS::m_pi;
using UTILS::SmallImageBitmaps;
using UTILS::Timer;
using UTILS::Weights;

class TentaclesFx::TentaclesImpl
{
public:
  TentaclesImpl(const FxHelpers& fxHelpers, const SmallImageBitmaps& smallBitmaps);

  void SetWeightedColorMaps(std::shared_ptr<RandomColorMaps> weightedMaps);

  void Start();
  void Resume();

  void Update();

private:
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  IGoomRand& m_goomRand;
  const SmallImageBitmaps& m_smallBitmaps;

  static constexpr double PROJECTION_DISTANCE = 170.0;
  static constexpr float CAMERA_DISTANCE = 8.0F;
  static constexpr float ROTATION = 1.5F * m_pi;
  static constexpr size_t NUM_TENTACLE_DRIVERS = 4;
  const Weights<size_t> m_driverWeights;
  const std::array<CirclesTentacleLayout, NUM_TENTACLE_DRIVERS> m_tentacleLayouts;
  std::vector<std::unique_ptr<TentacleDriver>> m_tentacleDrivers;
  [[nodiscard]] auto GetTentacleDrivers() const -> std::vector<std::unique_ptr<TentacleDriver>>;
  TentacleDriver* m_currentTentacleDriver;
  [[nodiscard]] auto GetNextDriver() const -> TentacleDriver*;

  std::shared_ptr<const IColorMap> m_dominantColorMap{};
  Pixel m_dominantColor{};
  Pixel m_dominantLowColor{};
  void ChangeDominantColor();
  void UpdateDominantColors();

  static constexpr uint32_t MAX_TIME_FOR_DOMINANT_COLOR = 100;
  Timer m_timeWithThisDominantColor{MAX_TIME_FOR_DOMINANT_COLOR};
  void UpdateTimers();

  void RefreshTentacles();
  void DoTentaclesUpdate();
  void UpdateTentacleWaveFrequency();
};

TentaclesFx::TentaclesFx(const FxHelpers& fxHelpers, const SmallImageBitmaps& smallBitmaps) noexcept
  : m_fxImpl{spimpl::make_unique_impl<TentaclesImpl>(fxHelpers, smallBitmaps)}
{
}

auto TentaclesFx::GetFxName() const -> std::string
{
  return "Tentacles FX";
}

void TentaclesFx::SetWeightedColorMaps(const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_fxImpl->SetWeightedColorMaps(weightedMaps);
}

void TentaclesFx::Start()
{
  m_fxImpl->Start();
}

void TentaclesFx::Resume()
{
  m_fxImpl->Resume();
}

void TentaclesFx::Suspend()
{
  // nothing to do
}

void TentaclesFx::Finish()
{
  // nothing to do
}

void TentaclesFx::ApplyMultiple()
{
  m_fxImpl->Update();
}

TentaclesFx::TentaclesImpl::TentaclesImpl(const FxHelpers& fxHelpers,
                                          const SmallImageBitmaps& smallBitmaps)
  : m_draw{fxHelpers.GetDraw()},
    m_goomInfo{fxHelpers.GetGoomInfo()},
    m_goomRand{fxHelpers.GetGoomRand()},
    m_smallBitmaps{smallBitmaps},
    // clang-format off
    m_driverWeights{
      m_goomRand,
      {
          {0,  5},
          {1, 15},
          {2, 15},
          {3,  5},
      }},
    m_tentacleLayouts{{
        {10,  80, {16, 12,  8,  6, 4}, 0},
        {10,  80, {20, 16, 12,  6, 4}, 0},
        {10, 100, {30, 20, 14,  6, 4}, 0},
        {10, 110, {36, 26, 20, 12, 6}, 0},
    }},
    // clang-format on
    m_tentacleDrivers{GetTentacleDrivers()},
    m_currentTentacleDriver{GetNextDriver()}
{
  assert(NUM_TENTACLE_DRIVERS == m_driverWeights.GetNumElements());
  assert(m_currentTentacleDriver);
}

inline void TentaclesFx::TentaclesImpl::Start()
{
  RefreshTentacles();
}

inline void TentaclesFx::TentaclesImpl::Resume()
{
  constexpr float PROB_NEW_DRIVER = 0.5F;
  if (m_goomRand.ProbabilityOf(PROB_NEW_DRIVER))
  {
    m_currentTentacleDriver = GetNextDriver();
  }

  RefreshTentacles();
}

auto TentaclesFx::TentaclesImpl::GetTentacleDrivers() const
    -> std::vector<std::unique_ptr<TentacleDriver>>
{
  std::vector<std::unique_ptr<TentacleDriver>> tentacleDrivers{};
  for (size_t i = 0; i < NUM_TENTACLE_DRIVERS; ++i)
  {
    tentacleDrivers.emplace_back(std::make_unique<TentacleDriver>(
        m_draw, m_goomRand, m_smallBitmaps, m_tentacleLayouts.at(i)));
  }

  for (size_t i = 0; i < NUM_TENTACLE_DRIVERS; ++i)
  {
    tentacleDrivers[i]->StartIterating();
    tentacleDrivers[i]->SetProjectionDistance(PROJECTION_DISTANCE);
    tentacleDrivers[i]->SetCameraPosition(CAMERA_DISTANCE, m_half_pi - ROTATION);
  }

  return tentacleDrivers;
}

inline auto TentaclesFx::TentaclesImpl::GetNextDriver() const -> TentacleDriver*
{
  const size_t driverIndex = m_driverWeights.GetRandomWeighted();
  return m_tentacleDrivers[driverIndex].get();
}

inline void TentaclesFx::TentaclesImpl::RefreshTentacles()
{
  assert(m_currentTentacleDriver);

  constexpr float PROB_REVERSE_COLOR_MIX = 0.33F;
  m_currentTentacleDriver->SetReverseColorMix(m_goomRand.ProbabilityOf(PROB_REVERSE_COLOR_MIX));
  m_currentTentacleDriver->TentaclesColorMapsChanged();
}

inline void TentaclesFx::TentaclesImpl::SetWeightedColorMaps(
    const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_dominantColorMap = weightedMaps->GetRandomColorMapPtr(RandomColorMaps::ALL);
  m_dominantColor = RandomColorMaps{m_goomRand}.GetRandomColor(*m_dominantColorMap, 0.0F, 1.0F);
  UpdateDominantColors();

  for (auto& driver : m_tentacleDrivers)
  {
    driver->SetWeightedColorMaps(weightedMaps);
    driver->SetDominantColors(m_dominantColor, m_dominantLowColor);
  }
}

inline void TentaclesFx::TentaclesImpl::Update()
{
  UpdateTimers();

  DoTentaclesUpdate();
}

inline void TentaclesFx::TentaclesImpl::UpdateTimers()
{
  m_timeWithThisDominantColor.Increment();
}

inline void TentaclesFx::TentaclesImpl::DoTentaclesUpdate()
{
  if (0 == m_goomInfo.GetSoundInfo().GetTimeSinceLastGoom())
  {
    ChangeDominantColor();
  }

  UpdateTentacleWaveFrequency();

  m_currentTentacleDriver->Update();
}

inline void TentaclesFx::TentaclesImpl::UpdateTentacleWaveFrequency()
{
  // Higher sound acceleration increases tentacle wave frequency.
  assert(m_currentTentacleDriver);
  const float tentacleWaveFreq =
      m_goomInfo.GetSoundInfo().GetAcceleration() < 0.3F
          ? 1.25F
          : (1.0F / (1.10F - m_goomInfo.GetSoundInfo().GetAcceleration()));
  m_currentTentacleDriver->MultiplyIterZeroYValWaveFreq(tentacleWaveFreq);
}

inline void TentaclesFx::TentaclesImpl::ChangeDominantColor()
{
  if (!m_timeWithThisDominantColor.Finished())
  {
    return;
  }

  m_timeWithThisDominantColor.ResetToZero();

  UpdateDominantColors();
  m_currentTentacleDriver->SetDominantColors(m_dominantColor, m_dominantLowColor);
}

inline void TentaclesFx::TentaclesImpl::UpdateDominantColors()
{
  assert(m_dominantColorMap);
  const Pixel newColor =
      RandomColorMaps{m_goomRand}.GetRandomColor(*m_dominantColorMap, 0.0F, 1.0F);
  m_dominantColor = IColorMap::GetColorMix(m_dominantColor, newColor, 0.7F);
  m_dominantLowColor = GetLightenedColor(m_dominantColor, 0.67F);
}

#if __cplusplus <= 201402L
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX
#endif
