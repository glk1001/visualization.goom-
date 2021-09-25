#include "catch2/catch.hpp"
#include "filters/filter_settings.h"
#include "filters/filter_settings_service.h"
#include "goom/goom_graphic.h"
#include "goom/v2d.h"
#include "goom_plugin_info.h"
#include "goomutils/parallel_utils.h"
#include "visual_fx/zoom_filter_fx.h"

using GOOM::Pixel;
using GOOM::PixelBuffer;
using GOOM::PluginInfo;
using GOOM::V2dInt;
using GOOM::FILTERS::FilterSettingsService;
using GOOM::FILTERS::ZoomFilterBufferSettings;
using GOOM::UTILS::Parallel;
using GOOM::VISUAL_FX::ZoomFilterFx;

constexpr size_t WIDTH = 120;
constexpr size_t HEIGHT = 70;

constexpr const char* RESOURCES_DIRECTORY = "";

TEST_CASE("ZoomFilterFx", "[ZoomFilterFx]")
{
  Parallel parallel{-1};
  const PluginInfo goomInfo{WIDTH, HEIGHT};
  FilterSettingsService filterSettingsService{parallel, goomInfo, RESOURCES_DIRECTORY};
  ZoomFilterFx zoomFilter_fx{parallel, goomInfo,
                             std::move(filterSettingsService.GetFilterBuffersService()),
                             std::move(filterSettingsService.GetFilterColorsService())};

  SECTION("Correct initial lerp factor") { REQUIRE(0 == zoomFilter_fx.GetTranLerpFactor()); }
  SECTION("Correct lerp factor after an increment")
  {
    ZoomFilterBufferSettings filterBufferSettings = {127, 1.0F};
    zoomFilter_fx.UpdateFilterBufferSettings(filterBufferSettings);
    REQUIRE(127 == zoomFilter_fx.GetTranLerpFactor());
  }
}
