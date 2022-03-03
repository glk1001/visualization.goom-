#pragma once

#include "goom_all_visual_fx.h"
#include "goom_graphic.h"
#include "goom_music_settings_reactor.h"
#include "visual_fx/filters/filter_settings_service.h"

#include <string>

namespace GOOM::CONTROL
{

[[nodiscard]] auto GetGoomStateStrings(
    const GoomAllVisualFx& visualFx,
    const GoomMusicSettingsReactor& musicSettingsReactor,
    const VISUAL_FX::FILTERS::FilterSettingsService& filterSettingsService,
    const GoomShaderEffects& shaderEffects) -> std::string;

} // namespace GOOM::CONTROL
