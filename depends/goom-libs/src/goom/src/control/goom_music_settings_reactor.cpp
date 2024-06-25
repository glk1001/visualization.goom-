module;

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <format>
#include <fstream>
#include <print>
#include <stdexcept>
#include <string>
#include <utility>

module Goom.Control.GoomMusicSettingsReactor;

import Goom.Control.GoomAllVisualFx;
import Goom.FilterFx.FilterSettingsService;
import Goom.FilterFx.FilterSpeed;
import Goom.Lib.AssertUtils;
import Goom.Lib.SoundInfo;
import Goom.Lib.GoomPaths;
import Goom.Lib.GoomTypes;
import Goom.Utils.EnumUtils;
import Goom.Utils.DateUtils;
import Goom.PluginInfo;
import :GoomLock;

namespace GOOM::CONTROL
{

using FILTER_FX::FilterSettingsService;
using FILTER_FX::Vitesse;
using UTILS::EnumToString;
using UTILS::GetCurrentDateTimeAsString;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::NUM;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::NumberRange;

class GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl
{
public:
  GoomMusicSettingsReactorImpl(const PluginInfo& goomInfo,
                               const IGoomRand& goomRand,
                               GoomAllVisualFx& visualFx,
                               FilterSettingsService& filterSettingsService) noexcept;

  auto SetDumpDirectory(const std::string& dumpDirectory) -> void;

  auto Start() -> void;
  auto Finish() -> void;
  auto NewCycle() -> void;

  auto UpdateSettings() -> void;

  [[nodiscard]] auto GetNameValueParams() const -> UTILS::NameValuePairs;

private:
  const PluginInfo* m_goomInfo;
  const IGoomRand* m_goomRand;
  GoomAllVisualFx* m_visualFx;
  FilterSettingsService* m_filterSettingsService;
  std::string m_dumpDirectory;

  enum class ChangeEvents : UnderlyingEnumType
  {
    CHANGE_FILTER_MODE,
    BIG_UPDATE,
    BIG_BREAK,
    GO_SLOWER,
    UPDATE_TRANSFORM_BUFFER_LERP_DATA,
    CHANGE_ROTATION,

    UPDATE_FILTER_SETTINGS_NOW,

    MEGA_LENT_UPDATE,

    BIG_NORMAL_UPDATE,
    CHANGE_STATE,
    SET_SPEED_REVERSE,
    SET_SLOW_SPEED,
    SET_STOP_SPEED,
    TURN_OFF_ROTATION,
    SLOWER_ROTATION,
    FASTER_ROTATION,
    TOGGLE_ROTATION,
    CHANGE_FILTER_EXTRA_SETTINGS,
    SET_SLOWER_SPEED_AND_SPEED_FORWARD,
    SET_SLOWER_SPEED_AND_TOGGLE_REVERSE,
    CHANGE_SPEED,
    SET_LERP_TO_END,
    SET_NEW_LERP_DATA_BASED_ON_SPEED,
    RESET_LERP_DATA,
    _num // unused, and marks the enum end
  };
  using enum ChangeEvents;
  struct ChangeEventData
  {
    uint64_t eventTime                              = 0U;
    int32_t numUpdatesSinceLastFilterSettingsChange = 0;
    int32_t maxTimeBetweenFilterSettingsChange      = 0;
    uint32_t lockTime                               = 0U;
    uint32_t previousZoomSpeed                      = 0;
    uint32_t timeInState                            = 0U;
    uint32_t timeSinceLastGoom                      = 0U;
    uint32_t totalGoomsInCurrentCycle               = 0U;
    float soundSpeed                                = 0.0F;
    bool filterModeChangedSinceLastUpdate           = true;
    std::vector<ChangeEvents> changeEvents{};
  };
  static constexpr auto EVENT_DATA_TO_RESERVE_SIZE = 500U;
  std::vector<ChangeEventData> m_allChangeEvents{};
  ChangeEventData* m_currentChangeEventData{};
  auto UpdateChangeEventData() noexcept -> void;
  auto LogChangeEvent(ChangeEvents changeEvent) noexcept -> void;
  auto ClearChangeEventData() noexcept -> void;
  auto DumpChangeEventData() -> void;
  [[nodiscard]] auto GetChangeEventsStr(const std::vector<ChangeEvents>& changeEvents) const
      -> std::string;

