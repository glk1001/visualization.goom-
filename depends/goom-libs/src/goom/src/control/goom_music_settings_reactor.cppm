module;

#include <cstdint>

export module Goom.Control.GoomMusicSettingsReactor;

import Goom.Control.GoomAllVisualFx;
import Goom.FilterFx.FilterSettingsService;
import Goom.FilterFx.FilterSpeed;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRandBase;
import Goom.PluginInfo;
import :GoomLock;

export namespace GOOM::CONTROL
{

class GoomMusicSettingsReactor
{
public:
  GoomMusicSettingsReactor(const PluginInfo& goomInfo,
                           const UTILS::MATH::IGoomRand& goomRand,
                           GoomAllVisualFx& visualFx,
                           FILTER_FX::FilterSettingsService& filterSettingsService) noexcept;

  auto Start() -> void;
  auto NewCycle() -> void;

  auto UpdateSettings() -> void;

  [[nodiscard]] auto GetNameValueParams() const -> UTILS::NameValuePairs;

private:
  const PluginInfo* m_goomInfo;
  const UTILS::MATH::IGoomRand* m_goomRand;
  GoomAllVisualFx* m_visualFx;
  FILTER_FX::FilterSettingsService* m_filterSettingsService;

  static constexpr auto NORMAL_UPDATE_LOCK_TIME                  = 50U;
  static constexpr auto SLOWER_SPEED_AND_SPEED_FORWARD_LOCK_TIME = 75U;
  static constexpr auto REVERSE_SPEED_LOCK_TIME                  = 100U;
  static constexpr auto MEGA_LENT_LOCK_TIME_INCREASE             = 50U;
  static constexpr auto CHANGE_VITESSE_LOCK_TIME_INCREASE        = 50U;
  static constexpr auto CHANGE_LERP_TO_END_LOCK_TIME             = 150U;
  GoomLock m_lock; // pour empecher de nouveaux changements

  static constexpr auto MIN_MAX_TIME_BETWEEN_ZOOM_EFFECTS_CHANGE = 300;
  static constexpr auto MAX_MAX_TIME_BETWEEN_ZOOM_EFFECTS_CHANGE = 500;
  int32_t m_maxTimeBetweenFilterSettingsChange      = MIN_MAX_TIME_BETWEEN_ZOOM_EFFECTS_CHANGE;
  int32_t m_numUpdatesSinceLastFilterSettingsChange = 0;
  uint32_t m_previousZoomSpeed                      = FILTER_FX::Vitesse::STOP_SPEED;

  static constexpr auto MAX_NUM_STATE_SELECTIONS_BLOCKED = 3U;
  uint32_t m_stateSelectionBlocker                       = MAX_NUM_STATE_SELECTIONS_BLOCKED;
  uint32_t m_timeInState                                 = 0U;
  auto ChangeState() -> void;
  auto DoChangeState() -> void;

  auto ChangeTransformBufferLerpData() -> void;
  auto ChangeRotation() -> void;
  auto ChangeFilterModeIfMusicChanges() -> void;

  // gros frein si la musique est calme
  // big break if the music is quiet
  auto BigBreakIfMusicIsCalm() -> void;

  // tout ceci ne sera fait qu'en cas de non-blocage
  // all this will only be done in case of non-blocking
  auto BigUpdateIfNotLocked() -> void;

  // baisser regulierement la vitesse
  // steadily lower the speed
  auto RegularlyLowerTheSpeed() -> void;

  // Changement d'effet de zoom !
  auto DoBigNormalUpdate() -> void;
  auto DoMegaLentUpdate() -> void;
  auto DoBigUpdate() -> void;
  auto DoBigBreak() -> void;
  auto DoChangeFilterMode() -> void;
  [[nodiscard]] auto UpdateFilterSettingsNow() const noexcept -> bool;
  auto DoUpdateFilterSettingsNow() -> void;
  auto ChangeFilterExtraSettings() -> void;
  auto DoChangeFilterExtraSettings() -> void;
  auto DoUpdateTransformBufferLerpData() -> void;
  auto DoSetNewTransformBufferLerpDataBasedOnSpeed() -> void;
  auto ChangeTransformBufferLerpToEnd() -> void;
  auto DoSetTransformBufferLerpToEnd() -> void;
  auto DoChangeRotation() -> void;
  auto ChangeSpeedReverse() -> void;
  auto DoChangeSpeedSlowAndForward() -> void;
  auto DoChangeSpeedReverse() -> void;
  auto ChangeVitesse() -> void;
  auto DoSetSlowerSpeedAndToggleReverse() -> void;
  auto DoChangeSpeed(uint32_t currentVitesse, uint32_t newVitesse) -> void;
  auto ChangeStopSpeeds() -> void;

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

} // namespace GOOM::CONTROL
