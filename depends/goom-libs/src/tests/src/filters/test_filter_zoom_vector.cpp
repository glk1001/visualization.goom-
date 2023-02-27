#ifndef GOOM_DEBUG
#define GOOM_DEBUG
#endif

// TODO(glk) - Look at a better way to deal with this - Vitesse::SetVitesse
#if not defined(_MSC_VER)
#include "filter_fx/filter_settings.h"
#else
#pragma warning(push)
#pragma warning(disable : 4296)
#include "filter_fx/filter_settings.h"
#pragma warning(pop)
#endif

#include "filter_fx/after_effects/after_effects_types.h"
#include "filter_fx/filter_buffers.h"
#include "filter_fx/filter_effects/uniform_zoom_in_coefficients_effect.h"
#include "filter_fx/filter_effects/zoom_vector_effects.h"
#include "filter_fx/filter_zoom_vector.h"
#include "filter_fx/normalized_coords.h"
#include "goom_config.h"
#include "utils/enum_utils.h"
#include "utils/math/goom_rand.h"

#include <cmath>
#include <memory>

#if __clang_major__ >= 16
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunsafe-buffer-usage"
#endif
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#if __clang_major__ >= 16
#pragma GCC diagnostic pop
#endif

namespace GOOM::UNIT_TESTS
{

using Catch::Approx;
using FILTER_FX::FilterZoomVector;
using FILTER_FX::NormalizedCoords;
using FILTER_FX::NormalizedCoordsConverter;
using FILTER_FX::Vitesse;
using FILTER_FX::ZoomFilterEffectsSettings;
using FILTER_FX::AFTER_EFFECTS::AfterEffectsTypes;
using FILTER_FX::AFTER_EFFECTS::HypercosOverlay;
using FILTER_FX::AFTER_EFFECTS::RotationAdjustments;
using FILTER_FX::FILTER_BUFFERS::MIN_SCREEN_COORD_ABS_VAL;
using FILTER_FX::FILTER_EFFECTS::UniformZoomInCoefficientsEffect;
using FILTER_FX::FILTER_EFFECTS::ZoomVectorEffects;
using UTILS::EnumMap;
using UTILS::MATH::GoomRand;

namespace
{

constexpr auto WIDTH                       = 120;
constexpr auto HEIGHT                      = 70;
constexpr auto* RESOURCES_DIRECTORY        = "";
const auto GOOM_RAND                       = GoomRand{};
constexpr auto NORMALIZED_COORDS_CONVERTER = NormalizedCoordsConverter{
    {WIDTH, HEIGHT},
    MIN_SCREEN_COORD_ABS_VAL
};

[[nodiscard]] constexpr auto GetRelativeSpeed(const uint32_t intSpeed, const bool reverseSpeed)
    -> float
{
  constexpr auto MAX_INT_SPEED = static_cast<float>(Vitesse::MAXIMUM_SPEED);
  const auto absRelativeSpeed  = static_cast<float>(intSpeed) / MAX_INT_SPEED;
  return reverseSpeed ? -absRelativeSpeed : absRelativeSpeed;
}

[[nodiscard]] constexpr auto GetZoomInCoeff(const float relativeSpeed) -> float
{
  constexpr auto BASE_ZOOM_IN_COEFF_FACTOR = ZoomVectorEffects::RAW_BASE_ZOOM_IN_COEFF_FACTOR;
  return BASE_ZOOM_IN_COEFF_FACTOR * (1.0F + relativeSpeed);
}

[[nodiscard]] auto GetZoomFilterEffectsSettings() -> ZoomFilterEffectsSettings
{
  static constexpr auto DEFAULT_ZOOM_MID_X                        = 16;
  static constexpr auto DEFAULT_ZOOM_MID_Y                        = 1;
  static constexpr auto DEFAULT_MAX_ZOOM_IN_COEFF                 = 2.01F;
  static constexpr auto UNIT_BASE_ZOOM_IN_COEFF_FACTOR_MULTIPLIER = 1.0F;

  static constexpr auto ALL_OFF_AFTER_EFFECTS_STATES = EnumMap<AfterEffectsTypes, bool>{{{
      {AfterEffectsTypes::BLOCK_WAVY, false},
      {AfterEffectsTypes::HYPERCOS, false},
      {AfterEffectsTypes::IMAGE_VELOCITY, false},
      {AfterEffectsTypes::NOISE, false},
      {AfterEffectsTypes::PLANES, false},
      {AfterEffectsTypes::ROTATION, false},
      {AfterEffectsTypes::TAN_EFFECT, false},
      {AfterEffectsTypes::XY_LERP_EFFECT, false},
  }}};

  return ZoomFilterEffectsSettings{
      Vitesse{},
      DEFAULT_MAX_ZOOM_IN_COEFF,
      UNIT_BASE_ZOOM_IN_COEFF_FACTOR_MULTIPLIER,
      std::make_shared<UniformZoomInCoefficientsEffect>(),
      {DEFAULT_ZOOM_MID_X, DEFAULT_ZOOM_MID_Y},
      {
              HypercosOverlay::NONE,
              ALL_OFF_AFTER_EFFECTS_STATES, RotationAdjustments{},
              },
  };
}

auto TestZoomInPoint(FilterZoomVector& filterZoomVector,
                     ZoomFilterEffectsSettings& filterSettings,
                     const bool reverseSpeed,
                     const uint32_t speedInc)
{
  static constexpr auto COORDS = NormalizedCoords{1.0F, 1.0F};

  UNSCOPED_INFO("speedInc = " << speedInc);

  const auto intSpeed      = Vitesse::STOP_SPEED + speedInc;
  const auto relativeSpeed = GetRelativeSpeed(intSpeed, reverseSpeed);
  UNSCOPED_INFO("relativeSpeed = " << relativeSpeed);
  REQUIRE(-1.0F <= relativeSpeed);
  REQUIRE(relativeSpeed <= 1.0F);
  filterSettings.vitesse.SetVitesse(intSpeed);
  REQUIRE(filterSettings.vitesse.GetRelativeSpeed() == Approx(relativeSpeed));

  const auto baseZoomInCoeff     = GetZoomInCoeff(relativeSpeed);
  const auto zoomInFactor        = 1.0F - baseZoomInCoeff;
  const auto expectedZoomInPoint = NormalizedCoords{zoomInFactor, zoomInFactor};
  UNSCOPED_INFO("baseZoomInCoeff = " << baseZoomInCoeff);
  UNSCOPED_INFO("zoomInFactor = " << zoomInFactor);

  filterZoomVector.SetFilterSettings(filterSettings);
  const auto zoomInPoint = filterZoomVector.GetZoomInPoint(COORDS, COORDS);
  REQUIRE(zoomInPoint.GetX() == Approx(expectedZoomInPoint.GetX()));
  REQUIRE(zoomInPoint.GetY() == Approx(expectedZoomInPoint.GetY()));
}

} // namespace

TEST_CASE("FilterZoomVector")
{
  auto filterZoomVector =
      FilterZoomVector{WIDTH, RESOURCES_DIRECTORY, GOOM_RAND, NORMALIZED_COORDS_CONVERTER};

  auto filterSettings = GetZoomFilterEffectsSettings();

  SECTION("Zero Speed")
  {
    const auto coords = NormalizedCoords{1.0F, 1.0F};

    filterSettings.vitesse.SetVitesse(Vitesse::STOP_SPEED);
    REQUIRE(filterSettings.vitesse.GetRelativeSpeed() == Approx(0.0F));
    const auto baseZoomInCoeff     = GetZoomInCoeff(0.0F);
    const auto zoomInFactor        = 1.0F - baseZoomInCoeff;
    const auto expectedZoomInPoint = zoomInFactor * coords;

    filterZoomVector.SetFilterSettings(filterSettings);
    REQUIRE(filterZoomVector.GetZoomInPoint(coords, coords).GetX() ==
            Approx(expectedZoomInPoint.GetX()));
    REQUIRE(filterZoomVector.GetZoomInPoint(coords, coords).GetY() ==
            Approx(expectedZoomInPoint.GetY()));
  }

  SECTION("Non-zero Speed")
  {
    static constexpr auto NUM_SPEEDS = 2U;

    for (auto i = 0U; i < NUM_SPEEDS; ++i)
    {
      const auto reverseSpeed = 1 == i;
      filterSettings.vitesse.SetReverseVitesse(reverseSpeed);

      for (auto speedInc = 0U; speedInc <= Vitesse::MAXIMUM_SPEED; ++speedInc)
      {
        TestZoomInPoint(filterZoomVector, filterSettings, reverseSpeed, speedInc);
      }
    }
  }
}

} // namespace GOOM::UNIT_TESTS
