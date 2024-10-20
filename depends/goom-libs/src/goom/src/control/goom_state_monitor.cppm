module;

#include <string>
#include <vector>

export module Goom.Control.GoomStateMonitor;

import Goom.Control.GoomAllVisualFx;
import Goom.Control.GoomMessageDisplayer;
import Goom.Control.GoomMusicSettingsReactor;
import Goom.FilterFx.FilterBuffersService;
import Goom.FilterFx.FilterSettingsService;
import Goom.Utils.NameValuePairs;
import Goom.Utils.EnumUtils;

using GOOM::UTILS::NameValuePairs;
using GOOM::UTILS::NUM;

export namespace GOOM::CONTROL
{

class GoomStateMonitor
{
public:
  GoomStateMonitor(const GoomAllVisualFx& visualFx,
                   const GoomMusicSettingsReactor& musicSettingsReactor,
                   const FILTER_FX::FilterSettingsService& filterSettingsService,
                   const FILTER_FX::FilterBuffersService& filterBuffersService) noexcept;
  [[nodiscard]] auto GetCurrentState() const -> std::vector<MessageGroup>;

private:
  const GoomAllVisualFx* m_visualFx;
  const GoomMusicSettingsReactor* m_musicSettingsReactor;
  const FILTER_FX::FilterSettingsService* m_filterSettingsService;
  const FILTER_FX::FilterBuffersService* m_filterBuffersService;

  static constexpr auto NUM_GROUPS = 8U;
  static_assert(NUM_GROUPS < NUM<MessageGroupColors>);
  [[nodiscard]] auto GetStateNameValueParams() const -> NameValuePairs;
  [[nodiscard]] auto GetShaderVariablesNameValueParams() const -> NameValuePairs;
  [[nodiscard]] auto GetMusicSettingsNameValueParams() const -> NameValuePairs;

  [[nodiscard]] auto GetFilterSettingsNameValueParams() const -> NameValuePairs;
  [[nodiscard]] auto GetFilterEffectsNameValueParams() const -> NameValuePairs;

  [[nodiscard]] auto GetFilterBufferValueParams() const -> NameValuePairs;
  [[nodiscard]] auto GetZoomEffectsNameValueParams() const -> NameValuePairs;
  [[nodiscard]] auto GetFilterAfterEffectsNameValueParams() const -> NameValuePairs;
};

} // namespace GOOM::CONTROL
