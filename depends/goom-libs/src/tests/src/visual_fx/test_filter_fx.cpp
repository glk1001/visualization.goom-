#include "catch2/catch.hpp"
#include "goom_plugin_info.h"
#include "utils/goom_rand.h"
#include "utils/parallel_utils.h"
#include "visual_fx/filters/filter_settings.h"
#include "visual_fx/filters/filter_settings_service.h"
#include "visual_fx/filters/speed_coefficients_effect_factory.h"
#include "visual_fx/zoom_filter_fx.h"

using GOOM::PluginInfo;
using GOOM::UTILS::GoomRand;
using GOOM::UTILS::Parallel;
using GOOM::VISUAL_FX::ZoomFilterFx;
using GOOM::VISUAL_FX::FILTERS::CreateSpeedCoefficientsEffect;
using GOOM::VISUAL_FX::FILTERS::FilterSettingsService;
using GOOM::VISUAL_FX::FILTERS::ZoomFilterBufferSettings;

constexpr size_t WIDTH = 120;
constexpr size_t HEIGHT = 70;

constexpr const char* RESOURCES_DIRECTORY = "";

TEST_CASE("ZoomFilterFx", "[ZoomFilterFx]")
{
  Parallel parallel{-1};
  const PluginInfo goomInfo{WIDTH, HEIGHT};
  const GoomRand goomRand{};
  FilterSettingsService filterSettingsService{parallel, goomInfo, goomRand, RESOURCES_DIRECTORY,
                                              CreateSpeedCoefficientsEffect};
  ZoomFilterFx zoomFilterFx{parallel, goomInfo, filterSettingsService.GetFilterBuffersService(),
                            filterSettingsService.GetFilterColorsService()};

  SECTION("Correct initial lerp factor") { REQUIRE(0 == zoomFilterFx.GetTranLerpFactor()); }
  SECTION("Correct lerp factor after an increment")
  {
    const ZoomFilterBufferSettings filterBufferSettings = {127, 1.0F};
    zoomFilterFx.UpdateFilterBufferSettings(filterBufferSettings);
    REQUIRE(127 == zoomFilterFx.GetTranLerpFactor());
  }
}
