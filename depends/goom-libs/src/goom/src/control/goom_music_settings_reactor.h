#pragma once

#include "goom_all_visual_fx.h"
#include "goom_events.h"
#include "goom_lock.h"
#include "goom_music_lines_reactor.h"
#include "goom_plugin_info.h"
#include "utils/randutils.h"
#include "utils/name_value_pairs.h"
#include "utils/timer.h"
#include "visual_fx/filters/filter_settings_service.h"

#include <cstdint>

namespace GOOM
{
class PluginInfo;

namespace CONTROL
{
class GoomAllVisualFx;

class GoomMusicSettingsReactor
{
public:
  GoomMusicSettingsReactor(const PluginInfo& goomInfo,
                           GoomAllVisualFx& visualFx,
                           const GoomEvents& goomEvents,
                           FILTERS::FilterSettingsService& filterSettingsService) noexcept;

  void Start();
  void NewCycle();

  void ChangeZoomEffects();
  void ChangeFilterModeIfMusicChanges();
  [[nodiscard]] auto CanDisplayLines() const -> bool;
  void ChangeGoomLines();
  void UpdateLineModes();

  // gros frein si la musique est calme
  void BigBreakIfMusicIsCalm();

  // tout ceci ne sera fait qu'en cas de non-blocage
  void BigUpdateIfNotLocked();

  // baisser regulierement la vitesse
  void RegularlyLowerTheSpeed();

  [[nodiscard]] auto GetNameValueParams() const -> UTILS::NameValuePairs;

private:
  const PluginInfo& m_goomInfo;
  GoomAllVisualFx& m_visualFx;
  using GoomEvent = GoomEvents::GoomEvent;
  const GoomEvents& m_goomEvents;
  FILTERS::FilterSettingsService& m_filterSettingsService;
  GoomMusicLinesReactor m_musicLinesReactor;

  static constexpr uint32_t NORMAL_UPDATE_LOCK_TIME = 50;
  static constexpr uint32_t REVERSE_SPEED_AND_STOP_SPEED_LOCK_TIME = 75;
  static constexpr uint32_t REVERSE_SPEED_LOCK_TIME = 100;
  static constexpr uint32_t MEGA_LENT_LOCK_TIME_INCREASE = 50;
  static constexpr uint32_t CHANGE_VITESSE_LOCK_TIME_INCREASE = 50;
  static constexpr uint32_t CHANGE_SWITCH_VALUES_LOCK_TIME = 150;
  GoomLock m_lock{}; // pour empecher de nouveaux changements

  uint32_t m_updateNum = 0;

  static constexpr int32_t MAX_TIME_BETWEEN_ZOOM_EFFECTS_CHANGE = 200;
  int32_t m_updatesSinceLastZoomEffectsChange = 0; // nombre de Cycle Depuis Dernier Changement
  int32_t m_previousZoomSpeed = FILTERS::Vitesse::DEFAULT_VITESSE + 1;

  static constexpr uint32_t MAX_NUM_STATE_SELECTIONS_BLOCKED = 3;
  uint32_t m_stateSelectionBlocker = 0;
  uint32_t m_timeInState = 0;
  void ChangeState();
  void DoChangeState();

  // Changement d'effet de zoom !
  void BigNormalUpdate();
  void MegaLentUpdate();
  void BigUpdate();
  void BigBreak();
  void ChangeFilterMode();
  void ChangeMilieu();
  void ChangeRotation();
  void ChangeTranBufferSwitchValues();
  void ChangeSpeedReverse();
  void ChangeVitesse();
  void ChangeStopSpeeds();

  void UpdateTimers();

  static constexpr uint32_t NUM_ALLOW_OVEREXPOSED_ON_UPDATES = 500;
  static constexpr uint32_t NUM_ALLOW_OVEREXPOSED_OFF_UPDATES = 100;
  UTILS::Timer m_allowOverexposedTimer{NUM_ALLOW_OVEREXPOSED_ON_UPDATES};
  void ChangeAllowOverexposed();

  static constexpr uint32_t NUM_BLOCKY_WAVY_UPDATES = 100;
  UTILS::Timer m_blockyWavyTimer{NUM_BLOCKY_WAVY_UPDATES};
  void ChangeBlockyWavy();

