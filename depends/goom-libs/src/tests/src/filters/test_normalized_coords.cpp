#include "catch2/catch.hpp"
#include "utils/mathutils.h"
#include "v2d.h"
#include "visual_fx/filters/normalized_coords.h"

#include <cmath>

using GOOM::FILTERS::NormalizedCoords;
using GOOM::V2dInt;
using GOOM::UTILS::floats_equal;

constexpr uint32_t WIDTH = 1280;
constexpr uint32_t HEIGHT = 720;
static const float MIN_COORD_VAL =
    (NormalizedCoords::MAX_NORMALIZED_COORD - NormalizedCoords::MIN_NORMALIZED_COORD) /
    static_cast<float>(WIDTH - 1);

inline auto ConvertToScreen(const float normalizedValue) -> int32_t
{
  return static_cast<int32_t>(static_cast<float>(WIDTH - 1) *
      ((normalizedValue - NormalizedCoords::MIN_NORMALIZED_COORD) /
      (NormalizedCoords::MAX_NORMALIZED_COORD - NormalizedCoords::MIN_NORMALIZED_COORD)));
}

TEST_CASE("Normalized Coords Values", "[CoordsValues]")
{
  NormalizedCoords::SetScreenDimensions(WIDTH, HEIGHT, MIN_COORD_VAL);

  SECTION("Min coords")
  {
    const NormalizedCoords coords{V2dInt{0, 0}};
    UNSCOPED_INFO("coords.GetX() = " << coords.GetX());
    REQUIRE(floats_equal(coords.GetX(), NormalizedCoords::MIN_NORMALIZED_COORD));
    UNSCOPED_INFO("coords.GetY() = " << coords.GetY());
    REQUIRE(floats_equal(coords.GetY(), NormalizedCoords::MIN_NORMALIZED_COORD));

    const V2dInt screenCoords = coords.GetScreenCoordsFlt().ToInt();
    UNSCOPED_INFO("screenCoords.x = " << screenCoords.x);
    REQUIRE(screenCoords.x == 0);
    UNSCOPED_INFO("screenCoords.y = " << screenCoords.y);
    REQUIRE(screenCoords.y == 0);
  }

  SECTION("Max coords")
  {
    const NormalizedCoords coords{V2dInt{WIDTH - 1U, HEIGHT - 1U}};
    UNSCOPED_INFO("coords.GetX() = " << coords.GetX());
    REQUIRE(floats_equal(coords.GetX(), NormalizedCoords::MAX_NORMALIZED_COORD));
    const float maxY =
        NormalizedCoords::MIN_NORMALIZED_COORD +
        (NormalizedCoords::MAX_NORMALIZED_COORD - NormalizedCoords::MIN_NORMALIZED_COORD) *
            (static_cast<float>(HEIGHT) / static_cast<float>(WIDTH));
    UNSCOPED_INFO("coords.GetY() = " << coords.GetY());
    UNSCOPED_INFO("maxY = " << maxY);
    REQUIRE(floats_equal(coords.GetY(), maxY, MIN_COORD_VAL));

    const V2dInt screenCoords = coords.GetScreenCoordsFlt().ToInt();
    UNSCOPED_INFO("screenCoords.x = " << screenCoords.x);
    REQUIRE(screenCoords.x == WIDTH - 1);
    UNSCOPED_INFO("screenCoords.y = " << screenCoords.y);
    REQUIRE(screenCoords.y == HEIGHT - 1);
  }

  SECTION("Zero coords (middle)")
  {
    const NormalizedCoords coords{V2dInt{WIDTH / 2, WIDTH / 2}};
    UNSCOPED_INFO("coords.GetX() = " << coords.GetX());
    REQUIRE(floats_equal(coords.GetX(), 0.0F, MIN_COORD_VAL));
    UNSCOPED_INFO("coords.GetY() = " << coords.GetY());
    REQUIRE(floats_equal(coords.GetY(), 0.0F, MIN_COORD_VAL));

    const V2dInt screenCoords = coords.GetScreenCoordsFlt().ToInt();
    UNSCOPED_INFO("screenCoords.x = " << screenCoords.x);
    REQUIRE(screenCoords.x == WIDTH / 2);
    UNSCOPED_INFO("screenCoords.y = " << screenCoords.y);
    REQUIRE(screenCoords.y == WIDTH / 2);
  }

  SECTION("From normalized")
  {
    const NormalizedCoords coords{NormalizedCoords{0.5F, 0.3F}};
    UNSCOPED_INFO("coords.GetX() = " << coords.GetX());
    REQUIRE(floats_equal(coords.GetX(), 0.5F));
    UNSCOPED_INFO("coords.GetY() = " << coords.GetY());
    REQUIRE(floats_equal(coords.GetY(), 0.3F));

    const V2dInt screenCoords = coords.GetScreenCoordsFlt().ToInt();
    const int32_t x = ConvertToScreen(0.5F);
    UNSCOPED_INFO("screenCoords.x = " << screenCoords.x);
    UNSCOPED_INFO("x = " << x);
    REQUIRE(screenCoords.x == x);
  }
}

