#include "goom_music_settings_reactor.h"

#include "goomutils/goomrand.h"
#include "goomutils/name_value_pairs.h"

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
using UTILS::ProbabilityOfMInN;

GoomMusicSettingsReactor::GoomMusicSettingsReactor(const PluginInfo& goomInfo,
                                                   GoomAllVisualFx& visualFx,
                                                   FilterSettingsService& filterSettingsService)
  : m_goomInfo{goomInfo}, m_visualFx{visualFx}, m_filterSettingsService{filterSettingsService}
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
      m_visualFx.DoIfsRenew();
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
      m_visualFx.DoIfsRenew();
    }
  }
}

void GoomMusicSettingsReactor::ChangeVitesse()
{
  const auto goFasterVal = static_cast<int32_t>(
      std::lround(3.5F * std::log10(1.0F + (500.0F * m_goomInfo.GetSoundInfo().GetSpeed()))));
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
      m_goomEvent.Happens(GoomEvent::FILTER_CHANGE_VITESSE_AND_TOGGLE_REVERSE))
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

auto GoomMusicSettingsReactor::GetGoomLineResetSettings(const int farVal) const -> GoomLineSettings
{
  GoomLineSettings lineSettings;

  lineSettings.amplitude = 1.0F;
  lineSettings.mode = m_goomEvent.GetRandomLineTypeEvent();

  switch (lineSettings.mode)
  {
    case LinesFx::LineType::CIRCLE:
      if (farVal)
      {
        lineSettings.param1 = 0.47F;
        lineSettings.param2 = lineSettings.param1;
        lineSettings.amplitude = 0.8F;
        break;
      }
      if (m_goomEvent.Happens(GoomEvent::CHANGE_LINE_CIRCLE_AMPLITUDE))
      {
        lineSettings.param1 = 0.0F;
        lineSettings.param2 = 0.0F;
        lineSettings.amplitude = 3.0F;
      }
      else if (m_goomEvent.Happens(GoomEvent::CHANGE_LINE_CIRCLE_PARAMS))
      {
        lineSettings.param1 = 0.40F * static_cast<float>(m_goomInfo.GetScreenInfo().height);
        lineSettings.param2 = 0.22F * static_cast<float>(m_goomInfo.GetScreenInfo().height);
      }
      else
      {
        lineSettings.param1 = static_cast<float>(m_goomInfo.GetScreenInfo().height) * 0.35F;
        lineSettings.param2 = lineSettings.param1;
      }
      break;
    case LinesFx::LineType::H_LINE:
      if (m_goomEvent.Happens(GoomEvent::CHANGE_H_LINE_PARAMS) || (farVal != 0))
      {
        lineSettings.param1 = static_cast<float>(m_goomInfo.GetScreenInfo().height) / 7.0F;
        lineSettings.param2 = (6.0F * static_cast<float>(m_goomInfo.GetScreenInfo().height)) / 7.0F;
      }
      else
      {
        lineSettings.param1 = static_cast<float>(m_goomInfo.GetScreenInfo().height) / 2.0F;
        lineSettings.param2 = lineSettings.param1;
        lineSettings.amplitude = 2.0F;
      }
      break;
    case LinesFx::LineType::V_LINE:
      if (m_goomEvent.Happens(GoomEvent::CHANGE_V_LINE_PARAMS) || (farVal != 0))
      {
        lineSettings.param1 = static_cast<float>(m_goomInfo.GetScreenInfo().width) / 7.0F;
        lineSettings.param2 = (6.0F * static_cast<float>(m_goomInfo.GetScreenInfo().width)) / 7.0F;
      }
      else
      {
        lineSettings.param1 = static_cast<float>(m_goomInfo.GetScreenInfo().width) / 2.0F;
        lineSettings.param2 = lineSettings.param1;
        lineSettings.amplitude = 1.5F;
      }
      break;
    default:
      throw std::logic_error("Unknown LineTypes enum.");
  }

  if ((farVal != 0) && m_goomEvent.Happens(GoomEvent::CHANGE_LINE_TO_BLACK))
  {
    lineSettings.line1Color = GetBlackLineColor();
    lineSettings.line2Color = lineSettings.line1Color;
  }
  else
  {
    lineSettings.line1Color = m_visualFx.GetGoomLine1RandomColor();
    lineSettings.line2Color = m_visualFx.GetGoomLine2RandomColor();
  }

  return lineSettings;
}

/* arret aleatore.. changement de mode de ligne..
  */
void GoomMusicSettingsReactor::StopRandomLineChangeMode()
{
  constexpr uint32_t DEC_LINE_MODE_CYCLES = 80;
  constexpr uint32_t UPDATE_LINE_MODE_CYCLES = 120;

  if (m_lineMode != m_drawLinesDuration)
  {
    --m_lineMode;
    if (-1 == m_lineMode)
    {
      m_lineMode = 0;
    }
  }
  else if ((0 == (m_updateNum % DEC_LINE_MODE_CYCLES)) &&
           m_goomEvent.Happens(GoomEvent::REDUCE_LINE_MODE) && (m_lineMode != 0))
  {
    --m_lineMode;
  }

  if ((0 == (m_updateNum % UPDATE_LINE_MODE_CYCLES)) &&
      m_goomEvent.Happens(GoomEvent::UPDATE_LINE_MODE) &&
      m_visualFx.IsCurrentlyDrawable(GoomDrawable::SCOPE))
  {
    if (0 == m_lineMode)
    {
      m_lineMode = m_drawLinesDuration;
    }
    else if ((m_lineMode == m_drawLinesDuration) && m_visualFx.CanResetDestGoomLines())
    {
      --m_lineMode;

      ResetGoomLines();
    }
  }
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
