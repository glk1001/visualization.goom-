module;

#include <format>
#include <string>
#include <unordered_set>
#include <vector>

module Goom.Control.GoomStateMonitor;

import Goom.Control.GoomAllVisualFx;
import Goom.Control.GoomMessageDisplayer;
import Goom.Control.GoomMusicSettingsReactor;
import Goom.FilterFx.FilterBuffersService;
import Goom.FilterFx.FilterSettingsService;
import Goom.Lib.Point2d;

namespace GOOM::CONTROL
{

using FILTER_FX::FilterBuffersService;
using FILTER_FX::FilterSettingsService;
using UTILS::GetNameValueStrings;
using UTILS::GetPair;
using UTILS::NameValuePairs;

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

auto GoomStateMonitor::GetCurrentState() const -> std::vector<MessageGroup>
{
  using enum MessageGroupColors;

  auto messageGroup = std::vector<MessageGroup>(NUM_GROUPS + 1);

  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
  // NOTE: Clunky but first index 0 is reserved for the caller.
  messageGroup.at(1) = {.color    = PURE_RED,
                        .messages = GetNameValueStrings(GetStateNameValueParams())};
  messageGroup.at(2) = {.color    = PURE_LIME,
                        .messages = GetNameValueStrings(GetShaderVariablesNameValueParams())};
  messageGroup.at(3) = {.color    = PURE_BLUE,
                        .messages = GetNameValueStrings(GetMusicSettingsNameValueParams())};
  messageGroup.at(4) = {.color    = TIA_MARIA,
                        .messages = GetNameValueStrings(GetFilterSettingsNameValueParams())};
  messageGroup.at(5) = {.color    = ORANGE,
                        .messages = GetNameValueStrings(GetFilterEffectsNameValueParams())};
  messageGroup.at(6) = {.color    = PURE_YELLOW,
                        .messages = GetNameValueStrings(GetFilterBufferValueParams())};
  messageGroup.at(7) = {.color    = PIZAZZ,
                        .messages = GetNameValueStrings(GetZoomEffectsNameValueParams())};
  messageGroup.at(8) = {.color    = LIGHT_ORCHID,
                        .messages = GetNameValueStrings(GetFilterAfterEffectsNameValueParams())};
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

  return messageGroup;
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

auto GoomStateMonitor::GetStateNameValueParams() const -> NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "Main";
  return {
      GetPair(PARAM_GROUP, "State", m_visualFx->GetCurrentStateName()),
      GetPair(PARAM_GROUP, "Color Maps", GetString(GoomAllVisualFx::GetCurrentColorMapsNames())),
  };
}

auto GoomStateMonitor::GetShaderVariablesNameValueParams() const -> NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "Shader";
  const auto& lastFrameMiscData      = m_visualFx->GetFrameMiscData();
  return {
      GetPair(PARAM_GROUP, "Brightness", lastFrameMiscData.brightness),
      GetPair(PARAM_GROUP, "HueShift", lastFrameMiscData.hueShift),
      GetPair(PARAM_GROUP, "ChromaFactor", lastFrameMiscData.chromaFactor),
      GetPair(PARAM_GROUP, "BaseColorMultiplier", lastFrameMiscData.baseColorMultiplier),
  };
}

inline auto GoomStateMonitor::GetMusicSettingsNameValueParams() const -> NameValuePairs
{
  return m_musicSettingsReactor->GetNameValueParams();
}

inline auto GoomStateMonitor::GetFilterSettingsNameValueParams() const -> NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "Filter Settings";
  const auto& filterSettings         = m_filterSettingsService->GetFilterSettings();
  const auto& filterEffectsSettings  = filterSettings.filterEffectsSettings;
  return {
      GetPair(PARAM_GROUP, "Filter", m_filterSettingsService->GetCurrentFilterModeName()),
      GetPair(PARAM_GROUP, "Previous Filter", m_filterSettingsService->GetPreviousFilterModeName()),
      GetPair(PARAM_GROUP, "Midpoint", filterEffectsSettings.zoomMidpoint),
      GetPair(PARAM_GROUP, "After Mult", filterEffectsSettings.afterEffectsVelocityMultiplier),
  };
}

inline auto GoomStateMonitor::GetFilterEffectsNameValueParams() const -> NameValuePairs
{
  const auto& filterEffectsSettings =
      m_filterSettingsService->GetFilterSettings().filterEffectsSettings;
  return {filterEffectsSettings.zoomAdjustmentEffect->GetZoomAdjustmentEffectNameValueParams()};
}

inline auto GoomStateMonitor::GetFilterBufferValueParams() const -> NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "Filter Buffer";
  const auto& transformBufferLerpData =
      m_filterSettingsService->GetFilterSettings().transformBufferLerpData;
  return {GetPair(PARAM_GROUP,
                  "params",
                  std::format("{:.2f}, {:.2f}, {}",
                              transformBufferLerpData.GetLerpFactor(),
                              transformBufferLerpData.GetIncrement(),
                              transformBufferLerpData.GetUseSFunction()))};
}

inline auto GoomStateMonitor::GetZoomEffectsNameValueParams() const -> NameValuePairs
{
  return m_filterBuffersService->GetNameValueParams();
}

inline auto GoomStateMonitor::GetFilterAfterEffectsNameValueParams() const -> NameValuePairs
{
  return m_filterBuffersService->GetAfterEffectsNameValueParams();
}

} // namespace GOOM::CONTROL
