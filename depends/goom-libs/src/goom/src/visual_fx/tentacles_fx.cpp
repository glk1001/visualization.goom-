module;

//#undef NO_LOGGING
#define REQUIRE_ASSERTS_FOR_ALL_BUILDS // Check for non-null pointers.

#include "goom/goom_config.h"
#include "goom/spimpl.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

module Goom.VisualFx.TentaclesFx;

import Goom.Color.RandomColorMaps;
import Goom.Color.ColorMaps;
import Goom.Draw.GoomDrawBase;
import Goom.Utils.EnumUtils;
import Goom.Utils.GoomTime;
import Goom.Utils.Timer;
import Goom.Utils.Math.GoomRandBase;
import Goom.VisualFx.FxHelper;
import Goom.VisualFx.FxUtils;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;
import Goom.Lib.SoundInfo;
import :CirclesTentacleLayout;
import :TentacleDriver;

namespace GOOM::VISUAL_FX
{

using COLOR::WeightedRandomColorMaps;
using DRAW::IGoomDraw;
using FX_UTILS::RandomPixelBlender;
using TENTACLES::CirclesTentacleLayout;
using TENTACLES::TentacleDriver;
using UTILS::NUM;
using UTILS::Timer;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::Weights;

class TentaclesFx::TentaclesImpl
{
public:
  explicit TentaclesImpl(FxHelper& fxHelper);

  auto Start() -> void;
  auto Resume() -> void;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;
  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>;

  auto ApplyToImageBuffers() -> void;

private:
  FxHelper* m_fxHelper;

  PixelChannelType m_defaultAlpha = MAX_ALPHA;

  enum class Drivers : UnderlyingEnumType
  {
    NUM0,
    NUM1,
    NUM2,
    NUM3,
  };
  static constexpr size_t NUM_TENTACLE_DRIVERS = NUM<Drivers>;
  Weights<Drivers> m_driverWeights;
  std::vector<TentacleDriver> m_tentacleDrivers;
  [[nodiscard]] static auto GetTentacleDrivers(
      IGoomDraw& draw,
      const IGoomRand& goomRand,
      const GoomTime& goomTime,
      const std::array<CirclesTentacleLayout, NUM_TENTACLE_DRIVERS>& tentacleLayouts,
      PixelChannelType defaultAlpha) -> std::vector<TentacleDriver>;
  TentacleDriver* m_currentTentacleDriver{GetNextDriver()};
  [[nodiscard]] auto GetNextDriver() -> TentacleDriver*;

  WeightedRandomColorMaps m_weightedDominantMainColorMaps;
  WeightedRandomColorMaps m_weightedDominantLowColorMaps;
  COLOR::ConstColorMapSharedPtr m_dominantMainColorMapPtr = nullptr;
  COLOR::ConstColorMapSharedPtr m_dominantLowColorMapPtr  = nullptr;
  auto ChangeDominantColor() -> void;

  RandomPixelBlender m_pixelBlender;
  auto UpdatePixelBlender() noexcept -> void;

  static constexpr uint32_t MAX_TIME_FOR_DOMINANT_COLOR = 100;
  Timer m_timeWithThisDominantColor{m_fxHelper->GetGoomTime(), MAX_TIME_FOR_DOMINANT_COLOR};

  auto RefreshTentacles() -> void;
  auto DoTentaclesUpdate() -> void;
  auto UpdateTentacleWaveFrequency() -> void;
};

TentaclesFx::TentaclesFx(FxHelper& fxHelper) noexcept
  : m_pimpl{spimpl::make_unique_impl<TentaclesImpl>(fxHelper)}
{
}

auto TentaclesFx::GetFxName() const noexcept -> std::string
{
  return "Tentacles FX";
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

auto TentaclesFx::ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pimpl->ChangePixelBlender(pixelBlenderParams);
}

auto TentaclesFx::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  m_pimpl->SetZoomMidpoint(zoomMidpoint);
}

auto TentaclesFx::SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  m_pimpl->SetWeightedColorMaps(weightedColorMaps);
}

auto TentaclesFx::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return m_pimpl->GetCurrentColorMapsNames();
}

