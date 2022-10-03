#include "catch2/catch.hpp"
#include "filter_fx/normalized_coords.h"
#include "point2d.h"
#include "utils/math/misc.h"

#include <cmath>

namespace GOOM::UNIT_TESTS
{

using FILTER_FX::NormalizedCoords;
using FILTER_FX::NormalizedCoordsConverter;
using UTILS::MATH::FloatsEqual;

static constexpr uint32_t WIDTH  = 1280U;
static constexpr uint32_t HEIGHT = 720U;
static constexpr float MIN_COORD_VAL =
    (NormalizedCoords::MAX_NORMALIZED_COORD - NormalizedCoords::MIN_NORMALIZED_COORD) /
    static_cast<float>(WIDTH - 1);
static constexpr NormalizedCoordsConverter NORMALIZED_COORDS_CONVERTER{
    {WIDTH, HEIGHT},
    MIN_COORD_VAL
};

constexpr auto ConvertToScreen(const float normalizedValue) -> int32_t
{
  return static_cast<int32_t>(
      static_cast<float>(WIDTH - 1) *
      ((normalizedValue - NormalizedCoords::MIN_NORMALIZED_COORD) /
       (NormalizedCoords::MAX_NORMALIZED_COORD - NormalizedCoords::MIN_NORMALIZED_COORD)));
}

TEST_CASE("Normalized Coords Values", "[CoordsValues]")
{
  SECTION("Min coords")
  {
    static constexpr auto COORDS =
        NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(Point2dInt{0, 0});
    UNSCOPED_INFO("coords.GetX() = " << COORDS.GetX());
    REQUIRE(FloatsEqual(COORDS.GetX(), NormalizedCoords::MIN_NORMALIZED_COORD));
    UNSCOPED_INFO("coords.GetY() = " << COORDS.GetY());
    REQUIRE(FloatsEqual(COORDS.GetY(), NormalizedCoords::MIN_NORMALIZED_COORD));

    const auto screenCoords =
        NORMALIZED_COORDS_CONVERTER.NormalizedToOtherCoordsFlt(COORDS).ToInt();
    UNSCOPED_INFO("screenCoords.x = " << screenCoords.x);
    REQUIRE(screenCoords.x == 0);
    UNSCOPED_INFO("screenCoords.y = " << screenCoords.y);
    REQUIRE(screenCoords.y == 0);
  }

  SECTION("Max coords")
  {
    static constexpr auto COORDS =
        NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(Point2dInt{WIDTH - 1U, HEIGHT - 1U});
    UNSCOPED_INFO("coords.GetX() = " << COORDS.GetX());
    REQUIRE(FloatsEqual(COORDS.GetX(), NormalizedCoords::MAX_NORMALIZED_COORD));
    static constexpr auto MAX_Y =
        NormalizedCoords::MIN_NORMALIZED_COORD +
        (NormalizedCoords::MAX_NORMALIZED_COORD - NormalizedCoords::MIN_NORMALIZED_COORD) *
            (static_cast<float>(HEIGHT) / static_cast<float>(WIDTH));
    UNSCOPED_INFO("coords.GetY() = " << COORDS.GetY());
    UNSCOPED_INFO("maxY = " << MAX_Y);
    REQUIRE(FloatsEqual(COORDS.GetY(), MAX_Y, MIN_COORD_VAL));

    const auto screenCoords =
        NORMALIZED_COORDS_CONVERTER.NormalizedToOtherCoordsFlt(COORDS).ToInt();
    UNSCOPED_INFO("screenCoords.x = " << screenCoords.x);
    REQUIRE(screenCoords.x == WIDTH - 1);
    UNSCOPED_INFO("screenCoords.y = " << screenCoords.y);
    REQUIRE(screenCoords.y == HEIGHT - 1);
  }

  SECTION("Zero coords (middle)")
  {
    static constexpr auto COORDS =
        NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(Point2dInt{WIDTH / 2, WIDTH / 2});
    UNSCOPED_INFO("coords.GetX() = " << COORDS.GetX());
    REQUIRE(FloatsEqual(COORDS.GetX(), 0.0F, MIN_COORD_VAL));
    UNSCOPED_INFO("coords.GetY() = " << COORDS.GetY());
    REQUIRE(FloatsEqual(COORDS.GetY(), 0.0F, MIN_COORD_VAL));

    const auto screenCoords =
        NORMALIZED_COORDS_CONVERTER.NormalizedToOtherCoordsFlt(COORDS).ToInt();
    UNSCOPED_INFO("screenCoords.x = " << screenCoords.x);
    REQUIRE(screenCoords.x == WIDTH / 2);
    UNSCOPED_INFO("screenCoords.y = " << screenCoords.y);
    REQUIRE(screenCoords.y == WIDTH / 2);
  }

  SECTION("From normalized")
  {
    static constexpr auto COORDS = NormalizedCoords{0.5F, 0.3F};
    UNSCOPED_INFO("coords.GetX() = " << COORDS.GetX());
    REQUIRE(FloatsEqual(COORDS.GetX(), 0.5F));
    UNSCOPED_INFO("coords.GetY() = " << COORDS.GetY());
    REQUIRE(FloatsEqual(COORDS.GetY(), 0.3F));

    const auto screenCoords =
        NORMALIZED_COORDS_CONVERTER.NormalizedToOtherCoordsFlt(COORDS).ToInt();
    const int32_t x = ConvertToScreen(0.5F);
    UNSCOPED_INFO("screenCoords.x = " << screenCoords.x);
    UNSCOPED_INFO("x = " << x);
    REQUIRE(screenCoords.x == x);
  }
}

TEST_CASE("Normalized Coords Increments", "[CoordsIncrements]")
{
  SECTION("Inc")
  {
    auto coords = NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(Point2dInt{0, 0});
    NORMALIZED_COORDS_CONVERTER.Inc(coords);
    UNSCOPED_INFO("coords.GetX() = " << coords.GetX());
    REQUIRE(FloatsEqual(coords.GetX(), NormalizedCoords::MIN_NORMALIZED_COORD + MIN_COORD_VAL));
    UNSCOPED_INFO("coords.GetY() = " << coords.GetY());
    REQUIRE(FloatsEqual(coords.GetY(), NormalizedCoords::MIN_NORMALIZED_COORD + MIN_COORD_VAL));
  }
  SECTION("IncX")
  {
    auto coords = NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(Point2dInt{0, 0});
    NORMALIZED_COORDS_CONVERTER.IncX(coords);
    UNSCOPED_INFO("coords.GetX() = " << coords.GetX());
    REQUIRE(FloatsEqual(coords.GetX(), NormalizedCoords::MIN_NORMALIZED_COORD + MIN_COORD_VAL));
  }
  SECTION("IncY")
  {
    auto coords = NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(Point2dInt{0, 0});
    NORMALIZED_COORDS_CONVERTER.IncY(coords);
    UNSCOPED_INFO("coords.GetY() = " << coords.GetY());
    REQUIRE(FloatsEqual(coords.GetY(), NormalizedCoords::MIN_NORMALIZED_COORD + MIN_COORD_VAL));
  }
}

TEST_CASE("Normalized Coords Operations", "[CoordsOperations]")
{
  SECTION("Plus")
  {
    auto coords = NormalizedCoords{0.5F, 0.5F};
    coords += NormalizedCoords{0.5F, 0.5F};
    UNSCOPED_INFO("coords.GetX() = " << coords.GetX());
    REQUIRE(FloatsEqual(coords.GetX(), 1.0F));
    UNSCOPED_INFO("coords.GetY() = " << coords.GetY());
    REQUIRE(FloatsEqual(coords.GetY(), 1.0F));
  }

  SECTION("Minus")
  {
    auto coords = NormalizedCoords{1.0F, 1.0F};
    coords -= NormalizedCoords{0.5F, 0.5F};
    UNSCOPED_INFO("coords.GetX() = " << coords.GetX());
    REQUIRE(FloatsEqual(coords.GetX(), 0.5F));
    UNSCOPED_INFO("coords.GetY() = " << coords.GetY());
    REQUIRE(FloatsEqual(coords.GetY(), 0.5F));
  }

  SECTION("Scalar Mult")
  {
    auto coords = NormalizedCoords{0.5F, 0.5F};
    coords *= 0.5F;
    UNSCOPED_INFO("coords.GetX() = " << coords.GetX());
    REQUIRE(FloatsEqual(coords.GetX(), 0.25F));
    UNSCOPED_INFO("coords.GetY() = " << coords.GetY());
    REQUIRE(FloatsEqual(coords.GetY(), 0.25F));
  }
}

} // namespace GOOM::UNIT_TESTS
