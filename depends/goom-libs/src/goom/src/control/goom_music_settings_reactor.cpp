module;

#include "goom/debug_with_println.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <format>
#include <fstream>
#include <print>
#include <stdexcept>
#include <string>

module Goom.Control.GoomMusicSettingsReactor;

import Goom.Control.GoomAllVisualFx;
import Goom.FilterFx.FilterSettingsService;
import Goom.FilterFx.FilterSpeed;
import Goom.Lib.AssertUtils;
import Goom.Lib.SoundInfo;
import Goom.Lib.GoomPaths;
import Goom.Lib.GoomTypes;
import Goom.Utils.DateUtils;
import Goom.Utils.EnumUtils;
import Goom.Utils.StrUtils;
import Goom.PluginInfo;
import :GoomLock;

namespace GOOM::CONTROL
{

using FILTER_FX::FilterSettingsService;
using FILTER_FX::Speed;
using UTILS::EnumToString;
using UTILS::GetCurrentDateTimeAsString;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::NUM;
using UTILS::StringJoin;
using UTILS::ToStrings;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;

static constexpr auto COLLECT_EVENT_STATS = true;

static constexpr auto LOCK_TIME_NORMAL_UPDATE                  = 50U;
static constexpr auto LOCK_TIME_SLOWER_SPEED_AND_SPEED_FORWARD = 75U;
static constexpr auto LOCK_TIME_REVERSE_SPEED                  = 100U;
static constexpr auto LOCK_TIME_MEGA_LENT_INCREASE             = 50U;
static constexpr auto LOCK_TIME_SPEED_CHANGE_INCREASE          = 50U;
static constexpr auto LOCK_TIME_CHANGE_LERP_FACTOR_TO_END      = 150U;

static constexpr auto PROB_CHANGE_STATE                           = 0.40F;
static constexpr auto PROB_CHANGE_FILTER_MODE                     = 0.05F;
static constexpr auto PROB_CHANGE_GPU_FILTER_MODE                 = 0.05F;
static constexpr auto PROB_CHANGE_FILTER_EXTRA_SETTINGS           = 0.90F;
static constexpr auto PROB_CHANGE_TO_MEGA_LENT_MODE               = 1.0F / 700.F;
static constexpr auto PROB_FILTER_REVERSE_ON                      = 0.10F;
static constexpr auto PROB_FILTER_REVERSE_OFF_AND_STOP_SPEED      = 0.20F;
static constexpr auto PROB_FILTER_SPEED_STOP_SPEED_MINUS_1        = 0.20F;
static constexpr auto PROB_FILTER_SPEED_STOP_SPEED                = 0.10F;
static constexpr auto PROB_FILTER_CHANGE_SPEED_AND_TOGGLE_REVERSE = 0.05F;
static constexpr auto PROB_FILTER_TOGGLE_ROTATION                 = 0.125F;
static constexpr auto PROB_FILTER_INCREASE_ROTATION               = 0.250F;
static constexpr auto PROB_FILTER_DECREASE_ROTATION               = 0.875F;
static constexpr auto PROB_FILTER_STOP_ROTATION                   = 0.250F;

enum class ChangeEvents : UnderlyingEnumType
{
  CHANGE_STATE,
  CHANGE_FILTER_MODE,
  CHANGE_FILTER_MODE_REJECTED,
  CHANGE_GPU_FILTER_MODE,
  CHANGE_GPU_FILTER_MODE_REJECTED,
  BIG_UPDATE,
  BIG_BREAK,
  GO_SLOWER,
  BIG_NORMAL_UPDATE,
  MEGA_LENT_UPDATE,
  CHANGE_FILTER_EXTRA_SETTINGS,
  CHANGE_FILTER_EXTRA_SETTINGS_REJECTED,
  CHANGE_ROTATION,
  TURN_OFF_ROTATION,
  TURN_OFF_ROTATION_REJECTED,
  SLOWER_ROTATION,
  SLOWER_ROTATION_REJECTED,
  FASTER_ROTATION,
  FASTER_ROTATION_REJECTED,
  TOGGLE_ROTATION,
  TOGGLE_ROTATION_REJECTED,
  SET_SPEED_REVERSE,
  SET_SLOW_SPEED,
  SET_STOP_SPEED,
  SET_SLOWER_SPEED_AND_SPEED_FORWARD,
  SET_SLOWER_SPEED_AND_TOGGLE_REVERSE,
  CHANGE_SPEED,
  CHANGE_GPU_LERP_FACTOR,
  RESET_GPU_LERP_FACTOR_BASED_ON_SPEED,
  CHANGE_FILTER_LERP_FACTOR,
  RESET_FILTER_LERP_FACTOR_BASED_ON_SPEED,
  RESET_FILTER_LERP_FACTOR_TO_END,
  RESET_FILTER_LERP_FACTOR,
};
using enum ChangeEvents;

class GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl
{
public:
  GoomMusicSettingsReactorImpl(const PluginInfo& goomInfo,
                               const GoomRand& goomRand,
                               GoomAllVisualFx& visualFx,
                               FilterSettingsService& filterSettingsService) noexcept;

