#include "tentacles_fx.h"

#include "color/color_maps.h"
#include "color/random_color_maps.h"
#include "draw/goom_draw.h"
#include "fx_helper.h"
#include "goom_config.h"
#include "goom_plugin_info.h"
#include "spimpl.h"
#include "tentacles/circles_tentacle_layout.h"
#include "tentacles/tentacle_driver.h"
#include "utils/enum_utils.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/propagate_const.h"
#include "utils/timer.h"

#include <array>
#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX
{

using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using DRAW::IGoomDraw;
using DRAW::MultiplePixels;
using std::experimental::propagate_const;
using STD20::pi;
using TENTACLES::CirclesTentacleLayout;
using TENTACLES::TentacleDriver;
using UTILS::NUM;
using UTILS::Timer;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::HALF_PI;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::Weights;

class TentaclesFx::TentaclesImpl
{
public:
  TentaclesImpl(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps);

  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>;
  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;

  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;

  auto Start() -> void;
  auto Resume() -> void;

  auto Update() -> void;

private:
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const IGoomRand& m_goomRand;
  const SmallImageBitmaps& m_smallBitmaps;

  static constexpr double PROJECTION_DISTANCE = 170.0;
  static constexpr auto CAMERA_DISTANCE       = 8.0F;
  static constexpr auto ROTATION              = 1.5F * pi;
  enum class Drivers
  {
    NUM0 = 0,
    NUM1,
    NUM2,
    NUM3,
    _num // unused, and marks the enum end
  };
  static constexpr size_t NUM_TENTACLE_DRIVERS = NUM<Drivers>;
  const Weights<Drivers> m_driverWeights;
  const std::array<CirclesTentacleLayout, NUM_TENTACLE_DRIVERS> m_tentacleLayouts;
  std::vector<propagate_const<std::unique_ptr<TentacleDriver>>> m_tentacleDrivers{
      GetTentacleDrivers()};
  [[nodiscard]] auto GetTentacleDrivers() const
      -> std::vector<propagate_const<std::unique_ptr<TentacleDriver>>>;
  TentacleDriver* m_currentTentacleDriver{GetNextDriver()};
  [[nodiscard]] auto GetNextDriver() -> TentacleDriver*;

  std::shared_ptr<const RandomColorMaps> m_weightedDominantMainColorMaps{};
  std::shared_ptr<const RandomColorMaps> m_weightedDominantLowColorMaps{};
  std::shared_ptr<const IColorMap> m_dominantMainColorMap{};
  std::shared_ptr<const IColorMap> m_dominantLowColorMap{};
  std::shared_ptr<const IColorMap> m_dominantDotColorMap{};
  auto ChangeDominantColor() -> void;

  static constexpr uint32_t MAX_TIME_FOR_DOMINANT_COLOR = 100;
  Timer m_timeWithThisDominantColor{MAX_TIME_FOR_DOMINANT_COLOR};
  auto UpdateTimers() -> void;

  auto RefreshTentacles() -> void;
  auto DoTentaclesUpdate() -> void;
  auto UpdateTentacleWaveFrequency() -> void;
};

TentaclesFx::TentaclesFx(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept
  : m_pimpl{spimpl::make_unique_impl<TentaclesImpl>(fxHelper, smallBitmaps)}
{
}

auto TentaclesFx::GetFxName() const noexcept -> std::string
{
  return "Tentacles FX";
}

auto TentaclesFx::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return m_pimpl->GetCurrentColorMapsNames();
}

auto TentaclesFx::SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  m_pimpl->SetWeightedColorMaps(weightedColorMaps);
}

auto TentaclesFx::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  m_pimpl->SetZoomMidpoint(zoomMidpoint);
}

auto TentaclesFx::Start() noexcept -> void
{
  m_pimpl->Start();
}

auto TentaclesFx::Finish() noexcept -> void
{
  // nothing to do
}

auto TentaclesFx::Resume() noexcept -> void
{
  m_pimpl->Resume();
}

auto TentaclesFx::Suspend() noexcept -> void
{
  // nothing to do
}

auto TentaclesFx::ApplyMultiple() noexcept -> void
{
  m_pimpl->Update();
}

// clang-format off
static const auto LAYOUT1 = CirclesTentacleLayout{
    {10.0F,  80.0F}, {5.0F, 60.0F}, {16, 12,  8,  6, 4}
};
static const auto LAYOUT2 = CirclesTentacleLayout{
    {10.0F,  80.0F}, {5.0F, 50.0F}, {20, 16, 12,  6, 4}
};
static const auto LAYOUT3 = CirclesTentacleLayout{
    {10.0F, 100.0F}, {5.0F, 80.0F}, {30, 20, 14,  6, 4}
};
static const auto LAYOUT4 = CirclesTentacleLayout{
    {10.0F, 110.0F}, {5.0F, 90.0F}, {36, 26, 20, 12, 6}
};
// clang-format on

static constexpr auto DRIVERS_NUM0_WEIGHT = 05.0F;
static constexpr auto DRIVERS_NUM1_WEIGHT = 15.0F;
static constexpr auto DRIVERS_NUM2_WEIGHT = 15.0F;
static constexpr auto DRIVERS_NUM3_WEIGHT = 05.0F;

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
    }
{
  Expects(NUM_TENTACLE_DRIVERS == m_driverWeights.GetNumElements());
  Ensures(m_currentTentacleDriver != nullptr);
}

