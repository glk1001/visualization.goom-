#include "goom_state_monitor.h"

#include "filter_fx/filter_settings_service.h"
#include "goom_all_visual_fx.h"
#include "goom_graphic.h"
#include "goom_music_settings_reactor.h"
#include "utils/name_value_pairs.h"

namespace GOOM::CONTROL
{

using FILTER_FX::FilterBuffersService;
using FILTER_FX::FilterSettingsService;
using UTILS::GetNameValuesString;
using UTILS::GetPair;

GoomStateMonitor::GoomStateMonitor(const GoomAllVisualFx& visualFx,
                                   const GoomMusicSettingsReactor& musicSettingsReactor,
                                   const FilterSettingsService& filterSettingsService,
                                   const FilterBuffersService& filterBuffersService) noexcept
  : m_visualFx{&visualFx},
    m_musicSettingsReactor{&musicSettingsReactor},
    m_filterSettingsService{&filterSettingsService},
    m_filterBuffersService{&filterBuffersService}
{
}

auto GoomStateMonitor::GetCurrentState() const -> std::string
{
  auto message = std::string{};

  message += GetNameValuesString(GetStateAndFilterModeNameValueParams()) + "\n";
  message += GetNameValuesString(GetShaderVariablesNameValueParams()) + "\n";
  message += GetNameValuesString(GetMusicSettingsNameValueParams()) + "\n";
  message += GetNameValuesString(GetFilterBufferValueParams()) + "\n";
  message += GetNameValuesString(GetZoomFilterFxNameValueParams()) + "\n";
  message += GetNameValuesString(GetFilterEffectsNameValueParams());

  return message;
}

// TODO(glk) - clean this up.
namespace
{

[[nodiscard]] auto GetString(const std::unordered_set<std::string>& theSet) noexcept -> std::string
{
  auto str = std::string{};

  for (const auto& val : theSet)
  {
    str += "  " + val + "\n";
  }

  str.pop_back();
  return str;
}

} // namespace

auto GoomStateMonitor::GetStateAndFilterModeNameValueParams() const -> UTILS::NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "";
  return {
      GetPair(PARAM_GROUP, "State", m_visualFx->GetCurrentStateName()),
      GetPair(PARAM_GROUP, "Color Maps", GetString(m_visualFx->GetCurrentColorMapsNames())),
      GetPair(PARAM_GROUP, "Filter Mode", m_filterSettingsService->GetCurrentFilterModeName()),
      GetPair(PARAM_GROUP,
              "Previous Filter Mode",
              m_filterSettingsService->GetPreviousFilterModeName()),
  };
}

auto GoomStateMonitor::GetShaderVariablesNameValueParams() const -> UTILS::NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "Shader";
  const auto& lastShaderVariables    = m_visualFx->GetLastShaderVariables();
  return {
      GetPair(PARAM_GROUP, "Contrast", lastShaderVariables.contrast),
      GetPair(PARAM_GROUP, "MinChan", lastShaderVariables.contrastMinChannelValue),
      GetPair(PARAM_GROUP, "Brightness", lastShaderVariables.brightness),
      GetPair(PARAM_GROUP, "HueShift", lastShaderVariables.hueShift),
      GetPair(PARAM_GROUP, "ChromaFactor", lastShaderVariables.chromaFactor),
      GetPair(PARAM_GROUP, "BaseColorMultiplier", lastShaderVariables.baseColorMultiplier),
  };
}

inline auto GoomStateMonitor::GetFilterBufferValueParams() const -> UTILS::NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "Filter Buffer";
  const auto& filterBufferSettings =
      m_filterSettingsService->GetFilterSettings().filterBufferSettings;
  return {
      GetPair(PARAM_GROUP, "TranLerpIncrement", filterBufferSettings.tranLerpIncrement),
      GetPair(PARAM_GROUP, "TranLerpToMaxSwitchMult", filterBufferSettings.tranLerpToMaxSwitchMult),
  };
}

inline auto GoomStateMonitor::GetFilterEffectsNameValueParams() const -> UTILS::NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "Filter Settings";
  const auto& filterEffectsSettings =
      m_filterSettingsService->GetFilterSettings().filterEffectsSettings;
  return {
      GetPair(
          PARAM_GROUP,
          "Middle",
          Point2dInt{filterEffectsSettings.zoomMidpoint.x, filterEffectsSettings.zoomMidpoint.y}),
      GetPair(PARAM_GROUP, "After Effects", filterEffectsSettings.afterEffectsVelocityContribution),
  };
}

inline auto GoomStateMonitor::GetMusicSettingsNameValueParams() const -> UTILS::NameValuePairs
{
  return m_musicSettingsReactor->GetNameValueParams();
}

inline auto GoomStateMonitor::GetZoomFilterFxNameValueParams() const -> UTILS::NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "ZoomFilterFx";
  return m_filterBuffersService->GetNameValueParams(PARAM_GROUP);
}

} // namespace GOOM::CONTROL
