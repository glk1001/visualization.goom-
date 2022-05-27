#include "tentacles_fx.h"

#include "color/colormaps.h"
#include "color/colorutils.h"
#include "color/random_colormaps.h"
#include "draw/goom_draw.h"
#include "fx_helper.h"
#include "goom/spimpl.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "tentacles/circles_tentacle_layout.h"
#include "tentacles/tentacle_driver.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/timer.h"

#include <array>
#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX
{

using COLOR::GetLightenedColor;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using DRAW::IGoomDraw;
using STD20::pi;
using TENTACLES::CirclesTentacleLayout;
using TENTACLES::TentacleDriver;
using UTILS::Timer;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::HALF_PI;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::Weights;

class TentaclesFx::TentaclesImpl
{
public:
  TentaclesImpl(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps);

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;

  auto Start() -> void;
  auto Resume() -> void;

  auto Update() -> void;

private:
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const IGoomRand& m_goomRand;
  const SmallImageBitmaps& m_smallBitmaps;

  static constexpr double PROJECTION_DISTANCE = 170.0;
  static constexpr float CAMERA_DISTANCE = 8.0F;
  static constexpr float ROTATION = 1.5F * pi;
  static constexpr size_t NUM_TENTACLE_DRIVERS = 4;
  enum class Drivers
  {
    NUM0 = 0,
    NUM1,
    NUM2,
    NUM3,
    _num // unused and must be last
  };
  const Weights<Drivers> m_driverWeights;
  const std::array<CirclesTentacleLayout, NUM_TENTACLE_DRIVERS> m_tentacleLayouts;
  std::vector<std::unique_ptr<TentacleDriver>> m_tentacleDrivers;
  [[nodiscard]] auto GetTentacleDrivers() const -> std::vector<std::unique_ptr<TentacleDriver>>;
  TentacleDriver* m_currentTentacleDriver;
  [[nodiscard]] auto GetNextDriver() const -> TentacleDriver*;

  std::shared_ptr<const IColorMap> m_dominantColorMap{};
  Pixel m_dominantColor{};
  Pixel m_dominantLowColor{};
  Pixel m_dominantDotColor{};
  auto ChangeDominantColor() -> void;
  auto UpdateDominantColors() -> void;

  static constexpr uint32_t MAX_TIME_FOR_DOMINANT_COLOR = 100;
  Timer m_timeWithThisDominantColor{MAX_TIME_FOR_DOMINANT_COLOR};
  auto UpdateTimers() -> void;

  auto RefreshTentacles() -> void;
  auto DoTentaclesUpdate() -> void;
  auto UpdateTentacleWaveFrequency() -> void;
};

TentaclesFx::TentaclesFx(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept
  : m_fxImpl{spimpl::make_unique_impl<TentaclesImpl>(fxHelper, smallBitmaps)}
{
}

auto TentaclesFx::GetFxName() const noexcept -> std::string
{
  return "Tentacles FX";
}

auto TentaclesFx::SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  m_fxImpl->SetWeightedColorMaps(weightedColorMaps);
}

auto TentaclesFx::Start() noexcept -> void
{
  m_fxImpl->Start();
}

auto TentaclesFx::Finish() noexcept -> void
{
  // nothing to do
}

auto TentaclesFx::Resume() noexcept -> void
{
  m_fxImpl->Resume();
}

auto TentaclesFx::Suspend() noexcept -> void
{
  // nothing to do
}

auto TentaclesFx::ApplyMultiple() noexcept -> void
{
  m_fxImpl->Update();
}

// clang-format off
static const CirclesTentacleLayout LAYOUT1{
    10,  80, {16, 12,  8,  6, 4}, 0
};
static const CirclesTentacleLayout LAYOUT2{
    10,  80, {20, 16, 12,  6, 4}, 0
};
static const CirclesTentacleLayout LAYOUT3{
    10, 100, {30, 20, 14,  6, 4}, 0
};
static const CirclesTentacleLayout LAYOUT4{
    10, 110, {36, 26, 20, 12, 6}, 0
};

static constexpr float DRIVERS_NUM0_WEIGHT =   5.0F;
static constexpr float DRIVERS_NUM1_WEIGHT =  15.0F;
static constexpr float DRIVERS_NUM2_WEIGHT =  15.0F;
static constexpr float DRIVERS_NUM3_WEIGHT =   5.0F;
// clang-format on

TentaclesFx::TentaclesImpl::TentaclesImpl(const FxHelper& fxHelper,
                                          const SmallImageBitmaps& smallBitmaps)
  : m_draw{fxHelper.GetDraw()},
    m_goomInfo{fxHelper.GetGoomInfo()},
    m_goomRand{fxHelper.GetGoomRand()},
    m_smallBitmaps{smallBitmaps},
    m_driverWeights{
      m_goomRand,
      {
          {Drivers::NUM0, DRIVERS_NUM0_WEIGHT},
          {Drivers::NUM1, DRIVERS_NUM1_WEIGHT},
          {Drivers::NUM2, DRIVERS_NUM2_WEIGHT},
          {Drivers::NUM3, DRIVERS_NUM3_WEIGHT},
      }},
    m_tentacleLayouts{
        LAYOUT1,
        LAYOUT2,
        LAYOUT3,
        LAYOUT4,
    },
    m_tentacleDrivers{GetTentacleDrivers()},
    m_currentTentacleDriver{GetNextDriver()}
{
  Expects(NUM_TENTACLE_DRIVERS == m_driverWeights.GetNumElements());
  Ensures(m_currentTentacleDriver != nullptr);
}

inline auto TentaclesFx::TentaclesImpl::Start() -> void
{
  RefreshTentacles();
}

inline auto TentaclesFx::TentaclesImpl::Resume() -> void
{
  if (constexpr float PROB_NEW_DRIVER = 0.5F; m_goomRand.ProbabilityOf(PROB_NEW_DRIVER))
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
    tentacleDrivers[i]->SetCameraPosition(CAMERA_DISTANCE, HALF_PI - ROTATION);
  }

  return tentacleDrivers;
}

