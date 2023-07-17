#include "control/goom_sound_events.h"
#include "filter_fx/filter_buffers_service.h"
#include "filter_fx/filter_effects/zoom_in_coefficients_effect_factory.h"
#include "filter_fx/filter_settings.h"
#include "filter_fx/filter_settings_service.h"
#include "filter_fx/filter_zoom_vector.h"
#include "filter_fx/normalized_coords.h"
#include "goom_plugin_info.h"
#include "sound_info.h"
#include "utils/math/goom_rand.h"
#include "utils/parallel_utils.h"

#include <memory>

#if __clang_major__ >= 16
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wunsafe-buffer-usage"
#endif
#include "catch2/catch_test_macros.hpp"
#if __clang_major__ >= 16
#pragma GCC diagnostic pop
#endif

namespace GOOM::UNIT_TESTS
{

using CONTROL::GoomSoundEvents;
using FILTER_FX::FilterBuffersService;
using FILTER_FX::FilterSettingsService;
using FILTER_FX::FilterTransformBufferSettings;
using FILTER_FX::FilterZoomVector;
using FILTER_FX::NormalizedCoordsConverter;
using FILTER_FX::Viewport;
using FILTER_FX::FILTER_BUFFERS::MIN_SCREEN_COORD_ABS_VAL;
using FILTER_FX::FILTER_EFFECTS::CreateZoomInCoefficientsEffect;
using UTILS::Parallel;
using UTILS::MATH::GoomRand;

static constexpr auto WIDTH  = 120U;
static constexpr auto HEIGHT = 70U;

static constexpr auto* RESOURCES_DIRECTORY = "";

TEST_CASE("FilterBuffersService")
{
  Parallel parallel{-1};
  const auto soundInfo       = SoundInfo{};
  const auto goomSoundEvents = GoomSoundEvents{soundInfo};
  const auto goomInfo        = PluginInfo{
             {WIDTH, HEIGHT},
             goomSoundEvents
  };
  const auto goomRand              = GoomRand{};
  const auto filterSettingsService = FilterSettingsService{
      goomInfo, goomRand, RESOURCES_DIRECTORY, CreateZoomInCoefficientsEffect};
  const auto normalizedCoordsConverter = NormalizedCoordsConverter{
      {WIDTH, HEIGHT},
      MIN_SCREEN_COORD_ABS_VAL
  };
  auto filterBuffersService =
      FilterBuffersService{parallel,
                           goomInfo,
                           normalizedCoordsConverter,
                           std::make_unique<FilterZoomVector>(
                               WIDTH, RESOURCES_DIRECTORY, goomRand, normalizedCoordsConverter)};

  SECTION("Correct initial lerp factor")
  {
    REQUIRE(0.0F == filterBuffersService.GetTransformBufferLerpFactor());
  }
  SECTION("Correct lerp factor after an increment")
  {
    static constexpr auto DEFAULT_VIEWPORT = Viewport{};
    const auto filterBufferSettings        = FilterTransformBufferSettings{
               {FilterSettingsService::DEFAULT_TRAN_LERP_INCREMENT, 1.0F},
               DEFAULT_VIEWPORT
    };
    filterBuffersService.SetFilterTransformBufferSettings(filterBufferSettings);
    REQUIRE(FilterSettingsService::DEFAULT_TRAN_LERP_INCREMENT ==
            filterBuffersService.GetTransformBufferLerpFactor());
  }
}

} // namespace GOOM::UNIT_TESTS