auto TentaclesFx::ApplyToImageBuffers() noexcept -> void
{
  m_pimpl->ApplyToImageBuffers();
}

static constexpr auto LAYOUT0_START_RADIUS = 380.0F;
static constexpr auto LAYOUT1_START_RADIUS = 380.0F;
static constexpr auto LAYOUT2_START_RADIUS = 400.0F;
static constexpr auto LAYOUT3_START_RADIUS = 410.0F;

static constexpr auto LAYOUT0_END_RADIUS = 120.0F;
static constexpr auto LAYOUT1_END_RADIUS = 120.0F;
static constexpr auto LAYOUT2_END_RADIUS = 120.0F;
static constexpr auto LAYOUT3_END_RADIUS = 120.0F;

static constexpr auto LAYOUT0_NUM_TENTACLES = 40U;
static constexpr auto LAYOUT1_NUM_TENTACLES = 50U;
static constexpr auto LAYOUT2_NUM_TENTACLES = 70U;
static constexpr auto LAYOUT3_NUM_TENTACLES = 150U;

static constexpr auto DRIVERS_NUM0_WEIGHT = 50.0F;
static constexpr auto DRIVERS_NUM1_WEIGHT = 30.0F;
static constexpr auto DRIVERS_NUM2_WEIGHT = 10.0F;
static constexpr auto DRIVERS_NUM3_WEIGHT = 05.0F;

TentaclesFx::TentaclesImpl::TentaclesImpl(FxHelper& fxHelper)
  : m_fxHelper{&fxHelper},
    m_driverWeights{
      m_fxHelper->GetGoomRand(),
      {
          {Drivers::NUM0, DRIVERS_NUM0_WEIGHT},
          {Drivers::NUM1, DRIVERS_NUM1_WEIGHT},
          {Drivers::NUM2, DRIVERS_NUM2_WEIGHT},
          {Drivers::NUM3, DRIVERS_NUM3_WEIGHT},
      }},
    // clang-format off
    m_tentacleDrivers{GetTentacleDrivers(
        fxHelper.GetDraw(),
        m_fxHelper->GetGoomRand(),
        m_fxHelper->GetGoomTime(),
        {{
           CirclesTentacleLayout{{LAYOUT0_START_RADIUS, LAYOUT0_END_RADIUS, LAYOUT0_NUM_TENTACLES}},
           CirclesTentacleLayout{{LAYOUT1_START_RADIUS, LAYOUT1_END_RADIUS, LAYOUT1_NUM_TENTACLES}},
           CirclesTentacleLayout{{LAYOUT2_START_RADIUS, LAYOUT2_END_RADIUS, LAYOUT2_NUM_TENTACLES}},
           CirclesTentacleLayout{{LAYOUT3_START_RADIUS, LAYOUT3_END_RADIUS, LAYOUT3_NUM_TENTACLES}},
        }},
        m_defaultAlpha
    )},
    // clang-format on
    m_pixelBlender{fxHelper.GetGoomRand()}
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
  if (static constexpr auto PROB_NEW_DRIVER = 0.5F;
      m_fxHelper->GetGoomRand().ProbabilityOf(PROB_NEW_DRIVER))
  {
    m_currentTentacleDriver = GetNextDriver();
  }

  m_timeWithThisDominantColor.SetToFinished();

  RefreshTentacles();
}

auto TentaclesFx::TentaclesImpl::GetTentacleDrivers(
    IGoomDraw& draw,
    const IGoomRand& goomRand,
    const GoomTime& goomTime,
    const std::array<CirclesTentacleLayout, NUM_TENTACLE_DRIVERS>& tentacleLayouts,
    const PixelChannelType defaultAlpha) -> std::vector<TentacleDriver>
{
  auto tentacleDrivers = std::vector<TentacleDriver>{};
  for (auto i = 0U; i < NUM_TENTACLE_DRIVERS; ++i)
  {
    tentacleDrivers.emplace_back(draw, goomRand, goomTime, tentacleLayouts.at(i), defaultAlpha);
  }

  for (auto i = 0U; i < NUM_TENTACLE_DRIVERS; ++i)
  {
    tentacleDrivers[i].StartIterating();
  }

  return tentacleDrivers;
}