  static constexpr auto NORMAL_UPDATE_LOCK_TIME                  = 50U;
  static constexpr auto SLOWER_SPEED_AND_SPEED_FORWARD_LOCK_TIME = 75U;
  static constexpr auto REVERSE_SPEED_LOCK_TIME                  = 100U;
  static constexpr auto MEGA_LENT_LOCK_TIME_INCREASE             = 50U;
  static constexpr auto CHANGE_VITESSE_LOCK_TIME_INCREASE        = 50U;
  static constexpr auto CHANGE_LERP_TO_END_LOCK_TIME             = 150U;
  GoomLock m_lock; // pour empecher de nouveaux changements

  static constexpr auto MAX_TIME_BETWEEN_FILTER_SETTINGS_CHANGE_RANGE = NumberRange{300, 500};
  int32_t m_maxTimeBetweenFilterSettingsChange = MAX_TIME_BETWEEN_FILTER_SETTINGS_CHANGE_RANGE.min;
  int32_t m_numUpdatesSinceLastFilterSettingsChange = 0;
  uint32_t m_previousZoomSpeed                      = FILTER_FX::Vitesse::STOP_SPEED;

  static constexpr auto MAX_NUM_STATE_SELECTIONS_BLOCKED = 3U;
  uint32_t m_stateSelectionBlocker                       = MAX_NUM_STATE_SELECTIONS_BLOCKED;
  uint32_t m_timeInState                                 = 0U;
  auto ChangeState() -> void;
  auto DoChangeState() -> void;

  auto BigBreakIfMusicIsCalm() -> void;
  auto BigUpdateIfNotLocked() -> void;
  auto BigUpdate() -> void;
  auto RegularlyLowerTheSpeed() -> void;
  auto CheckIfFilterModeChanged() -> void;
  auto ChangeFilterModeIfMusicChanges() -> void;
  auto ChangeFilterExtraSettings() -> void;
  auto ChangeRotation() -> void;
  auto ChangeTransformBufferLerpData() -> void;
  auto ChangeTransformBufferLerpToEnd() -> void;

  // Changement d'effet de zoom !
  [[nodiscard]] auto UpdateFilterSettingsNow() const noexcept -> bool;
  auto ChangeVitesse() -> void;
  auto ChangeStopSpeeds() -> void;
  auto RestoreZoomIn() -> void;

  auto DoBigNormalUpdate() -> void;
  auto DoMegaLentUpdate() -> void;
  auto DoBigBreak() -> void;
  auto DoChangeFilterMode() -> void;
  auto DoUpdateFilterSettingsNow() -> void;
  auto DoChangeFilterExtraSettings() -> void;
  auto DoUpdateTransformBufferLerpData() -> void;
  auto DoSetNewTransformBufferLerpDataBasedOnSpeed() -> void;
  auto DoSetTransformBufferLerpToEnd() -> void;
  auto DoResetTransformBufferLerpData() -> void;
  auto DoChangeRotation() -> void;
  auto DoRegularlyLowerTheSpeed() -> void;
  auto DoChangeSpeedSlowAndForward() -> void;
  auto DoChangeSpeedReverse() -> void;
  auto DoSetSlowerSpeedAndToggleReverse() -> void;
  auto DoChangeSpeed(uint32_t currentVitesse, uint32_t newVitesse) -> void;

