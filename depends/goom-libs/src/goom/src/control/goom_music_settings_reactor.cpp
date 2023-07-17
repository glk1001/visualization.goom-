//#undef NO_LOGGING

#include "goom_music_settings_reactor.h"

//#include "utils/debugging_logger.h"
#include "utils/name_value_pairs.h"

namespace GOOM::CONTROL
{

using FILTER_FX::FilterSettingsService;
using FILTER_FX::Vitesse;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;

GoomMusicSettingsReactor::GoomMusicSettingsReactor(
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

auto GoomMusicSettingsReactor::ChangeFilterSettings() -> void
{
  //  LogInfo(UTILS::GetGoomLogger(),
  //          "ChangeFilterSettings: "
  //          "m_numUpdatesSinceLastFilterSettingsChange = {}, "
  //          "m_maxTimeBetweenFilterSettingsChange= {}, "
  //          "m_filterSettingsService->HasFilterModeChangedSinceLastUpdate = {}",
  //          m_numUpdatesSinceLastFilterSettingsChange,
  //          m_maxTimeBetweenFilterSettingsChange,
  //          m_filterSettingsService->HasFilterModeChangedSinceLastUpdate());

  if ((m_numUpdatesSinceLastFilterSettingsChange <= m_maxTimeBetweenFilterSettingsChange) and
      (not m_filterSettingsService->HasFilterModeChangedSinceLastUpdate()))
  {
    ++m_numUpdatesSinceLastFilterSettingsChange;
    return;
  }

  //  LogInfo(UTILS::GetGoomLogger(), "UpdateFilterSettings.");
  m_numUpdatesSinceLastFilterSettingsChange = 0;
  UpdateFilterSettings();
  m_previousZoomSpeed = m_filterSettingsService->GetROVitesse().GetVitesse();
}

inline auto GoomMusicSettingsReactor::UpdateFilterSettings() -> void
{
  if (m_filterSettingsService->HasFilterModeChangedSinceLastUpdate())
  {
    //    LogInfo(UTILS::GetGoomLogger(), "UpdateTransformBufferLerpData.");
    UpdateTransformBufferLerpData();
  }
  else
  {
    //    LogInfo(UTILS::GetGoomLogger(), "ChangeRotation.");
    ChangeRotation();
  }

  m_visualFx->RefreshAllFx();
}

inline auto GoomMusicSettingsReactor::UpdateTransformBufferLerpData() -> void
{
  if (static constexpr auto NUM_CYCLES_BEFORE_LERP_SPEED_CHANGE = 2U;
      (m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom() > 0U) or
      (m_goomInfo->GetSoundEvents().GetTotalGoomsInCurrentCycle() >=
       NUM_CYCLES_BEFORE_LERP_SPEED_CHANGE))
  {
    //    LogInfo(UTILS::GetGoomLogger(), "Set speed based lerp.");
    SetNewTransformBufferLerpDataBasedOnSpeed();
  }
  else
  {
    //    LogInfo(UTILS::GetGoomLogger(), "Resetting lerp.");
    ResetTransformBufferLerpData();
    ChangeRotation();
  }
}

auto GoomMusicSettingsReactor::ResetTransformBufferLerpData() -> void
{
  m_filterSettingsService->SetTransformBufferLerpIncrement(0.0F);
  m_filterSettingsService->SetDefaultTransformBufferLerpToMaxLerp();
}

inline auto GoomMusicSettingsReactor::SetNewTransformBufferLerpDataBasedOnSpeed() -> void
{
  m_filterSettingsService->SetDefaultTransformBufferLerpIncrement();
  //  LogInfo(UTILS::GetGoomLogger(),
  //          "Lerp Inc = {}.",
  //          m_filterSettingsService->GetFilterSettings()
  //              .filterTransformBufferSettings.lerpData.lerpIncrement);

  auto diff = static_cast<float>(m_filterSettingsService->GetROVitesse().GetVitesse()) -
              static_cast<float>(m_previousZoomSpeed);
  if (diff < 0.0F)
  {
    diff = -diff;
  }
  //  LogInfo(UTILS::GetGoomLogger(), "diff = {}.", diff);
  if (static constexpr auto DIFF_CUT = 2.0F; diff > DIFF_CUT)
  {
    m_filterSettingsService->MultiplyTransformBufferLerpIncrement((diff + DIFF_CUT) / DIFF_CUT);
  }
  //  LogInfo(UTILS::GetGoomLogger(),
  //          "Final Lerp Inc = {}.",
  //          m_filterSettingsService->GetFilterSettings()
  //              .filterTransformBufferSettings.lerpData.lerpIncrement);

  m_filterSettingsService->SetTransformBufferLerpToMaxLerp(0.0F);
}

auto GoomMusicSettingsReactor::ChangeVitesse() -> void
{
  const auto soundSpeed = m_goomInfo->GetSoundEvents().GetSoundInfo().GetSpeed();

  static constexpr auto MIN_USABLE_SOUND_SPEED = SoundInfo::SPEED_MIDPOINT - 0.1F;
  static constexpr auto MAX_USABLE_SOUND_SPEED = SoundInfo::SPEED_MIDPOINT + 0.1F;
  const auto usableRelativeSoundSpeed =
      (std::clamp(soundSpeed, MIN_USABLE_SOUND_SPEED, MAX_USABLE_SOUND_SPEED) -
       MIN_USABLE_SOUND_SPEED) /
      (MAX_USABLE_SOUND_SPEED - MIN_USABLE_SOUND_SPEED);

  static constexpr auto MAX_SPEED_CHANGE = 10U;
  const auto newSpeedVal = STD20::lerp(0U, MAX_SPEED_CHANGE, usableRelativeSoundSpeed);

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
       (0 == (m_updateNum % VITESSE_CYCLES))) or
      m_goomRand->ProbabilityOf(PROB_FILTER_CHANGE_VITESSE_AND_TOGGLE_REVERSE))
  {
    filterVitesse.SetVitesse(Vitesse::SLOWEST_SPEED);
    filterVitesse.ToggleReverseVitesse();
  }
  else
  {
    static constexpr auto OLD_TO_NEW_SPEED_MIX = 0.4F;
    filterVitesse.SetVitesse(STD20::lerp(currentVitesse, newVitesse, OLD_TO_NEW_SPEED_MIX));
  }

  m_lock.IncreaseLockTime(CHANGE_VITESSE_LOCK_TIME_INCREASE);
}

auto GoomMusicSettingsReactor::GetNameValueParams() const -> NameValuePairs
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
