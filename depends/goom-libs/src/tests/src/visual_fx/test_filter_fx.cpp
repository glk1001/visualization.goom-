#include "catch2/catch.hpp"
#include "goom_plugin_info.h"
#include "utils/math/goom_rand.h"
#include "utils/parallel_utils.h"
#include "visual_fx/filters/filter_buffers_service.h"
#include "visual_fx/filters/filter_colors_service.h"
#include "visual_fx/filters/filter_settings.h"
#include "visual_fx/filters/filter_settings_service.h"
#include "visual_fx/filters/filter_zoom_vector.h"
#include "visual_fx/filters/normalized_coords.h"
#include "visual_fx/filters/speed_coefficients_effect_factory.h"
#include "visual_fx/zoom_filter_fx.h"

#include <memory>

namespace GOOM::UNIT_TESTS
{

using UTILS::Parallel;
using UTILS::MATH::GoomRand;
using VISUAL_FX::ZoomFilterFx;
using VISUAL_FX::FILTERS::CreateSpeedCoefficientsEffect;
using VISUAL_FX::FILTERS::FilterBuffersService;
using VISUAL_FX::FILTERS::FilterColorsService;
using VISUAL_FX::FILTERS::FilterSettingsService;
using VISUAL_FX::FILTERS::FilterZoomVector;
using VISUAL_FX::FILTERS::NormalizedCoordsConverter;
using VISUAL_FX::FILTERS::ZoomFilterBuffers;
using VISUAL_FX::FILTERS::ZoomFilterBufferSettings;

static constexpr size_t WIDTH = 120;
static constexpr size_t HEIGHT = 70;

static constexpr const char* RESOURCES_DIRECTORY = "";

TEST_CASE("ZoomFilterFx", "[ZoomFilterFx]")
{
  Parallel parallel{-1};
  const PluginInfo goomInfo{WIDTH, HEIGHT};
  const GoomRand goomRand{};
  FilterSettingsService filterSettingsService{goomInfo, goomRand, RESOURCES_DIRECTORY,
                                              CreateSpeedCoefficientsEffect};
  const NormalizedCoordsConverter normalizedCoordsConverter{
      WIDTH, HEIGHT, ZoomFilterBuffers::MIN_SCREEN_COORD_ABS_VAL};
  ZoomFilterFx zoomFilterFx{
      parallel, goomInfo,
      std::make_unique<FilterBuffersService>(
          parallel, goomInfo, normalizedCoordsConverter,
          std::make_unique<FilterZoomVector>(WIDTH, RESOURCES_DIRECTORY, goomRand,
                                             normalizedCoordsConverter)),
      std::make_unique<FilterColorsService>()};

  SECTION("Correct initial lerp factor") { REQUIRE(0 == zoomFilterFx.GetTranLerpFactor()); }
  SECTION("Correct lerp factor after an increment")
  {
    const ZoomFilterBufferSettings filterBufferSettings = {127, 1.0F};
    zoomFilterFx.UpdateFilterBufferSettings(filterBufferSettings);
    REQUIRE(127 == zoomFilterFx.GetTranLerpFactor());
  }
}

} // namespace GOOM::UNIT_TESTS