inline auto TentaclesFx::TentaclesImpl::GetNextDriver() const -> TentacleDriver*
{
  const auto driverIndex = static_cast<size_t>(m_driverWeights.GetRandomWeighted());
  return m_tentacleDrivers[driverIndex].get();
}

inline auto TentaclesFx::TentaclesImpl::RefreshTentacles() -> void
{
  Expects(m_currentTentacleDriver);

  static constexpr float PROB_REVERSE_COLOR_MIX = 0.33F;
  m_currentTentacleDriver->SetReverseColorMix(m_goomRand.ProbabilityOf(PROB_REVERSE_COLOR_MIX));
  m_currentTentacleDriver->TentaclesColorMapsChanged();
}

inline auto TentaclesFx::TentaclesImpl::SetWeightedColorMaps(
    const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  Expects(weightedColorMaps.mainColorMaps != nullptr);

  m_dominantColorMap =
      weightedColorMaps.mainColorMaps->GetRandomColorMapPtr(RandomColorMaps::ALL_COLOR_MAP_TYPES);
  m_dominantColor = RandomColorMaps{m_goomRand}.GetRandomColor(*m_dominantColorMap, 0.0F, 1.0F);
  m_dominantDotColor = RandomColorMaps{m_goomRand}.GetRandomColor(*m_dominantColorMap, 0.0F, 1.0F);
  UpdateDominantColors();

  for (auto& driver : m_tentacleDrivers)
  {
    driver->SetWeightedColorMaps(weightedColorMaps.mainColorMaps);
    driver->SetDominantColors(m_dominantColor, m_dominantLowColor, m_dominantDotColor);
  }
}

inline auto TentaclesFx::TentaclesImpl::Update() -> void
{
  UpdateTimers();

  DoTentaclesUpdate();
}

inline auto TentaclesFx::TentaclesImpl::UpdateTimers() -> void
{
  m_timeWithThisDominantColor.Increment();
}

inline auto TentaclesFx::TentaclesImpl::DoTentaclesUpdate() -> void
{
  if (0 == m_goomInfo.GetSoundInfo().GetTimeSinceLastGoom())
  {
    ChangeDominantColor();
  }

  UpdateTentacleWaveFrequency();

  m_currentTentacleDriver->Update();
}

inline auto TentaclesFx::TentaclesImpl::UpdateTentacleWaveFrequency() -> void
{
  // Higher sound acceleration increases tentacle wave frequency.
  Expects(m_currentTentacleDriver);
  const float tentacleWaveFreq =
      m_goomInfo.GetSoundInfo().GetAcceleration() < 0.3F
          ? 1.25F
          : (1.0F / (1.10F - m_goomInfo.GetSoundInfo().GetAcceleration()));
  m_currentTentacleDriver->MultiplyIterZeroYValWaveFreq(tentacleWaveFreq);
}

inline auto TentaclesFx::TentaclesImpl::ChangeDominantColor() -> void
{
  if (!m_timeWithThisDominantColor.Finished())
  {
    return;
  }

  m_timeWithThisDominantColor.ResetToZero();

  UpdateDominantColors();
  m_currentTentacleDriver->SetDominantColors(m_dominantColor, m_dominantLowColor,
                                             m_dominantDotColor);
}

inline auto TentaclesFx::TentaclesImpl::UpdateDominantColors() -> void
{
  Expects(m_dominantColorMap != nullptr);
  const Pixel newColor =
      RandomColorMaps{m_goomRand}.GetRandomColor(*m_dominantColorMap, 0.0F, 1.0F);
  static constexpr float COLOR_MIX_T = 0.70F;
  m_dominantLowColor = IColorMap::GetColorMix(m_dominantLowColor, newColor, COLOR_MIX_T);
  static constexpr float COLOR_POWER = 0.67F;
  m_dominantColor = GetLightenedColor(m_dominantLowColor, COLOR_POWER);

  m_dominantDotColor = RandomColorMaps{m_goomRand}.GetRandomColor(*m_dominantColorMap, 0.0F, 1.0F);
}

} // namespace GOOM::VISUAL_FX
