#pragma once

#include "goom_all_visual_fx.h"
#include "goom_graphic.h"
#include "goom_music_settings_reactor.h"
#include "utils/name_value_pairs.h"
#include "visual_fx/filters/filter_settings_service.h"

#include <string>

namespace GOOM::CONTROL
{

class GoomStateMonitor
{
public:
  GoomStateMonitor(const GoomAllVisualFx& visualFx,
                   const GoomMusicSettingsReactor& musicSettingsReactor,
                   const VISUAL_FX::FILTERS::FilterSettingsService& filterSettingsService) noexcept;

  [[nodiscard]] auto GetCurrentState() const -> std::string;

private:
  const GoomAllVisualFx& m_visualFx;
  const GoomMusicSettingsReactor& m_musicSettingsReactor;
  const VISUAL_FX::FILTERS::FilterSettingsService& m_filterSettingsService;

  [[nodiscard]] auto GetStateAndFilterModeNameValueParams() const -> UTILS::NameValuePairs;
  [[nodiscard]] auto GetShaderEffectsNameValueParams() const -> UTILS::NameValuePairs;
  [[nodiscard]] auto GetFilterBufferValueParams() const -> UTILS::NameValuePairs;
  [[nodiscard]] auto GetFilterEffectsNameValueParams() const -> UTILS::NameValuePairs;
  [[nodiscard]] auto GetMusicSettingsNameValueParams() const -> UTILS::NameValuePairs;
  [[nodiscard]] auto GetZoomFilterFxNameValueParams() const -> UTILS::NameValuePairs;
};

} // namespace GOOM::CONTROL