  auto SetDumpDirectory(const std::string& dumpDirectory) -> void;

  auto Start() -> void;
  auto Finish() -> void;
  auto NewCycle() -> void;

  auto UpdateSettings() -> void;

  [[nodiscard]] auto GetNameValueParams() const -> NameValuePairs;

private:
  const PluginInfo* m_goomInfo;
  const GoomRand* m_goomRand;
  GoomAllVisualFx* m_visualFx;
  FilterSettingsService* m_filterSettingsService;
  std::string m_dumpDirectory;
  GoomLock m_changeLock; // pour empecher de nouveaux changements
  uint32_t m_timeInState = 0U;

  bool m_hasFilterModeChangedInThisUpdate                        = false;
  static constexpr auto MAX_UPDATES_BETWEEN_FILTER_CHANGES_RANGE = NumberRange{300, 500};
  int32_t m_maxUpdatesBetweenFilterChanges  = MAX_UPDATES_BETWEEN_FILTER_CHANGES_RANGE.min;
  int32_t m_numUpdatesSinceLastFilterChange = 0;

  bool m_hasGpuFilterModeChangedInThisUpdate                         = false;
  static constexpr auto MAX_UPDATES_BETWEEN_GPU_FILTER_CHANGES_RANGE = NumberRange{500, 700};
  int32_t m_numUpdatesSinceLastGpuFilterChange                       = 0;
  int32_t m_maxUpdatesBetweenGpuFilterChanges = MAX_UPDATES_BETWEEN_GPU_FILTER_CHANGES_RANGE.min;

  uint32_t m_previousZoomSpeed = Speed::STOP_SPEED;

  // Changement d'effet de zoom !
  auto CheckIfFilterModeChanged() -> void;
  auto CheckIfGpuFilterModeChanged() -> void;
  auto ChangeStateMaybe() -> void;
  auto ChangeFilterModeMaybe() -> void;
  auto ChangeGpuFilterModeMaybe() -> void;
  auto ColorChangeIfMusicIsCalm() -> void;
  auto BigUpdateIfNotLocked() -> void;
  auto LowerTheSpeedMaybe() -> void;
  auto ChangeRotationMaybe() -> void;
  auto ChangeFilterExtraSettingsMaybe() -> void;
  auto ChangeFilterLerpFactorMaybe() -> void;
  auto ChangeFilterLerpFactorToEndMaybe() -> void;
  auto ChangeGpuLerpFactorMaybe() -> void;
  auto ChangeFilterSpeedMaybe() -> void;
  auto ChangeStopSpeedsMaybe() -> void;
  auto RestoreZoomInMaybe() -> void;

  auto DoChangeState() -> void;
  auto DoBigUpdateMaybe() -> void;
  auto DoBigNormalUpdate() -> void;
  auto DoMegaLentUpdate() -> void;
  auto DoColorChange() -> void;
  auto DoChangeFilterMode() -> void;
  auto DoChangeGpuFilterMode() -> void;
  auto DoChangeFilterExtraSettings() -> void;
  auto DoChangeRotationMaybe() -> void;
  auto DoLowerTheSpeed() -> void;
  auto DoChangeSpeedSlowAndForward() -> void;
  auto DoChangeSpeedReverse() -> void;
  auto DoSetSlowerSpeedAndToggleReverse() -> void;
  auto DoChangeSpeed(uint32_t currentSpeed, uint32_t newSpeed) -> void;
  auto DoChangeGpuLerpFactor() -> void;
  auto DoResetGpuLerpFactorBasedOnSpeed() -> void;
  auto DoChangeFilterLerpFactor() -> void;
  auto DoResetFilterLerpFactorToEnd() -> void;
  auto DoResetFilterLerpFactor() -> void;
  auto DoResetFilterLerpFactorBasedOnSpeed() -> void;