  static constexpr auto PROB_CHANGE_FILTER_MODE                       = 0.05F;
  static constexpr auto PROB_CHANGE_STATE                             = 0.50F;
  static constexpr auto PROB_CHANGE_TO_MEGA_LENT_MODE                 = 1.0F / 700.F;
  static constexpr auto PROB_SLOW_FILTER_SETTINGS_UPDATE              = 0.5F;
  static constexpr auto PROB_FILTER_REVERSE_ON                        = 0.10F;
  static constexpr auto PROB_FILTER_REVERSE_OFF_AND_STOP_SPEED        = 0.20F;
  static constexpr auto PROB_FILTER_VITESSE_STOP_SPEED_MINUS_1        = 0.20F;
  static constexpr auto PROB_FILTER_VITESSE_STOP_SPEED                = 0.10F;
  static constexpr auto PROB_FILTER_CHANGE_VITESSE_AND_TOGGLE_REVERSE = 0.05F;
  static constexpr auto PROB_FILTER_TOGGLE_ROTATION                   = 0.125F;
  static constexpr auto PROB_FILTER_INCREASE_ROTATION                 = 0.25F;
  static constexpr auto PROB_FILTER_DECREASE_ROTATION                 = 0.875F;
  static constexpr auto PROB_FILTER_STOP_ROTATION                     = 0.25F;
};

GoomMusicSettingsReactor::GoomMusicSettingsReactor(
    const PluginInfo& goomInfo,
    const IGoomRand& goomRand,
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

static constexpr auto COLLECT_STATS = true;

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::LogChangeEvent(
    const ChangeEvents changeEvent) noexcept -> void
{
  if constexpr (COLLECT_STATS)
  {
    Expects(m_currentChangeEventData != nullptr);
    m_currentChangeEventData->changeEvents.emplace_back(changeEvent);
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ClearChangeEventData() noexcept -> void
{
  if constexpr (COLLECT_STATS)
  {
    m_allChangeEvents.clear();
    m_currentChangeEventData = nullptr;
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::UpdateChangeEventData() noexcept
    -> void
{
  if constexpr (COLLECT_STATS)
  {
    if (m_allChangeEvents.capacity() == m_allChangeEvents.size())
    {
      m_allChangeEvents.reserve(m_allChangeEvents.size() + EVENT_DATA_TO_RESERVE_SIZE);
    }

    m_currentChangeEventData = &m_allChangeEvents.emplace_back(ChangeEventData{
        .eventTime                               = m_goomInfo->GetTime().GetCurrentTime(),
        .numUpdatesSinceLastFilterSettingsChange = m_numUpdatesSinceLastFilterSettingsChange,
        .maxTimeBetweenFilterSettingsChange      = m_maxTimeBetweenFilterSettingsChange,
        .lockTime                                = m_lock.GetLockTime(),
        .previousZoomSpeed                       = m_previousZoomSpeed,
        .timeInState                             = m_timeInState,
        .timeSinceLastGoom        = m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom(),
        .totalGoomsInCurrentCycle = m_goomInfo->GetSoundEvents().GetTotalGoomsInCurrentCycle(),
        .soundSpeed               = m_goomInfo->GetSoundEvents().GetSoundInfo().GetSpeed(),
        .filterModeChangedSinceLastUpdate =
            m_filterSettingsService->HasFilterModeChangedSinceLastUpdate(),
    });
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DumpChangeEventData() -> void
{
  if constexpr (COLLECT_STATS)
  {
    if (static constexpr auto SMALL_TIME = 10U;
        m_allChangeEvents.empty() or m_dumpDirectory.empty() or
        (m_goomInfo->GetTime().GetCurrentTime() < SMALL_TIME))
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
                 "{:>7s} {:>7s} {:>7s} {:>7s} {:>7s} {:>7s} {:>7s} {:>7s} {:>7s} {:>7s} {}",
                 "TIME",
                 "N_UPDS",
                 "MX_UPDS",
                 "LK_TIME",
                 "PRV_SPD",
                 "IN_STAT",
                 "LAST_GM",
                 "TOT_GM",
                 "SND_SPD",
                 "FIL_CHG",
                 "EVENTS");
    for (const auto& eventData : m_allChangeEvents)
    {
      std::println(outFile,
                   "{:7d} {:7d} {:7d} {:7d} {:7d} {:7d} {:7d} {:7d} {:7.1f} {:7} {}",
                   eventData.eventTime,
                   eventData.numUpdatesSinceLastFilterSettingsChange,
                   eventData.maxTimeBetweenFilterSettingsChange,
                   eventData.lockTime,
                   eventData.previousZoomSpeed,
                   eventData.timeInState,
                   eventData.timeSinceLastGoom,
                   eventData.totalGoomsInCurrentCycle,
                   eventData.soundSpeed,
                   eventData.filterModeChangedSinceLastUpdate,
                   GetChangeEventsStr(eventData.changeEvents));
    }
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::GetChangeEventsStr(
    const std::vector<ChangeEvents>& changeEvents) const -> std::string
{
  auto eventsStr = std::string{};

  for (auto first = true; const auto& event : changeEvents)
  {
    eventsStr += std::format("{}{}", first ? "" : ", ", EnumToString(event));
    first = false;
  }

  return eventsStr;
}

GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::GoomMusicSettingsReactorImpl(
    const PluginInfo& goomInfo,
    const IGoomRand& goomRand,
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
  m_timeInState                             = 0;
  m_numUpdatesSinceLastFilterSettingsChange = 0;
  m_maxTimeBetweenFilterSettingsChange      = MAX_TIME_BETWEEN_FILTER_SETTINGS_CHANGE_RANGE.min;
  m_previousZoomSpeed                       = FILTER_FX::Vitesse::STOP_SPEED;

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
  m_lock.Update();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::UpdateSettings() -> void
{
  UpdateChangeEventData();

  ChangeFilterModeIfMusicChanges();
  BigUpdateIfNotLocked();
  BigBreakIfMusicIsCalm();

  RegularlyLowerTheSpeed();

  ChangeTransformBufferLerpData();
  ChangeRotation();

  CheckIfFilterModeChanged();

  m_previousZoomSpeed = m_filterSettingsService->GetROVitesse().GetVitesse();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::CheckIfFilterModeChanged() -> void
{
  ++m_numUpdatesSinceLastFilterSettingsChange;
  if ((m_numUpdatesSinceLastFilterSettingsChange <= m_maxTimeBetweenFilterSettingsChange) and
      not m_filterSettingsService->HasFilterModeChangedSinceLastUpdate())
  {
    return;
  }

  m_numUpdatesSinceLastFilterSettingsChange = 0;
  m_visualFx->RefreshAllFx();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeFilterModeIfMusicChanges()
    -> void
{
  if ((m_numUpdatesSinceLastFilterSettingsChange <= m_maxTimeBetweenFilterSettingsChange) and
      ((m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom() > 0) or
       (not m_goomRand->ProbabilityOf(PROB_CHANGE_FILTER_MODE))))
  {
    return;
  }

  DoChangeFilterMode();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::BigUpdateIfNotLocked() -> void
{
  if (m_lock.IsLocked())
  {
    return;
  }

  BigUpdate();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::BigBreakIfMusicIsCalm() -> void
{
  static constexpr auto CALM_SOUND_SPEED = 0.3F;
  static constexpr auto CALM_CYCLES      = 16U;

  if ((m_goomInfo->GetSoundEvents().GetSoundInfo().GetSpeed() > CALM_SOUND_SPEED) or
      (not m_filterSettingsService->GetROVitesse().IsFasterThan(FILTER_FX::Vitesse::CALM_SPEED)) or
      ((m_goomInfo->GetTime().GetCurrentTime() % CALM_CYCLES) != 0))
  {
    return;
  }

  DoBigBreak();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::RegularlyLowerTheSpeed() -> void
{
  if (static constexpr auto LOWER_SPEED_CYCLES = 73U;
      ((m_goomInfo->GetTime().GetCurrentTime() % LOWER_SPEED_CYCLES) != 0) or
      (not m_filterSettingsService->GetROVitesse().IsFasterThan(FILTER_FX::Vitesse::FAST_SPEED)))
  {
    return;
  }

  DoRegularlyLowerTheSpeed();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoRegularlyLowerTheSpeed() -> void
{
  LogChangeEvent(GO_SLOWER);
  m_filterSettingsService->GetRWVitesse().GoSlowerBy(1U);
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeTransformBufferLerpData() -> void
{
  if (not m_filterSettingsService->HasFilterModeChangedSinceLastUpdate())
  {
    return;
  }

  DoUpdateTransformBufferLerpData();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeRotation() -> void
{
  if (m_numUpdatesSinceLastFilterSettingsChange <= m_maxTimeBetweenFilterSettingsChange)
  {
    return;
  }

  DoChangeRotation();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoBigBreak() -> void
{
  LogChangeEvent(BIG_BREAK);

  static constexpr auto SLOWER_BY = 3U;
  m_filterSettingsService->GetRWVitesse().GoSlowerBy(SLOWER_BY);

  m_visualFx->ChangeAllFxColorMaps();
  m_visualFx->ChangeAllFxPixelBlenders();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeFilterMode() -> void
{
  LogChangeEvent(CHANGE_FILTER_MODE);

  m_filterSettingsService->SetNewRandomFilter();

  if (UpdateFilterSettingsNow())
  {
    DoUpdateFilterSettingsNow();
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::UpdateFilterSettingsNow()
    const noexcept -> bool
{
  return (not m_goomRand->ProbabilityOf(PROB_SLOW_FILTER_SETTINGS_UPDATE)) or
         (0 == m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom());
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoUpdateFilterSettingsNow() -> void
{
  LogChangeEvent(UPDATE_FILTER_SETTINGS_NOW);

  const auto& newFilterSettings = std::as_const(*m_filterSettingsService).GetFilterSettings();
  m_visualFx->SetZoomMidpoint(newFilterSettings.filterEffectsSettings.zoomMidpoint);
  m_filterSettingsService->NotifyUpdatedFilterEffectsSettings();
  m_numUpdatesSinceLastFilterSettingsChange = 0;
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::BigUpdate() -> void
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
  if (m_goomRand->ProbabilityOf(PROB_CHANGE_TO_MEGA_LENT_MODE))
  {
    DoMegaLentUpdate();
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoMegaLentUpdate() -> void
{
  LogChangeEvent(MEGA_LENT_UPDATE);

  m_lock.IncreaseLockTime(MEGA_LENT_LOCK_TIME_INCREASE);

  m_visualFx->ChangeAllFxColorMaps();
  m_filterSettingsService->GetRWVitesse().SetVitesse(FILTER_FX::Vitesse::SLOWEST_SPEED);

  DoSetTransformBufferLerpToEnd();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoBigNormalUpdate() -> void
{
  LogChangeEvent(BIG_NORMAL_UPDATE);

  m_lock.SetLockTime(NORMAL_UPDATE_LOCK_TIME);

  ChangeState();
  RestoreZoomIn();
  ChangeStopSpeeds();
  ChangeRotation();
  ChangeFilterExtraSettings();
  ChangeVitesse();
  ChangeTransformBufferLerpToEnd();
  m_visualFx->ChangeAllFxColorMaps();

  m_maxTimeBetweenFilterSettingsChange =
      m_goomRand->GetRandInRange(MAX_TIME_BETWEEN_FILTER_SETTINGS_CHANGE_RANGE);
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeState() -> void
{
  if (m_stateSelectionBlocker > 0)
  {
    --m_stateSelectionBlocker;
    return;
  }
  if (not m_goomRand->ProbabilityOf(PROB_CHANGE_STATE))
  {
    return;
  }

  DoChangeState();

  m_stateSelectionBlocker = MAX_NUM_STATE_SELECTIONS_BLOCKED;
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeState() -> void
{
  LogChangeEvent(CHANGE_STATE);

  m_visualFx->SetNextState();
  m_visualFx->ChangeAllFxColorMaps();

  m_timeInState = 0;
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::RestoreZoomIn() -> void
{
  // Retablir le zoom avant.
  // Restore zoom in.

  if (static constexpr auto REVERSE_VITESSE_CYCLES = 13U;
      (m_filterSettingsService->GetROVitesse().GetReverseVitesse()) and
      ((m_goomInfo->GetTime().GetCurrentTime() % REVERSE_VITESSE_CYCLES) != 0) and
      m_goomRand->ProbabilityOf(PROB_FILTER_REVERSE_OFF_AND_STOP_SPEED))
  {
    DoChangeSpeedSlowAndForward();
  }
  if (m_goomRand->ProbabilityOf(PROB_FILTER_REVERSE_ON))
  {
    DoChangeSpeedReverse();
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeSpeedSlowAndForward() -> void
{
  LogChangeEvent(SET_SLOWER_SPEED_AND_SPEED_FORWARD);

  m_filterSettingsService->GetRWVitesse().SetReverseVitesse(false);
  m_filterSettingsService->GetRWVitesse().SetVitesse(FILTER_FX::Vitesse::SLOW_SPEED);
  m_lock.SetLockTime(SLOWER_SPEED_AND_SPEED_FORWARD_LOCK_TIME);
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeSpeedReverse() -> void
{
  LogChangeEvent(SET_SPEED_REVERSE);

  m_filterSettingsService->GetRWVitesse().SetReverseVitesse(true);
  m_lock.SetLockTime(REVERSE_SPEED_LOCK_TIME);
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeStopSpeeds() -> void
{
  if (m_goomRand->ProbabilityOf(PROB_FILTER_VITESSE_STOP_SPEED_MINUS_1))
  {
    LogChangeEvent(SET_SLOW_SPEED);
    m_filterSettingsService->GetRWVitesse().SetVitesse(FILTER_FX::Vitesse::SLOWEST_SPEED);
  }
  else if (m_goomRand->ProbabilityOf(PROB_FILTER_VITESSE_STOP_SPEED))
  {
    LogChangeEvent(SET_STOP_SPEED);
    m_filterSettingsService->GetRWVitesse().SetVitesse(FILTER_FX::Vitesse::STOP_SPEED);
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeRotation() -> void
{
  LogChangeEvent(CHANGE_ROTATION);

  if (m_goomRand->ProbabilityOf(PROB_FILTER_STOP_ROTATION))
  {
    LogChangeEvent(TURN_OFF_ROTATION);
    m_filterSettingsService->TurnOffRotation();
  }
  else if (m_goomRand->ProbabilityOf(PROB_FILTER_DECREASE_ROTATION))
  {
    LogChangeEvent(SLOWER_ROTATION);
    static constexpr auto ROTATE_SLOWER_FACTOR = 0.9F;
    m_filterSettingsService->MultiplyRotation(ROTATE_SLOWER_FACTOR);
  }
  else if (m_goomRand->ProbabilityOf(PROB_FILTER_INCREASE_ROTATION))
  {
    LogChangeEvent(FASTER_ROTATION);
    static constexpr auto ROTATE_FASTER_FACTOR = 1.1F;
    m_filterSettingsService->MultiplyRotation(ROTATE_FASTER_FACTOR);
  }
  else if (m_goomRand->ProbabilityOf(PROB_FILTER_TOGGLE_ROTATION))
  {
    LogChangeEvent(TOGGLE_ROTATION);
    m_filterSettingsService->ToggleRotationDirection();
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeFilterExtraSettings() -> void
{
  DoChangeFilterExtraSettings();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeFilterExtraSettings() -> void
{
  LogChangeEvent(CHANGE_FILTER_EXTRA_SETTINGS);

  m_filterSettingsService->ChangeMilieu();
  m_filterSettingsService->ResetRandomAfterEffects();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeVitesse() -> void
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

  auto& filterVitesse = m_filterSettingsService->GetRWVitesse();

  const auto currentVitesse = filterVitesse.GetVitesse();
  const auto newVitesse     = Vitesse::GetFasterBy(Vitesse::STOP_SPEED, newSpeedVal);

  if (currentVitesse > newVitesse)
  {
    // Current speed is faster than new one. Nothing to do.
    return;
  }

  // on accelere
  if (static constexpr auto VITESSE_CYCLES = 3U;
      ((currentVitesse > Vitesse::FASTER_SPEED) and (newVitesse > Vitesse::EVEN_FASTER_SPEED) and
       (0 == (m_goomInfo->GetTime().GetCurrentTime() % VITESSE_CYCLES))) or
      m_goomRand->ProbabilityOf(PROB_FILTER_CHANGE_VITESSE_AND_TOGGLE_REVERSE))
  {
    LogChangeEvent(SET_SLOWER_SPEED_AND_TOGGLE_REVERSE);
    DoSetSlowerSpeedAndToggleReverse();
  }
  else
  {
    DoChangeSpeed(currentVitesse, newVitesse);
  }

  m_lock.IncreaseLockTime(CHANGE_VITESSE_LOCK_TIME_INCREASE);
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoSetSlowerSpeedAndToggleReverse()
    -> void
{
  LogChangeEvent(SET_SLOWER_SPEED_AND_TOGGLE_REVERSE);

  auto& filterVitesse = m_filterSettingsService->GetRWVitesse();
  filterVitesse.SetVitesse(Vitesse::SLOWEST_SPEED);
  filterVitesse.ToggleReverseVitesse();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeSpeed(
    const uint32_t currentVitesse, const uint32_t newVitesse) -> void
{
  LogChangeEvent(CHANGE_SPEED);

  auto& filterVitesse                        = m_filterSettingsService->GetRWVitesse();
  static constexpr auto OLD_TO_NEW_SPEED_MIX = 0.4F;
  filterVitesse.SetVitesse(static_cast<uint32_t>(std::lerp(
      static_cast<float>(currentVitesse), static_cast<float>(newVitesse), OLD_TO_NEW_SPEED_MIX)));
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeTransformBufferLerpToEnd()
    -> void
{
  if (m_lock.GetLockTime() < CHANGE_LERP_TO_END_LOCK_TIME)
  {
    return;
  }

  DoSetTransformBufferLerpToEnd();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoUpdateTransformBufferLerpData()
    -> void
{
  LogChangeEvent(UPDATE_TRANSFORM_BUFFER_LERP_DATA);

  static constexpr auto MIN_TIME_SINCE_LAST_GOOM            = 10U;
  static constexpr auto NUM_CYCLES_BEFORE_LERP_SPEED_CHANGE = 2U;
  if ((m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom() > MIN_TIME_SINCE_LAST_GOOM) or
      (m_goomInfo->GetSoundEvents().GetTotalGoomsInCurrentCycle() >=
       NUM_CYCLES_BEFORE_LERP_SPEED_CHANGE))
  {
    DoSetNewTransformBufferLerpDataBasedOnSpeed();
  }
  else
  {
    DoResetTransformBufferLerpData();

    DoChangeRotation();
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoSetTransformBufferLerpToEnd() -> void
{
  LogChangeEvent(SET_LERP_TO_END);

  m_filterSettingsService->SetDefaultTransformBufferLerpIncrement();
  m_filterSettingsService->SetTransformBufferLerpToEnd();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoResetTransformBufferLerpData()
    -> void
{
  LogChangeEvent(RESET_LERP_DATA);
  m_filterSettingsService->ResetTransformBufferLerpData();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::
    DoSetNewTransformBufferLerpDataBasedOnSpeed() -> void
{
  LogChangeEvent(SET_NEW_LERP_DATA_BASED_ON_SPEED);

  m_filterSettingsService->SetDefaultTransformBufferLerpIncrement();

  auto diff = static_cast<float>(m_filterSettingsService->GetROVitesse().GetVitesse()) -
              static_cast<float>(m_previousZoomSpeed);
  if (diff < 0.0F)
  {
    diff = -diff;
  }
  if (static constexpr auto DIFF_CUT = 2.0F; diff > DIFF_CUT)
  {
    m_filterSettingsService->MultiplyTransformBufferLerpIncrement((diff + DIFF_CUT) / DIFF_CUT);
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::GetNameValueParams() const
    -> NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "Music Settings";
  return {
      GetPair(PARAM_GROUP, "vitesse", m_filterSettingsService->GetROVitesse().GetVitesse()),
      GetPair(PARAM_GROUP, "previousZoomSpeed", m_previousZoomSpeed),
      GetPair(PARAM_GROUP, "reverse", m_filterSettingsService->GetROVitesse().GetReverseVitesse()),
      GetPair(PARAM_GROUP,
              "relative speed",
              m_filterSettingsService->GetROVitesse().GetRelativeSpeed()),
      GetPair(PARAM_GROUP, "updatesSinceLastChange", m_numUpdatesSinceLastFilterSettingsChange),
  };
}

} // namespace GOOM::CONTROL
