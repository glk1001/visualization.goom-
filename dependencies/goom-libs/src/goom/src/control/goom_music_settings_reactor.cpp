#include "goom_music_settings_reactor.h"

#include "utils/name_value_pairs.h"

#if __cplusplus <= 201402L
namespace GOOM
{
namespace CONTROL
{
#else
namespace GOOM::CONTROL
{
#endif

using FILTERS::FilterSettingsService;
using FILTERS::Vitesse;
using UTILS::GetPair;
using UTILS::NameValuePairs;

GoomMusicSettingsReactor::GoomMusicSettingsReactor(
    const PluginInfo& goomInfo,
    GoomAllVisualFx& visualFx,
    const GoomEvents& goomEvents,
    FilterSettingsService& filterSettingsService) noexcept
  : m_goomInfo{goomInfo},
    m_visualFx{visualFx},
    m_goomEvents{goomEvents},
    m_filterSettingsService{filterSettingsService},
    m_musicLinesReactor{m_goomInfo, m_visualFx, m_goomEvents}
{
}

void GoomMusicSettingsReactor::ChangeZoomEffects()
{
  ChangeBlockyWavy();
  ChangeAllowOverexposed();

  if (!m_filterSettingsService.HasFilterModeChangedSinceLastUpdate())
  {
    if (m_updatesSinceLastZoomEffectsChange > MAX_TIME_BETWEEN_ZOOM_EFFECTS_CHANGE)
    {
      m_updatesSinceLastZoomEffectsChange = 0;

      ChangeRotation();
      m_visualFx.RefreshAll();
    }
    else
    {
      ++m_updatesSinceLastZoomEffectsChange;
    }
  }
  else
  {
    m_updatesSinceLastZoomEffectsChange = 0;
    m_filterSettingsService.SetDefaultTranLerpIncrement();

    int32_t diff = m_filterSettingsService.GetROVitesse().GetVitesse() - m_previousZoomSpeed;
    if (diff < 0)
    {
      diff = -diff;
    }

    constexpr int32_t DIFF_CUT = 2;
    if (diff > DIFF_CUT)
    {
      m_filterSettingsService.MultiplyTranLerpIncrement((diff + DIFF_CUT) / DIFF_CUT);
    }
    m_previousZoomSpeed = m_filterSettingsService.GetROVitesse().GetVitesse();
    m_filterSettingsService.SetTranLerpToMaxSwitchMult(1.0F);

    constexpr uint32_t NUM_CYCLES_BEFORE_LERP_CHANGE = 2;
    if ((0 == m_goomInfo.GetSoundInfo().GetTimeSinceLastGoom()) &&
        (m_goomInfo.GetSoundInfo().GetTotalGoomsInCurrentCycle() < NUM_CYCLES_BEFORE_LERP_CHANGE))
    {
      m_filterSettingsService.SetTranLerpIncrement(0);
      m_filterSettingsService.SetTranLerpToMaxDefaultSwitchMult();

      ChangeRotation();
      m_visualFx.RefreshAll();
    }
  }
}

void GoomMusicSettingsReactor::ChangeVitesse()
{
  const auto goFasterVal = static_cast<int32_t>(
      std::lround(3.5F * std::log10(1.0F + (100.0F * m_goomInfo.GetSoundInfo().GetSpeed()))));
  const int32_t newVitesse = Vitesse::STOP_SPEED - goFasterVal;
  const int32_t oldVitesse = m_filterSettingsService.GetROVitesse().GetVitesse();

  if (newVitesse >= oldVitesse)
  {
    return;
  }

  constexpr uint32_t VITESSE_CYCLES = 3;
  constexpr int32_t FAST_SPEED = Vitesse::STOP_SPEED - 6;
  constexpr int32_t FASTER_SPEED = Vitesse::STOP_SPEED - 7;
  constexpr int32_t SLOW_SPEED = Vitesse::STOP_SPEED - 1;
  constexpr float OLD_TO_NEW_MIX = 0.4F;

  // on accelere
  if (((newVitesse < FASTER_SPEED) && (oldVitesse < FAST_SPEED) &&
       (0 == (m_updateNum % VITESSE_CYCLES))) ||
      m_goomEvents.Happens(GoomEvent::FILTER_CHANGE_VITESSE_AND_TOGGLE_REVERSE))
  {
    m_filterSettingsService.GetRWVitesse().SetVitesse(SLOW_SPEED);
    m_filterSettingsService.GetRWVitesse().ToggleReverseVitesse();
  }
  else
  {
    m_filterSettingsService.GetRWVitesse().SetVitesse(static_cast<int32_t>(std::lround(stdnew::lerp(
        static_cast<float>(oldVitesse), static_cast<float>(newVitesse), OLD_TO_NEW_MIX))));
  }

  m_lock.IncreaseLockTime(CHANGE_VITESSE_LOCK_TIME_INCREASE);
}

auto GoomMusicSettingsReactor::GetNameValueParams() const -> NameValuePairs
{
  constexpr const char* PARAM_GROUP = "Music Settings";
  return {
      GetPair(PARAM_GROUP, "vitesse", m_filterSettingsService.GetROVitesse().GetVitesse()),
      GetPair(PARAM_GROUP, "previousZoomSpeed", m_previousZoomSpeed),
      GetPair(PARAM_GROUP, "reverse", m_filterSettingsService.GetROVitesse().GetReverseVitesse()),
      GetPair(PARAM_GROUP, "relative speed",
              m_filterSettingsService.GetROVitesse().GetRelativeSpeed()),
      GetPair(PARAM_GROUP, "updatesSinceLastChange", m_updatesSinceLastZoomEffectsChange),
  };
}

#if __cplusplus <= 201402L
} // namespace CONTROL
} // namespace GOOM
#else
} // namespace GOOM::CONTROL
#endif
