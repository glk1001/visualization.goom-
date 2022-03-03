#include "goom_state_strings.h"

#include "goom_all_visual_fx.h"
#include "goom_graphic.h"
#include "goom_music_settings_reactor.h"
#include "utils/name_value_pairs.h"
#include "visual_fx/filters/filter_settings_service.h"

#include <format>

namespace GOOM::CONTROL
{

using UTILS::GetNameValuesString;
using VISUAL_FX::FILTERS::FilterSettingsService;
using VISUAL_FX::FILTERS::ZoomFilterBufferSettings;
using VISUAL_FX::FILTERS::ZoomFilterEffectsSettings;

auto GetGoomStateStrings(const GoomAllVisualFx& visualFx,
                         const GoomMusicSettingsReactor& musicSettingsReactor,
                         const VISUAL_FX::FILTERS::FilterSettingsService& filterSettingsService,
                         const GoomShaderEffects& shaderEffects) -> std::string
{

  std::string message;

  message += std20::format("State: {}\n", visualFx.GetCurrentStateName());
  message += std20::format("Filter Mode: {}\n", filterSettingsService.GetCurrentFilterMode());
  message +=
      std20::format("Previous Filter Mode: {}\n", filterSettingsService.GetPreviousFilterMode());

  const ZoomFilterBufferSettings& filterBufferSettings =
      filterSettingsService.GetFilterSettings().filterBufferSettings;
  message += std20::format("tranLerpIncrement: {}\n", filterBufferSettings.tranLerpIncrement);
  message +=
      std20::format("tranLerpToMaxSwitchMult: {}\n", filterBufferSettings.tranLerpToMaxSwitchMult);

  message += std20::format("exposure: {}\n", visualFx.GetCurrentExposure());
  message += std20::format("contrast: {}\n", shaderEffects.contrast);
  message += std20::format("minChan: {}\n", shaderEffects.contrastMinChannelValue);
  message += std20::format("brightness: {}\n", shaderEffects.brightness);

  const ZoomFilterEffectsSettings& filterEffectsSettings =
      filterSettingsService.GetFilterSettings().filterEffectsSettings;
  message += std20::format("middleX: {}\n", filterEffectsSettings.zoomMidpoint.x);
  message += std20::format("middleY: {}\n", filterEffectsSettings.zoomMidpoint.y);

  message += GetNameValuesString(musicSettingsReactor.GetNameValueParams()) + "\n";

  message += GetNameValuesString(visualFx.GetZoomFilterFxNameValueParams());

  return message;
}

} // namespace GOOM::CONTROL