  struct ChangeEventData
  {
    uint64_t eventTime                         = 0U;
    int32_t numUpdatesSinceLastFilterChange    = 0;
    int32_t numUpdatesSinceLastGpuFilterChange = 0;
    int32_t maxUpdatesBetweenFilterChanges     = 0;
    int32_t maxUpdatesBetweenGpuFilterChanges  = 0;
    uint32_t lockTime                          = 0U;
    uint32_t timeInState                       = 0U;
    uint32_t previousZoomSpeed                 = 0;
    uint32_t currentZoomSpeed                  = 0;
    uint32_t timeSinceLastGoom                 = 0U;
    uint32_t totalGoomsInThisCycle             = 0U;
    float soundSpeed                           = 0.0F;
    uint32_t filterMode                        = 0U;
    uint32_t gpuFilterMode                     = 0U;
    uint32_t state                             = 0U;
    float filterLerpFactor                     = 0.0F;
    float gpuLerpFactor                        = 0.0F;
    bool filterModeChangedInThisUpdate         = false;
    std::vector<ChangeEvents> changeEvents;
  };
  static constexpr auto NUM_EVENT_DATA_TO_RESERVE = 500U;
  std::vector<ChangeEventData> m_allChangeEvents;
  std::vector<std::string_view> m_usedStates;
  uint32_t m_currentState = 0U;
  auto ClearChangeEventData() noexcept -> void;
  auto UpdateChangeEventData() noexcept -> void;
  auto DumpChangeEventData() -> void;
  auto LogChangeState() noexcept -> void;
  auto LogChangeEvent(ChangeEvents changeEvent) noexcept -> void;
};

GoomMusicSettingsReactor::GoomMusicSettingsReactor(
    const PluginInfo& goomInfo,
    const GoomRand& goomRand,
    GoomAllVisualFx& visualFx,
    FilterSettingsService& filterSettingsService) noexcept
  : m_pimpl{spimpl::make_unique_impl<GoomMusicSettingsReactorImpl>(
        goomInfo, goomRand, visualFx, filterSettingsService)}
{
}

auto GoomMusicSettingsReactor::SetDumpDirectory(const std::string& dumpDirectory) -> void
{
  m_pimpl->SetDumpDirectory(dumpDirectory);
}

auto GoomMusicSettingsReactor::Start() -> void
{
  m_pimpl->Start();
}

auto GoomMusicSettingsReactor::Finish() -> void
{
  m_pimpl->Finish();
}

auto GoomMusicSettingsReactor::NewCycle() -> void
{
  m_pimpl->NewCycle();
}

auto GoomMusicSettingsReactor::UpdateSettings() -> void
{
  m_pimpl->UpdateSettings();
}

auto GoomMusicSettingsReactor::GetNameValueParams() const -> NameValuePairs
{
  return m_pimpl->GetNameValueParams();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::LogChangeState() noexcept -> void
{
  if constexpr (COLLECT_EVENT_STATS)
  {
    if (const auto iter = std::ranges::find(m_usedStates, m_visualFx->GetCurrentStateName());
        iter != m_usedStates.end())
    {
      m_currentState = static_cast<uint32_t>(iter - m_usedStates.begin());
    }
    {
      m_currentState = static_cast<uint32_t>(m_usedStates.size());
      m_usedStates.emplace_back(m_visualFx->GetCurrentStateName());
    }
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::LogChangeEvent(
    const ChangeEvents changeEvent) noexcept -> void
{
  if constexpr (COLLECT_EVENT_STATS)
  {
    m_allChangeEvents.back().changeEvents.emplace_back(changeEvent);
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ClearChangeEventData() noexcept -> void
{
  if constexpr (COLLECT_EVENT_STATS)
  {
    m_allChangeEvents.clear();
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::UpdateChangeEventData() noexcept
    -> void
{
  if constexpr (COLLECT_EVENT_STATS)
  {
    if (m_allChangeEvents.capacity() == m_allChangeEvents.size())
    {
      m_allChangeEvents.reserve(m_allChangeEvents.size() + NUM_EVENT_DATA_TO_RESERVE);
    }

    // clang-format off
    m_allChangeEvents.emplace_back(ChangeEventData{
        .eventTime                          = m_goomInfo->GetTime().GetCurrentTime(),
        .numUpdatesSinceLastFilterChange    = m_numUpdatesSinceLastFilterChange,
        .numUpdatesSinceLastGpuFilterChange = m_numUpdatesSinceLastGpuFilterChange,
        .maxUpdatesBetweenFilterChanges     = m_maxUpdatesBetweenFilterChanges,
        .maxUpdatesBetweenGpuFilterChanges  = m_maxUpdatesBetweenGpuFilterChanges,
        .lockTime                           = m_changeLock.GetLockTime(),
        .timeInState                        = m_timeInState,
        .previousZoomSpeed                  = m_previousZoomSpeed,
        .currentZoomSpeed      = m_filterSettingsService->GetReadOnlyFilterSpeed().GetSpeed(),
        .timeSinceLastGoom     = m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom(),
        .totalGoomsInThisCycle = m_goomInfo->GetSoundEvents().GetTotalGoomsInCurrentCycle(),
        .soundSpeed            = m_goomInfo->GetSoundEvents().GetSoundInfo().GetSpeed(),
        .filterMode    = static_cast<uint32_t>(m_filterSettingsService->GetCurrentFilterMode()),
        .gpuFilterMode = static_cast<uint32_t>(m_filterSettingsService->GetCurrentGpuFilterMode()),
        .state         = m_currentState,
        .filterLerpFactor = m_filterSettingsService->GetCurrentFilterBufferLerpFactor(),
        .gpuLerpFactor    = m_filterSettingsService->GetCurrentGpuLerpFactor(),
        .filterModeChangedInThisUpdate = m_hasFilterModeChangedInThisUpdate,
        .changeEvents                  = {},
    });
    // clang-format on
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DumpChangeEventData() -> void
{
  if constexpr (COLLECT_EVENT_STATS)
  {
    if (static constexpr auto DONT_CARE_SMALL_TIME = 10U;
        m_allChangeEvents.empty() or m_dumpDirectory.empty() or
        (m_goomInfo->GetTime().GetCurrentTime() < DONT_CARE_SMALL_TIME))
    {
      return;
    }

    const auto dumpFilepath = join_paths(
        m_dumpDirectory, std::format("reactor-stats-{}.txt", GetCurrentDateTimeAsString()));
    auto outFile = std::ofstream(dumpFilepath);
    if (not outFile)
    {
      throw std::runtime_error(std::format("Could not open dump stats file \"{}\".", dumpFilepath));
    }
    std::println(outFile,
                 "{:>8s}"
                 " {:>8s}"
                 " {:>8s}"
                 " {:>8s}"
                 " {:>8s}"
                 " {:>8s}"
                 " {:>8s}"
                 " {:>8s} {:>8s} {:>8s}"
                 " {:>8s} {:>8s} {:>8s} {:>8s} {:>8s}"
                 " {:>8s}"
                 "  {}",
                 "TIME",

                 "N_UPDS",

                 "MX_UPDS",

                 "LK_TIME",

                 "IN_STATE",

                 "PRV_SPD",

                 "CUR_SPD",

                 "LAST_GM",
                 "TOT_GM",
                 "SND_SPD",

                 "STATE",
                 "FILTER",
                 "GPU",
                 "FIL_LERP",
                 "GPU_LERP",

                 "FIL_CHG",

                 "EVENTS");
    for (const auto& eventData : m_allChangeEvents)
    {
      std::println(outFile,
                   "{:8d}"
                   " {:8d}"
                   " {:8d}"
                   " {:8d}"
                   " {:8d}"
                   " {:8d}"
                   " {:8d}"
                   " {:8d} {:8d} {:8.1f}"
                   " {:8d} {:8d} {:8d} {:8.1f} {:8.1f}"
                   " {:8}"
                   "  {}",
                   eventData.eventTime,

                   eventData.numUpdatesSinceLastFilterChange,

                   eventData.maxUpdatesBetweenFilterChanges,

                   eventData.lockTime,

                   eventData.timeInState,

                   eventData.previousZoomSpeed,

                   eventData.currentZoomSpeed,

                   eventData.timeSinceLastGoom,
                   eventData.totalGoomsInThisCycle,
                   eventData.soundSpeed,

                   eventData.state,
                   eventData.filterMode,
                   eventData.gpuFilterMode,
                   eventData.filterLerpFactor,
                   eventData.gpuLerpFactor,

                   eventData.filterModeChangedInThisUpdate,

                   StringJoin(ToStrings(eventData.changeEvents,
                                        [](const auto event) { return EnumToString(event); }),
                              ", "));
    }
  }
}

GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::GoomMusicSettingsReactorImpl(
    const PluginInfo& goomInfo,
    const GoomRand& goomRand,
    GoomAllVisualFx& visualFx,
    FilterSettingsService& filterSettingsService) noexcept
  : m_goomInfo{&goomInfo},
    m_goomRand{&goomRand},
    m_visualFx{&visualFx},
    m_filterSettingsService{&filterSettingsService}
{
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::SetDumpDirectory(
    const std::string& dumpDirectory) -> void
{
  m_dumpDirectory = dumpDirectory;
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::Start() -> void
{
  m_changeLock.SetLockTime(LOCK_TIME_NORMAL_UPDATE);

  m_timeInState                        = 0;
  m_numUpdatesSinceLastFilterChange    = 0;
  m_numUpdatesSinceLastGpuFilterChange = 0;
  m_maxUpdatesBetweenFilterChanges     = MAX_UPDATES_BETWEEN_FILTER_CHANGES_RANGE.min;
  m_maxUpdatesBetweenGpuFilterChanges  = MAX_UPDATES_BETWEEN_GPU_FILTER_CHANGES_RANGE.min;
  m_previousZoomSpeed                  = Speed::STOP_SPEED;

  ClearChangeEventData();
  UpdateChangeEventData();

  DoChangeState();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::Finish() -> void
{
  DumpChangeEventData();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::NewCycle() -> void
{
  ++m_timeInState;

  m_changeLock.Update();

  m_hasFilterModeChangedInThisUpdate    = false;
  m_hasGpuFilterModeChangedInThisUpdate = false;
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::UpdateSettings() -> void
{
  UpdateChangeEventData();

  ChangeFilterModeMaybe();
  ChangeGpuFilterModeMaybe();
  BigUpdateIfNotLocked();
  ColorChangeIfMusicIsCalm();

  LowerTheSpeedMaybe();

  ChangeFilterLerpFactorMaybe();
  ChangeGpuLerpFactorMaybe();
  ChangeRotationMaybe();

  CheckIfFilterModeChanged();
  CheckIfGpuFilterModeChanged();

  m_previousZoomSpeed = m_filterSettingsService->GetReadOnlyFilterSpeed().GetSpeed();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::CheckIfFilterModeChanged() -> void
{
  if (m_hasFilterModeChangedInThisUpdate)
  {
    m_numUpdatesSinceLastFilterChange = 0;
    m_visualFx->RefreshAllFx();
    return;
  }

  ++m_numUpdatesSinceLastFilterChange;
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::CheckIfGpuFilterModeChanged() -> void
{
  if (m_hasGpuFilterModeChangedInThisUpdate)
  {
    m_numUpdatesSinceLastGpuFilterChange = 0;
    return;
  }

  ++m_numUpdatesSinceLastGpuFilterChange;
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeStateMaybe() -> void
{
  if (not m_goomRand->ProbabilityOf<PROB_CHANGE_STATE>())
  {
    return;
  }

  DoChangeState();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeFilterModeMaybe() -> void
{
  if ((m_numUpdatesSinceLastFilterChange < m_maxUpdatesBetweenFilterChanges) and
      ((m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom() > 0) or
       (not m_goomRand->ProbabilityOf<PROB_CHANGE_FILTER_MODE>())))
  {
    return;
  }

  DoChangeFilterMode();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeGpuFilterModeMaybe() -> void
{
  if ((m_numUpdatesSinceLastGpuFilterChange < m_maxUpdatesBetweenGpuFilterChanges) and
      ((m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom() > 0) or
       (not m_goomRand->ProbabilityOf<PROB_CHANGE_GPU_FILTER_MODE>())))
  {
    return;
  }

  DoChangeGpuFilterMode();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ColorChangeIfMusicIsCalm() -> void
{
  static constexpr auto CALM_SOUND_SPEED = 0.3F;
  static constexpr auto CALM_CYCLES      = 16U;

  if ((m_goomInfo->GetSoundEvents().GetSoundInfo().GetSpeed() > CALM_SOUND_SPEED) or
      (not m_filterSettingsService->GetReadOnlyFilterSpeed().IsFasterThan(Speed::CALM_SPEED)) or
      ((m_goomInfo->GetTime().GetCurrentTime() % CALM_CYCLES) != 0))
  {
    return;
  }

  DoColorChange();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::BigUpdateIfNotLocked() -> void
{
  if (m_changeLock.IsLocked())
  {
    return;
  }

  DoBigUpdateMaybe();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::LowerTheSpeedMaybe() -> void
{
  if (static constexpr auto LOWER_SPEED_CYCLES = 73U;
      ((m_goomInfo->GetTime().GetCurrentTime() % LOWER_SPEED_CYCLES) != 0) or
      (not m_filterSettingsService->GetReadOnlyFilterSpeed().IsFasterThan(Speed::FAST_SPEED)))
  {
    return;
  }

  DoLowerTheSpeed();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeFilterLerpFactorMaybe() -> void
{
  if (not m_hasFilterModeChangedInThisUpdate)
  {
    return;
  }

  DoChangeFilterLerpFactor();
  m_filterSettingsService->SlowDownGpuLerpFactorABit();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeFilterLerpFactorToEndMaybe()
    -> void
{
  if (m_changeLock.GetLockTime() < LOCK_TIME_CHANGE_LERP_FACTOR_TO_END)
  {
    return;
  }

  DoResetFilterLerpFactorToEnd();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeGpuLerpFactorMaybe() -> void
{
  if (not m_hasGpuFilterModeChangedInThisUpdate)
  {
    return;
  }

  DoChangeGpuLerpFactor();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeRotationMaybe() -> void
{
  if (m_numUpdatesSinceLastFilterChange < m_maxUpdatesBetweenFilterChanges)
  {
    return;
  }

  DoChangeRotationMaybe();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeFilterExtraSettingsMaybe()
    -> void
{
  if (not m_goomRand->ProbabilityOf<PROB_CHANGE_FILTER_EXTRA_SETTINGS>())
  {
    return;
  }

  DoChangeFilterExtraSettings();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::RestoreZoomInMaybe() -> void
{
  // Retablir le zoom avant.
  // Restore zoom in.

  if (static constexpr auto REVERSE_SPEED_CYCLES = 13U;
      (m_filterSettingsService->GetReadOnlyFilterSpeed().IsSpeedReversed()) and
      ((m_goomInfo->GetTime().GetCurrentTime() % REVERSE_SPEED_CYCLES) != 0) and
      m_goomRand->ProbabilityOf<PROB_FILTER_REVERSE_OFF_AND_STOP_SPEED>())
  {
    DoChangeSpeedSlowAndForward();
  }
  if (m_goomRand->ProbabilityOf<PROB_FILTER_REVERSE_ON>())
  {
    DoChangeSpeedReverse();
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeStopSpeedsMaybe() -> void
{
  if (m_goomRand->ProbabilityOf<PROB_FILTER_SPEED_STOP_SPEED_MINUS_1>())
  {
    LogChangeEvent(SET_SLOW_SPEED);
    m_filterSettingsService->GetReadWriteFilterSpeed().SetSpeed(Speed::SLOWEST_SPEED);
  }
  else if (m_goomRand->ProbabilityOf<PROB_FILTER_SPEED_STOP_SPEED>())
  {
    LogChangeEvent(SET_STOP_SPEED);
    m_filterSettingsService->GetReadWriteFilterSpeed().SetSpeed(Speed::STOP_SPEED);
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeFilterSpeedMaybe() -> void
{
  const auto soundSpeed = m_goomInfo->GetSoundEvents().GetSoundInfo().GetSpeed();

  static constexpr auto MIN_USABLE_SOUND_SPEED = SoundInfo::SPEED_MIDPOINT - 0.1F;
  static constexpr auto MAX_USABLE_SOUND_SPEED = SoundInfo::SPEED_MIDPOINT + 0.1F;
  const auto usableRelativeSoundSpeed =
      (std::clamp(soundSpeed, MIN_USABLE_SOUND_SPEED, MAX_USABLE_SOUND_SPEED) -
       MIN_USABLE_SOUND_SPEED) /
      (MAX_USABLE_SOUND_SPEED - MIN_USABLE_SOUND_SPEED);

  static constexpr auto MAX_SPEED_CHANGE = 10U;
  const auto newSpeedVal                 = static_cast<uint32_t>(
      std::lerp(0.0F, static_cast<float>(MAX_SPEED_CHANGE), usableRelativeSoundSpeed));

  auto& filterSpeed = m_filterSettingsService->GetReadWriteFilterSpeed();

  const auto currentSpeed = filterSpeed.GetSpeed();
  const auto newSpeed     = Speed::GetFasterBy(Speed::STOP_SPEED, newSpeedVal);

  if (currentSpeed > newSpeed)
  {
    // Current speed is faster than new one. Nothing to do.
    return;
  }

  // on accelere
  if (static constexpr auto FILTER_SPEED_CYCLES = 3U;
      ((currentSpeed > Speed::FASTER_SPEED) and (newSpeed > Speed::EVEN_FASTER_SPEED) and
       (0 == (m_goomInfo->GetTime().GetCurrentTime() % FILTER_SPEED_CYCLES))) or
      m_goomRand->ProbabilityOf<PROB_FILTER_CHANGE_SPEED_AND_TOGGLE_REVERSE>())
  {
    LogChangeEvent(SET_SLOWER_SPEED_AND_TOGGLE_REVERSE);
    DoSetSlowerSpeedAndToggleReverse();
  }
  else
  {
    DoChangeSpeed(currentSpeed, newSpeed);
  }

  m_changeLock.IncreaseLockTime(LOCK_TIME_SPEED_CHANGE_INCREASE);
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeState() -> void
{
  LogChangeEvent(CHANGE_STATE);

  m_visualFx->SetNextState();
  m_visualFx->ChangeAllFxColorMaps();

  m_timeInState = 0;

  LogChangeState();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoBigUpdateMaybe() -> void
{
  // Reperage de goom (acceleration forte de l'acceleration du volume).
  // Coup de boost de la vitesse si besoin.
  // Goom tracking (strong acceleration of volume acceleration).
  // Speed boost if needed.
  if (0 == m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom())
  {
    DoBigNormalUpdate();
  }

  // mode mega-lent
  if (m_goomRand->ProbabilityOf<PROB_CHANGE_TO_MEGA_LENT_MODE>())
  {
    DoMegaLentUpdate();
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoBigNormalUpdate() -> void
{
  LogChangeEvent(BIG_NORMAL_UPDATE);

  m_changeLock.SetLockTime(LOCK_TIME_NORMAL_UPDATE);

  ChangeStateMaybe();
  RestoreZoomInMaybe();
  ChangeStopSpeedsMaybe();
  ChangeRotationMaybe();
  ChangeFilterExtraSettingsMaybe();
  ChangeFilterSpeedMaybe();
  ChangeFilterLerpFactorToEndMaybe();
  m_visualFx->ChangeAllFxColorMaps();

  m_maxUpdatesBetweenFilterChanges =
      m_goomRand->GetRandInRange<MAX_UPDATES_BETWEEN_FILTER_CHANGES_RANGE>();
  m_maxUpdatesBetweenGpuFilterChanges =
      m_goomRand->GetRandInRange<MAX_UPDATES_BETWEEN_GPU_FILTER_CHANGES_RANGE>();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoMegaLentUpdate() -> void
{
  LogChangeEvent(MEGA_LENT_UPDATE);

  m_changeLock.IncreaseLockTime(LOCK_TIME_MEGA_LENT_INCREASE);

  m_visualFx->ChangeAllFxColorMaps();
  m_filterSettingsService->GetReadWriteFilterSpeed().SetSpeed(Speed::SLOWEST_SPEED);

  DoResetFilterLerpFactorToEnd();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoColorChange() -> void
{
  LogChangeEvent(BIG_BREAK);

  static constexpr auto SLOWER_BY = 3U;
  m_filterSettingsService->GetReadWriteFilterSpeed().GoSlowerBy(SLOWER_BY);

  m_visualFx->ChangeAllFxColorMaps();
  m_visualFx->ChangeAllFxPixelBlenders();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeFilterMode() -> void
{
  if (not m_filterSettingsService->SetNewRandomFilter())
  {
    m_numUpdatesSinceLastFilterChange = 0;
    LogChangeEvent(CHANGE_FILTER_MODE_REJECTED);
    return;
  }

  LogChangeEvent(CHANGE_FILTER_MODE);
  m_numUpdatesSinceLastFilterChange  = 0;
  m_hasFilterModeChangedInThisUpdate = true;
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeGpuFilterMode() -> void
{
  if (not m_filterSettingsService->SetNewRandomGpuFilter(m_maxUpdatesBetweenFilterChanges))
  {
    m_numUpdatesSinceLastGpuFilterChange = 0;
    LogChangeEvent(CHANGE_GPU_FILTER_MODE_REJECTED);
    return;
  }

  LogChangeEvent(CHANGE_GPU_FILTER_MODE);
  m_numUpdatesSinceLastGpuFilterChange  = 0;
  m_hasGpuFilterModeChangedInThisUpdate = true;

#ifdef DEBUG_WITH_PRINTLN
  std::println("Changed GPU filter.");
#endif
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeFilterExtraSettings() -> void
{
  if (not m_filterSettingsService->ChangeMilieu())
  {
    LogChangeEvent(CHANGE_FILTER_EXTRA_SETTINGS_REJECTED);
    return;
  }

  m_filterSettingsService->SetRandomTextureWrapType();

  LogChangeEvent(CHANGE_FILTER_EXTRA_SETTINGS);
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeRotationMaybe() -> void
{
  LogChangeEvent(CHANGE_ROTATION);

  if (m_goomRand->ProbabilityOf<PROB_FILTER_STOP_ROTATION>())
  {
    if (not m_filterSettingsService->TurnOffRotation())
    {
      LogChangeEvent(TURN_OFF_ROTATION_REJECTED);
      return;
    }
    LogChangeEvent(TURN_OFF_ROTATION);
  }
  else if (m_goomRand->ProbabilityOf<PROB_FILTER_DECREASE_ROTATION>())
  {
    static constexpr auto ROTATE_SLOWER_FACTOR = 0.9F;
    if (not m_filterSettingsService->MultiplyRotation(ROTATE_SLOWER_FACTOR))
    {
      LogChangeEvent(SLOWER_ROTATION_REJECTED);
      return;
    }
    LogChangeEvent(SLOWER_ROTATION);
  }
  else if (m_goomRand->ProbabilityOf<PROB_FILTER_INCREASE_ROTATION>())
  {
    static constexpr auto ROTATE_FASTER_FACTOR = 1.1F;
    if (not m_filterSettingsService->MultiplyRotation(ROTATE_FASTER_FACTOR))
    {
      LogChangeEvent(FASTER_ROTATION_REJECTED);
      return;
    }
    LogChangeEvent(FASTER_ROTATION);
  }
  else if (m_goomRand->ProbabilityOf<PROB_FILTER_TOGGLE_ROTATION>())
  {
    if (not m_filterSettingsService->ToggleRotationDirection())
    {
      LogChangeEvent(TOGGLE_ROTATION_REJECTED);
      return;
    }
    LogChangeEvent(TOGGLE_ROTATION);
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoLowerTheSpeed() -> void
{
  LogChangeEvent(GO_SLOWER);
  m_filterSettingsService->GetReadWriteFilterSpeed().GoSlowerBy(1U);
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeSpeedSlowAndForward() -> void
{
  LogChangeEvent(SET_SLOWER_SPEED_AND_SPEED_FORWARD);

  m_filterSettingsService->GetReadWriteFilterSpeed().SetSpeedReversed(false);
  m_filterSettingsService->GetReadWriteFilterSpeed().SetSpeed(Speed::SLOW_SPEED);
  m_changeLock.SetLockTime(LOCK_TIME_SLOWER_SPEED_AND_SPEED_FORWARD);
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeSpeedReverse() -> void
{
  LogChangeEvent(SET_SPEED_REVERSE);

  m_filterSettingsService->GetReadWriteFilterSpeed().SetSpeedReversed(true);
  m_changeLock.SetLockTime(LOCK_TIME_REVERSE_SPEED);
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoSetSlowerSpeedAndToggleReverse()
    -> void
{
  LogChangeEvent(SET_SLOWER_SPEED_AND_TOGGLE_REVERSE);

  auto& filterSpeed = m_filterSettingsService->GetReadWriteFilterSpeed();
  filterSpeed.SetSpeed(Speed::SLOWEST_SPEED);
  filterSpeed.ToggleSpeedReversedFlag();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeSpeed(
    const uint32_t currentSpeed, const uint32_t newSpeed) -> void
{
  LogChangeEvent(CHANGE_SPEED);

  auto& filterSpeed                          = m_filterSettingsService->GetReadWriteFilterSpeed();
  static constexpr auto OLD_TO_NEW_SPEED_MIX = 0.4F;
  filterSpeed.SetSpeed(static_cast<uint32_t>(std::lerp(
      static_cast<float>(currentSpeed), static_cast<float>(newSpeed), OLD_TO_NEW_SPEED_MIX)));
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeGpuLerpFactor() -> void
{
#ifdef DEBUG_WITH_PRINTLN
  std::println("Changing GPU lerp data.");
#endif

  LogChangeEvent(CHANGE_GPU_LERP_FACTOR);

  static constexpr auto MIN_TIME_SINCE_LAST_GOOM                = 10U;
  static constexpr auto NUM_CYCLES_BEFORE_GPU_LERP_SPEED_CHANGE = 2U;
  if ((m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom() > MIN_TIME_SINCE_LAST_GOOM) or
      (m_goomInfo->GetSoundEvents().GetTotalGoomsInCurrentCycle() >=
       NUM_CYCLES_BEFORE_GPU_LERP_SPEED_CHANGE))
  {
    DoResetGpuLerpFactorBasedOnSpeed();
  }
  else
  {
    m_filterSettingsService->SpeedUpGpuLerpFactorABit();
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoResetGpuLerpFactorBasedOnSpeed()
    -> void
{
  LogChangeEvent(RESET_GPU_LERP_FACTOR_BASED_ON_SPEED);

  if (m_filterSettingsService->GetReadOnlyFilterSpeed().GetSpeed() < m_previousZoomSpeed)
  {
    m_filterSettingsService->SlowDownGpuLerpFactorABit();
  }
  else if (m_filterSettingsService->GetReadOnlyFilterSpeed().GetSpeed() > m_previousZoomSpeed)
  {
    m_filterSettingsService->SpeedUpGpuLerpFactorABit();
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeFilterLerpFactor() -> void
{
  LogChangeEvent(CHANGE_FILTER_LERP_FACTOR);

  static constexpr auto MIN_TIME_SINCE_LAST_GOOM            = 10U;
  static constexpr auto NUM_CYCLES_BEFORE_LERP_SPEED_CHANGE = 2U;
  if ((m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom() > MIN_TIME_SINCE_LAST_GOOM) or
      (m_goomInfo->GetSoundEvents().GetTotalGoomsInCurrentCycle() >=
       NUM_CYCLES_BEFORE_LERP_SPEED_CHANGE))
  {
    DoResetFilterLerpFactorBasedOnSpeed();
  }
  else
  {
    DoResetFilterLerpFactor();

    DoChangeRotationMaybe();
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoResetFilterLerpFactorToEnd() -> void
{
  LogChangeEvent(RESET_FILTER_LERP_FACTOR_TO_END);

  m_filterSettingsService->SetDefaultFilterBufferLerpIncrement();
  m_filterSettingsService->SetFilterBufferLerpToEnd();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoResetFilterLerpFactor() -> void
{
  LogChangeEvent(RESET_FILTER_LERP_FACTOR);
  m_filterSettingsService->ResetFilterBufferLerpData();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoResetFilterLerpFactorBasedOnSpeed()
    -> void
{
  LogChangeEvent(RESET_FILTER_LERP_FACTOR_BASED_ON_SPEED);

  m_filterSettingsService->SetDefaultFilterBufferLerpIncrement();

  auto diff = static_cast<float>(m_filterSettingsService->GetReadOnlyFilterSpeed().GetSpeed()) -
              static_cast<float>(m_previousZoomSpeed);
  if (diff < 0.0F)
  {
    diff = -diff;
  }
  if (static constexpr auto DIFF_CUT = 2.0F; diff > DIFF_CUT)
  {
    const auto lerpIncFactor = (diff + DIFF_CUT) / DIFF_CUT;
    m_filterSettingsService->MultiplyFilterBufferLerpIncrement(lerpIncFactor);
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::GetNameValueParams() const
    -> NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "Music Settings";
  return {
      GetPair(PARAM_GROUP, "speed", m_filterSettingsService->GetReadOnlyFilterSpeed().GetSpeed()),
      GetPair(PARAM_GROUP, "previousZoomSpeed", m_previousZoomSpeed),
      GetPair(PARAM_GROUP,
              "reverse",
              m_filterSettingsService->GetReadOnlyFilterSpeed().IsSpeedReversed()),
      GetPair(PARAM_GROUP,
              "relative speed",
              m_filterSettingsService->GetReadOnlyFilterSpeed().GetRelativeSpeed()),
      GetPair(PARAM_GROUP, "updatesSinceLastChange", m_numUpdatesSinceLastFilterChange),
      GetPair(PARAM_GROUP, "gpuUpdatesSinceLastChange", m_numUpdatesSinceLastGpuFilterChange),
  };
}

} // namespace GOOM::CONTROL
