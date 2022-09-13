#include "goom_music_settings_reactor.h"

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
    const GoomEvents& goomEvents,
    FilterSettingsService& filterSettingsService) noexcept
  : m_goomInfo{goomInfo},
    m_goomRand{goomRand},
    m_visualFx{visualFx},
    m_goomEvents{goomEvents},
    m_filterSettingsService{filterSettingsService}
{
}

auto GoomMusicSettingsReactor::ChangeZoomEffects() -> void
{
  if (!m_filterSettingsService.HasFilterModeChangedSinceLastUpdate())
  {
    if (m_updatesSinceLastZoomEffectsChange > MAX_TIME_BETWEEN_ZOOM_EFFECTS_CHANGE)
    {
      m_updatesSinceLastZoomEffectsChange = 0;

      ChangeRotation();
      m_visualFx.RefreshAllFx();
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

    auto diff = static_cast<int32_t>(m_filterSettingsService.GetROVitesse().GetVitesse()) -
                static_cast<int32_t>(m_previousZoomSpeed);
    if (diff < 0)
    {
      diff = -diff;
    }

    if (static constexpr auto DIFF_CUT = 2; diff > DIFF_CUT)
    {
      m_filterSettingsService.MultiplyTranLerpIncrement((diff + DIFF_CUT) / DIFF_CUT);
    }
    m_previousZoomSpeed = m_filterSettingsService.GetROVitesse().GetVitesse();
    m_filterSettingsService.SetTranLerpToMaxSwitchMult(1.0F);

    static constexpr auto NUM_CYCLES_BEFORE_LERP_CHANGE = 2U;
    if ((0 == m_goomInfo.GetSoundEvents().GetTimeSinceLastGoom()) &&
        (m_goomInfo.GetSoundEvents().GetTotalGoomsInCurrentCycle() < NUM_CYCLES_BEFORE_LERP_CHANGE))
    {
      m_filterSettingsService.SetTranLerpIncrement(0);
      m_filterSettingsService.SetTranLerpToMaxDefaultSwitchMult();

      ChangeRotation();
      m_visualFx.RefreshAllFx();
    }
  }
}

auto GoomMusicSettingsReactor::ChangeVitesse() -> void
{
  // SPEED_FACTOR is delicate. Too small and zooms don't happen often enough.
  static constexpr auto SPEED_FACTOR = 500.0F;
  const auto goFasterVal             = static_cast<uint32_t>(std::lround(
      3.5F *
      std::log10(1.0F + (SPEED_FACTOR * m_goomInfo.GetSoundEvents().GetSoundInfo().GetSpeed()))));

  auto& filterVitesse   = m_filterSettingsService.GetRWVitesse();
  const auto newVitesse = Vitesse::GetFasterBy(Vitesse::STOP_SPEED, goFasterVal);
  if (filterVitesse.IsFasterThan(newVitesse))
  {
    // Current speed is faster than new one. Nothing to do.
    return;
  }

  const auto oldVitesse = filterVitesse.GetVitesse();

  // on accelere
  if (static constexpr auto VITESSE_CYCLES = 3U;
      (Vitesse::IsFasterThan(oldVitesse, Vitesse::FASTER_SPEED) and
       Vitesse::IsFasterThan(newVitesse, Vitesse::EVEN_FASTER_SPEED) and
       (0 == (m_updateNum % VITESSE_CYCLES))) or
      m_goomEvents.Happens(GoomEvent::FILTER_CHANGE_VITESSE_AND_TOGGLE_REVERSE))
  {
    filterVitesse.SetVitesse(Vitesse::SLOWEST_SPEED);
    filterVitesse.ToggleReverseVitesse();
  }
  else
  {
    static constexpr auto OLD_TO_NEW_SPEED_MIX = 0.4F;
    filterVitesse.SetVitesse(STD20::lerp(oldVitesse, newVitesse, OLD_TO_NEW_SPEED_MIX));
  }

  m_lock.IncreaseLockTime(CHANGE_VITESSE_LOCK_TIME_INCREASE);
}

auto GoomMusicSettingsReactor::GetNameValueParams() const -> NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "Music Settings";
  return {
      GetPair(PARAM_GROUP, "vitesse", m_filterSettingsService.GetROVitesse().GetVitesse()),
      GetPair(PARAM_GROUP, "previousZoomSpeed", m_previousZoomSpeed),
      GetPair(PARAM_GROUP, "reverse", m_filterSettingsService.GetROVitesse().GetReverseVitesse()),
      GetPair(
          PARAM_GROUP, "relative speed", m_filterSettingsService.GetROVitesse().GetRelativeSpeed()),
      GetPair(PARAM_GROUP, "updatesSinceLastChange", m_updatesSinceLastZoomEffectsChange),
  };
}

} // namespace GOOM::CONTROL