  static constexpr uint32_t NUM_NOISE_UPDATES = 100;
  UTILS::Timer m_noiseTimer{NUM_NOISE_UPDATES};
  void ChangeNoise();
};

inline void GoomMusicSettingsReactor::Start()
{
  m_updateNum = 0;
  m_timeInState = 0;

  m_musicLinesReactor.Start();

  DoChangeState();
}

inline void GoomMusicSettingsReactor::NewCycle()
{
  ++m_updateNum;
  ++m_timeInState;
  m_lock.Update();
  UpdateTimers();

  m_musicLinesReactor.NewCycle();
}

inline void GoomMusicSettingsReactor::UpdateTimers()
{
  m_blockyWavyTimer.Increment();
  m_noiseTimer.Increment();
  m_allowOverexposedTimer.Increment();
}

inline auto GoomMusicSettingsReactor::CanDisplayLines() const -> bool
{
  return m_musicLinesReactor.CanDisplayLines();
}

inline void GoomMusicSettingsReactor::UpdateLineModes()
{
  m_musicLinesReactor.UpdateLineModes();
}

inline void GoomMusicSettingsReactor::BigBreakIfMusicIsCalm()
{
  constexpr float CALM_SPEED = 0.05F;
  constexpr uint32_t CALM_CYCLES = 16;
  constexpr int32_t CALM_VITESSE = FILTERS::Vitesse::STOP_SPEED - 4;

  if ((m_goomInfo.GetSoundInfo().GetSpeed() < CALM_SPEED) &&
      (m_filterSettingsService.GetROVitesse().GetVitesse() < CALM_VITESSE) &&
      (0 == (m_updateNum % CALM_CYCLES)))
  {
    BigBreak();
  }
}

inline void GoomMusicSettingsReactor::BigBreak()
{
  constexpr int32_t SLOWER_BY = 3;
  m_filterSettingsService.GetRWVitesse().GoSlowerBy(SLOWER_BY);

  m_visualFx.ChangeAllFxColorMaps();
}

inline void GoomMusicSettingsReactor::ChangeFilterModeIfMusicChanges()
{
  if (((0 == m_goomInfo.GetSoundInfo().GetTimeSinceLastGoom()) ||
       (m_updatesSinceLastZoomEffectsChange > MAX_TIME_BETWEEN_ZOOM_EFFECTS_CHANGE)) &&
      m_goomEvents.Happens(GoomEvent::CHANGE_FILTER_MODE))
  {
    ChangeFilterMode();
  }
}

inline void GoomMusicSettingsReactor::ChangeFilterMode()
{
  m_filterSettingsService.SetRandomFilterSettings();
  m_visualFx.RefreshAllFx();
}

inline void GoomMusicSettingsReactor::ChangeMilieu()
{
  m_filterSettingsService.ChangeMilieu();
}

inline void GoomMusicSettingsReactor::ChangeAllowOverexposed()
{
  if (!m_allowOverexposedTimer.Finished())
  {
    return;
  }

  const bool allowOverexposed =
      m_goomEvents.Happens(GoomEvent::CHANGE_ZOOM_FILTER_ALLOW_OVEREXPOSED_TO_ON);

  m_visualFx.SetZoomFilterAllowOverexposed(allowOverexposed);
  m_allowOverexposedTimer.SetTimeLimit(allowOverexposed ? NUM_ALLOW_OVEREXPOSED_ON_UPDATES
                                                        : NUM_ALLOW_OVEREXPOSED_OFF_UPDATES);
}

inline void GoomMusicSettingsReactor::ChangeBlockyWavy()
{
  if (!m_blockyWavyTimer.Finished())
  {
    return;
  }

  const bool blockyWavy = m_goomEvents.Happens(GoomEvent::CHANGE_BLOCKY_WAVY_TO_ON);

  m_filterSettingsService.SetBlockyWavy(blockyWavy);
  m_blockyWavyTimer.ResetToZero();
}

inline void GoomMusicSettingsReactor::ChangeNoise()
{
  if (!m_noiseTimer.Finished())
  {
    return;
  }

  const bool noiseOn = !m_goomEvents.Happens(GoomEvent::TURN_OFF_NOISE);

  m_filterSettingsService.SetNoise(noiseOn);
  m_noiseTimer.ResetToZero();
}

inline void GoomMusicSettingsReactor::ChangeRotation()
{
  if (m_goomEvents.Happens(GoomEvent::FILTER_STOP_ROTATION))
  {
    m_filterSettingsService.SetRotateToZero();
  }
  else if (m_goomEvents.Happens(GoomEvent::FILTER_DECREASE_ROTATION))
  {
    constexpr float ROTATE_SLOWER_FACTOR = 0.9F;
    m_filterSettingsService.MultiplyRotate(ROTATE_SLOWER_FACTOR);
  }
  else if (m_goomEvents.Happens(GoomEvent::FILTER_INCREASE_ROTATION))
  {
    constexpr float ROTATE_FASTER_FACTOR = 1.1F;
    m_filterSettingsService.MultiplyRotate(ROTATE_FASTER_FACTOR);
  }
  else if (m_goomEvents.Happens(GoomEvent::FILTER_TOGGLE_ROTATION))
  {
    m_filterSettingsService.ToggleRotate();
  }
}

inline void GoomMusicSettingsReactor::RegularlyLowerTheSpeed()
{
  constexpr uint32_t LOWER_SPEED_CYCLES = 73;
  constexpr int32_t FAST_SPEED = FILTERS::Vitesse::STOP_SPEED - 5;

  if ((0 == (m_updateNum % LOWER_SPEED_CYCLES)) &&
      (m_filterSettingsService.GetROVitesse().GetVitesse() < FAST_SPEED))
  {
    m_filterSettingsService.GetRWVitesse().GoSlowerBy(1);
  }
}

inline void GoomMusicSettingsReactor::BigUpdateIfNotLocked()
{
  if (m_lock.IsLocked())
  {
    return;
  }

  BigUpdate();
}

inline void GoomMusicSettingsReactor::BigUpdate()
{
  // Reperage de goom (acceleration forte de l'acceleration du volume).
  // Coup de boost de la vitesse si besoin.
  // Goom tracking (strong acceleration of volume acceleration).
  // Speed boost if needed.
  if (0 == m_goomInfo.GetSoundInfo().GetTimeSinceLastGoom())
  {
    BigNormalUpdate();
  }

  // mode mega-lent
  if (m_goomEvents.Happens(GoomEvent::CHANGE_TO_MEGA_LENT_MODE))
  {
    MegaLentUpdate();
  }
}

inline void GoomMusicSettingsReactor::BigNormalUpdate()
{
  m_lock.SetLockTime(NORMAL_UPDATE_LOCK_TIME);

  ChangeState();
  ChangeSpeedReverse();
  ChangeStopSpeeds();
  ChangeRotation();
  ChangeMilieu();
  ChangeNoise();
  ChangeBlockyWavy();
  ChangeAllowOverexposed();
  ChangeVitesse();
  ChangeTranBufferSwitchValues();

  constexpr float PROB_SINGLE_BUFFER_DOTS = 1.0F / 20.0F;
  m_visualFx.SetSingleBufferDots(UTILS::ProbabilityOf(PROB_SINGLE_BUFFER_DOTS));
}

inline void GoomMusicSettingsReactor::MegaLentUpdate()
{
  m_lock.IncreaseLockTime(MEGA_LENT_LOCK_TIME_INCREASE);

  m_filterSettingsService.GetRWVitesse().SetVitesse(FILTERS::Vitesse::STOP_SPEED - 1);
  m_filterSettingsService.SetDefaultTranLerpIncrement();
  m_filterSettingsService.SetTranLerpToMaxSwitchMult(1.0F);
}

inline void GoomMusicSettingsReactor::ChangeSpeedReverse()
{
  // Retablir le zoom avant.
  // Restore zoom in.

  constexpr uint32_t REVERSE_VITESSE_CYCLES = 13;
  constexpr int32_t SLOW_SPEED = FILTERS::Vitesse::STOP_SPEED - 2;

  if ((m_filterSettingsService.GetROVitesse().GetReverseVitesse()) &&
      ((m_updateNum % REVERSE_VITESSE_CYCLES) != 0) &&
      m_goomEvents.Happens(GoomEvent::FILTER_REVERSE_OFF_AND_STOP_SPEED))
  {
    m_filterSettingsService.GetRWVitesse().SetReverseVitesse(false);
    m_filterSettingsService.GetRWVitesse().SetVitesse(SLOW_SPEED);
    m_lock.SetLockTime(REVERSE_SPEED_AND_STOP_SPEED_LOCK_TIME);
  }
  if (m_goomEvents.Happens(GoomEvent::FILTER_REVERSE_ON))
  {
    m_filterSettingsService.GetRWVitesse().SetReverseVitesse(true);
    m_lock.SetLockTime(REVERSE_SPEED_LOCK_TIME);
  }
}

inline void GoomMusicSettingsReactor::ChangeStopSpeeds()
{
  if (m_goomEvents.Happens(GoomEvent::FILTER_VITESSE_STOP_SPEED_MINUS1))
  {
    constexpr int32_t SLOW_SPEED = FILTERS::Vitesse::STOP_SPEED - 1;
    m_filterSettingsService.GetRWVitesse().SetVitesse(SLOW_SPEED);
  }
  else if (m_goomEvents.Happens(GoomEvent::FILTER_VITESSE_STOP_SPEED))
  {
    m_filterSettingsService.GetRWVitesse().SetVitesse(FILTERS::Vitesse::STOP_SPEED);
  }
}

inline void GoomMusicSettingsReactor::ChangeTranBufferSwitchValues()
{
  if (m_lock.GetLockTime() > CHANGE_SWITCH_VALUES_LOCK_TIME)
  {
    m_filterSettingsService.SetDefaultTranLerpIncrement();
    m_filterSettingsService.SetTranLerpToMaxSwitchMult(1.0F);
  }
}

inline void GoomMusicSettingsReactor::ChangeState()
{
  if (m_stateSelectionBlocker > 0)
  {
    --m_stateSelectionBlocker;
    return;
  }
  if (!m_goomEvents.Happens(GoomEvent::CHANGE_STATE))
  {
    return;
  }

  DoChangeState();

  m_stateSelectionBlocker = MAX_NUM_STATE_SELECTIONS_BLOCKED;
}

inline void GoomMusicSettingsReactor::DoChangeState()
{
  m_visualFx.SetNextState();

  m_timeInState = 0;

  m_musicLinesReactor.ResetLineModes();
}

inline void GoomMusicSettingsReactor::ChangeGoomLines()
{
  m_musicLinesReactor.ChangeGoomLines();
}

} // namespace CONTROL
} // namespace GOOM