inline auto TentaclesFx::TentaclesImpl::GetNextDriver() -> TentacleDriver*
{
  const auto driverIndex = static_cast<size_t>(m_driverWeights.GetRandomWeighted());
  return &m_tentacleDrivers[driverIndex];
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
  return {m_weightedDominantMainColorMaps.GetColorMapsName(),
          m_weightedDominantLowColorMaps.GetColorMapsName()};
}

auto TentaclesFx::TentaclesImpl::SetWeightedColorMaps(
    const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  const auto newWeightedColorMaps =
      GetWeightedColorMapsWithNewAlpha(weightedColorMaps, m_defaultAlpha);

  if (weightedColorMaps.id == NORMAL_COLOR_TYPE)
  {
    std::for_each(begin(m_tentacleDrivers),
                  end(m_tentacleDrivers),
                  [&newWeightedColorMaps](auto& driver)
                  { driver.SetWeightedColorMaps(newWeightedColorMaps); });
  }
  else if (newWeightedColorMaps.id == DOMINANT_COLOR_TYPE)
  {
    m_weightedDominantMainColorMaps = newWeightedColorMaps.mainColorMaps;
    m_weightedDominantLowColorMaps  = newWeightedColorMaps.lowColorMaps;

    m_dominantMainColorMapPtr = m_weightedDominantMainColorMaps.GetRandomColorMapSharedPtr(
        WeightedRandomColorMaps::GetAllColorMapsTypes());
    m_dominantLowColorMapPtr = m_weightedDominantLowColorMaps.GetRandomColorMapSharedPtr(
        WeightedRandomColorMaps::GetAllColorMapsTypes());
  }
  else
  {
    FailFast();
  }
}

inline auto TentaclesFx::TentaclesImpl::ChangePixelBlender(
    const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pixelBlender.SetPixelBlendType(pixelBlenderParams);
}

inline auto TentaclesFx::TentaclesImpl::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept
    -> void
{
  std::for_each(begin(m_tentacleDrivers),
                end(m_tentacleDrivers),
                [&zoomMidpoint](auto& driver) { driver.SetTentaclesEndCentrePos(zoomMidpoint); });
}

inline auto TentaclesFx::TentaclesImpl::ChangeDominantColor() -> void
{
  Expects(m_currentTentacleDriver != nullptr);

  if (!m_timeWithThisDominantColor.Finished())
  {
    return;
  }

  m_timeWithThisDominantColor.ResetToZero();

  m_currentTentacleDriver->SetDominantColorMaps(m_dominantMainColorMapPtr,
                                                m_dominantLowColorMapPtr);
}

inline auto TentaclesFx::TentaclesImpl::ApplyToImageBuffers() -> void
{
  UpdatePixelBlender();
  DoTentaclesUpdate();
}

inline auto TentaclesFx::TentaclesImpl::UpdatePixelBlender() noexcept -> void
{
  m_fxHelper->GetDraw().SetPixelBlendFunc(m_pixelBlender.GetCurrentPixelBlendFunc());
  m_pixelBlender.Update();
}

inline auto TentaclesFx::TentaclesImpl::DoTentaclesUpdate() -> void
{
  Expects(m_currentTentacleDriver != nullptr);

  if (0 == m_fxHelper->GetSoundEvents().GetTimeSinceLastGoom())
  {
    ChangeDominantColor();
  }

  UpdateTentacleWaveFrequency();

  m_currentTentacleDriver->Update();
}

inline auto TentaclesFx::TentaclesImpl::UpdateTentacleWaveFrequency() -> void
{
  Expects(m_currentTentacleDriver != nullptr);

  // Higher sound acceleration increases tentacle wave frequency.
  const auto tentacleWaveFreqMultiplier =
      m_fxHelper->GetSoundEvents().GetSoundInfo().GetAcceleration() <
              SoundInfo::ACCELERATION_MIDPOINT
          ? 0.95F
          : (1.0F / (1.1F - m_fxHelper->GetSoundEvents().GetSoundInfo().GetAcceleration()));
  m_currentTentacleDriver->MultiplyIterZeroYValWaveFreq(tentacleWaveFreqMultiplier);
}

} // namespace GOOM::VISUAL_FX