TEST_CASE("Normalized Coords Increments", "[CoordsIncrements]")
{
  NormalizedCoords::SetScreenDimensions(WIDTH, HEIGHT, MIN_COORD_VAL);

  SECTION("Inc")
  {
    NormalizedCoords coords{V2dInt{0, 0}};
    coords.Inc();
    UNSCOPED_INFO("coords.GetX() = " << coords.GetX());
    REQUIRE(floats_equal(coords.GetX(), NormalizedCoords::MIN_NORMALIZED_COORD + MIN_COORD_VAL));
    UNSCOPED_INFO("coords.GetY() = " << coords.GetY());
    REQUIRE(floats_equal(coords.GetY(), NormalizedCoords::MIN_NORMALIZED_COORD + MIN_COORD_VAL));
  }
  SECTION("IncX")
  {
    NormalizedCoords coords{V2dInt{0, 0}};
    coords.IncX();
    UNSCOPED_INFO("coords.GetX() = " << coords.GetX());
    REQUIRE(floats_equal(coords.GetX(), NormalizedCoords::MIN_NORMALIZED_COORD + MIN_COORD_VAL));
  }
  SECTION("IncY")
  {
    NormalizedCoords coords{V2dInt{0, 0}};
    coords.IncY();
    UNSCOPED_INFO("coords.GetY() = " << coords.GetY());
    REQUIRE(floats_equal(coords.GetY(), NormalizedCoords::MIN_NORMALIZED_COORD + MIN_COORD_VAL));
  }
}

TEST_CASE("Normalized Coords Operations", "[CoordsOperations]")
{
  NormalizedCoords::SetScreenDimensions(WIDTH, HEIGHT, MIN_COORD_VAL);

  SECTION("Plus")
  {
    NormalizedCoords coords{0.5F, 0.5F};
    coords += NormalizedCoords{0.5F, 0.5F};
    UNSCOPED_INFO("coords.GetX() = " << coords.GetX());
    REQUIRE(floats_equal(coords.GetX(), 1.0F));
    UNSCOPED_INFO("coords.GetY() = " << coords.GetY());
    REQUIRE(floats_equal(coords.GetY(), 1.0F));
  }

  SECTION("Minus")
  {
    NormalizedCoords coords{1.0F, 1.0F};
    coords -= NormalizedCoords{0.5F, 0.5F};
    UNSCOPED_INFO("coords.GetX() = " << coords.GetX());
    REQUIRE(floats_equal(coords.GetX(), 0.5F));
    UNSCOPED_INFO("coords.GetY() = " << coords.GetY());
    REQUIRE(floats_equal(coords.GetY(), 0.5F));
  }

  SECTION("Scalar Mult")
  {
    NormalizedCoords coords{0.5F, 0.5F};
    coords *= 0.5F;
    UNSCOPED_INFO("coords.GetX() = " << coords.GetX());
    REQUIRE(floats_equal(coords.GetX(), 0.25F));
    UNSCOPED_INFO("coords.GetY() = " << coords.GetY());
    REQUIRE(floats_equal(coords.GetY(), 0.25F));
  }
}