inline auto TentaclesFx::TentaclesImpl::Start() -> void
{
  m_timeWithThisDominantColor.SetToFinished();

  RefreshTentacles();
}

inline auto TentaclesFx::TentaclesImpl::Resume() -> void
{
  if (static constexpr auto PROB_NEW_DRIVER = 0.5F; m_goomRand.ProbabilityOf(PROB_NEW_DRIVER))
  {
    m_currentTentacleDriver = GetNextDriver();
  }

  m_timeWithThisDominantColor.SetToFinished();

  RefreshTentacles();
}

auto TentaclesFx::TentaclesImpl::GetTentacleDrivers() const
    -> std::vector<propagate_const<std::unique_ptr<TentacleDriver>>>
{
  auto tentacleDrivers = std::vector<propagate_const<std::unique_ptr<TentacleDriver>>>{};
  for (auto i = 0U; i < NUM_TENTACLE_DRIVERS; ++i)
  {
    tentacleDrivers.emplace_back(std::make_unique<TentacleDriver>(
        m_draw, m_goomRand, m_smallBitmaps, m_tentacleLayouts.at(i)));
  }

  for (auto i = 0U; i < NUM_TENTACLE_DRIVERS; ++i)
  {
    tentacleDrivers[i]->StartIterating();
    tentacleDrivers[i]->SetProjectionDistance(PROJECTION_DISTANCE);
    tentacleDrivers[i]->SetCameraPosition(CAMERA_DISTANCE, HALF_PI - ROTATION);
  }

  return tentacleDrivers;
}

inline auto TentaclesFx::TentaclesImpl::GetNextDriver() -> TentacleDriver*
{
  const auto driverIndex = static_cast<size_t>(m_driverWeights.GetRandomWeighted());
  return m_tentacleDrivers[driverIndex].get();
}

inline auto TentaclesFx::TentaclesImpl::RefreshTentacles() -> void
{
  Expects(m_currentTentacleDriver != nullptr);

  ChangeDominantColor();

  m_currentTentacleDriver->ChangeTentacleColorMaps();
}

auto TentaclesFx::TentaclesImpl::GetCurrentColorMapsNames() const noexcept
    -> std::vector<std::string>
{
  return {m_weightedDominantMainColorMaps->GetColorMapsName(),
          m_weightedDominantLowColorMaps->GetColorMapsName()};
}

auto TentaclesFx::TentaclesImpl::SetWeightedColorMaps(
    const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  Expects(weightedColorMaps.mainColorMaps != nullptr);
  Expects(weightedColorMaps.lowColorMaps != nullptr);

  if (weightedColorMaps.id == NORMAL_COLOR_TYPE)
  {
    std::for_each(begin(m_tentacleDrivers),
                  end(m_tentacleDrivers),
                  [&weightedColorMaps](auto& driver)
                  { driver->SetWeightedColorMaps(weightedColorMaps); });
  }
  else if (weightedColorMaps.id == DOMINANT_COLOR_TYPE)
  {
    m_weightedDominantMainColorMaps = weightedColorMaps.mainColorMaps;
    m_dominantMainColorMap =
        m_weightedDominantMainColorMaps->GetRandomColorMapPtr(RandomColorMaps::ALL_COLOR_MAP_TYPES);

    m_weightedDominantLowColorMaps = weightedColorMaps.lowColorMaps;
    m_dominantLowColorMap =
        m_weightedDominantLowColorMaps->GetRandomColorMapPtr(RandomColorMaps::ALL_COLOR_MAP_TYPES);

    m_dominantDotColorMap =
        m_weightedDominantMainColorMaps->GetRandomColorMapPtr(RandomColorMaps::ALL_COLOR_MAP_TYPES);
  }
  else
  {
    FailFast();
  }
}

inline auto TentaclesFx::TentaclesImpl::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept
    -> void
{
  std::for_each(begin(m_tentacleDrivers),
                end(m_tentacleDrivers),
                [&zoomMidpoint](auto& driver)
                { driver->SetAllTentaclesEndCentrePos(zoomMidpoint); });
}

inline auto TentaclesFx::TentaclesImpl::ChangeDominantColor() -> void
{
  if (!m_timeWithThisDominantColor.Finished())
  {
    return;
  }

  m_timeWithThisDominantColor.ResetToZero();

  m_currentTentacleDriver->SetDominantColorMaps(
      m_dominantMainColorMap, m_dominantLowColorMap, m_dominantDotColorMap);
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
  if (0 == m_goomInfo.GetSoundEvents().GetTimeSinceLastGoom())
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
  const auto tentacleWaveFreqMultiplier =
      m_goomInfo.GetSoundEvents().GetSoundInfo().GetAcceleration() <
              SoundInfo::ACCELERATION_MIDPOINT
          ? 1.0F
          : (1.0F / (1.1F - m_goomInfo.GetSoundEvents().GetSoundInfo().GetAcceleration()));
  m_currentTentacleDriver->MultiplyIterZeroYValWaveFreq(tentacleWaveFreqMultiplier);
}

} // namespace GOOM::VISUAL_FX
