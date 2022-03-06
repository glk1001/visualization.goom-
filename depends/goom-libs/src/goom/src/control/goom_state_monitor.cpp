#include "goom_state_monitor.h"

#include "goom_all_visual_fx.h"
#include "goom_graphic.h"
#include "goom_music_settings_reactor.h"
#include "utils/name_value_pairs.h"
#include "visual_fx/filters/filter_settings_service.h"

#include <format>

namespace GOOM::CONTROL
{

using UTILS::GetNameValuesString;
using UTILS::GetPair;
using VISUAL_FX::FILTERS::FilterSettingsService;
using VISUAL_FX::FILTERS::ZoomFilterBufferSettings;
using VISUAL_FX::FILTERS::ZoomFilterEffectsSettings;

GoomStateMonitor::GoomStateMonitor(
    const GoomAllVisualFx& visualFx,
    const GoomMusicSettingsReactor& musicSettingsReactor,
    const VISUAL_FX::FILTERS::FilterSettingsService& filterSettingsService) noexcept
  : m_visualFx{visualFx},
    m_musicSettingsReactor{musicSettingsReactor},
    m_filterSettingsService{filterSettingsService}
{
}

auto GoomStateMonitor::GetCurrentState() const -> std::string
{
  std::string message;

  message += GetNameValuesString(GetStateAndFilterModeNameValueParams()) + "\n";
  message += GetNameValuesString(GetShaderEffectsNameValueParams()) + "\n";
  message += GetNameValuesString(GetFilterBufferValueParams()) + "\n";
  message += GetNameValuesString(GetFilterEffectsNameValueParams()) + "\n";
  message += GetNameValuesString(GetMusicSettingsNameValueParams()) + "\n";
  message += GetNameValuesString(GetZoomFilterFxNameValueParams());

  return message;
}

inline auto GoomStateMonitor::GetStateAndFilterModeNameValueParams() const -> UTILS::NameValuePairs
{
  static constexpr const char* PARAM_GROUP = "";
  return {
      GetPair(PARAM_GROUP, "State", m_visualFx.GetCurrentStateName()),
      GetPair(PARAM_GROUP, "Filter Mode", m_filterSettingsService.GetCurrentFilterMode()),
      GetPair(PARAM_GROUP, "Previous Filter Mode", m_filterSettingsService.GetPreviousFilterMode()),
  };
}

inline auto GoomStateMonitor::GetShaderEffectsNameValueParams() const -> UTILS::NameValuePairs
{
  static constexpr const char* PARAM_GROUP = "Shader";
  const GoomShaderEffects& lastShaderEffects = m_visualFx.GetLastShaderEffects();
  return {
      GetPair(PARAM_GROUP, "Exposure", m_visualFx.GetCurrentExposure()),
      GetPair(PARAM_GROUP, "Contrast", lastShaderEffects.contrast),
      GetPair(PARAM_GROUP, "MinChan", lastShaderEffects.contrastMinChannelValue),
      GetPair(PARAM_GROUP, "Brightness", lastShaderEffects.brightness),
  };
}

inline auto GoomStateMonitor::GetFilterBufferValueParams() const -> UTILS::NameValuePairs
{
  static constexpr const char* PARAM_GROUP = "Filter Buffer";
  const ZoomFilterBufferSettings& filterBufferSettings =
      m_filterSettingsService.GetFilterSettings().filterBufferSettings;
  return {
      GetPair(PARAM_GROUP, "TranLerpIncrement", filterBufferSettings.tranLerpIncrement),
      GetPair(PARAM_GROUP, "TranLerpToMaxSwitchMult", filterBufferSettings.tranLerpToMaxSwitchMult),
  };
}

inline auto GoomStateMonitor::GetFilterEffectsNameValueParams() const -> UTILS::NameValuePairs
{
  static constexpr const char* PARAM_GROUP = "Filter Settings";
  const ZoomFilterEffectsSettings& filterEffectsSettings =
      m_filterSettingsService.GetFilterSettings().filterEffectsSettings;
  return {
      GetPair(
          PARAM_GROUP, "Middle",
          Point2dInt{filterEffectsSettings.zoomMidpoint.x, filterEffectsSettings.zoomMidpoint.y}),
  };
}

inline auto GoomStateMonitor::GetMusicSettingsNameValueParams() const -> UTILS::NameValuePairs
{
  return m_musicSettingsReactor.GetNameValueParams();
}

inline auto GoomStateMonitor::GetZoomFilterFxNameValueParams() const -> UTILS::NameValuePairs
{
  return m_visualFx.GetZoomFilterFxNameValueParams();
}

} // namespace GOOM::CONTROL
