#ifndef GOOM_DEBUG
#define GOOM_DEBUG
#endif

#include "goom/goom_config.h"

#include <catch2/catch_test_macros.hpp>

import Goom.Utils.Math.Misc;
import Goom.Lib.GoomGraphic;

namespace GOOM::UNIT_TESTS
{

using UTILS::MATH::FloatsEqual;

static constexpr PixelChannelType TEST_R = 200;
static constexpr PixelChannelType TEST_G = 100;
static constexpr PixelChannelType TEST_B = 50;

// NOLINTBEGIN(bugprone-chained-comparison): Catch2 needs to fix this.
// NOLINTBEGIN(readability-function-cognitive-complexity)
TEST_CASE("Pixels")
{
  SECTION("Pixel RGB")
  {
    static constexpr auto PIXEL = Pixel{
        {TEST_R, TEST_G, TEST_B}
    };
    REQUIRE(FloatsEqual(static_cast<float>(TEST_R) / channel_limits<float>::max(), PIXEL.RFlt()));
    REQUIRE(FloatsEqual(static_cast<float>(TEST_G) / channel_limits<float>::max(), PIXEL.GFlt()));
    REQUIRE(FloatsEqual(static_cast<float>(TEST_B) / channel_limits<float>::max(), PIXEL.BFlt()));
  }
  SECTION("Pixel Set")
  {
    auto pixel = Pixel{
        {TEST_R, TEST_G, TEST_B}
    };
    pixel.SetA(1);
    REQUIRE(1 == pixel.A());
    static constexpr auto R_VAL = 10;
    pixel.SetR(R_VAL);
    REQUIRE(R_VAL == pixel.R());
    static constexpr auto G_VAL = 20;
    pixel.SetG(G_VAL);
    REQUIRE(G_VAL == pixel.G());
    static constexpr auto B_VAL = 30;
    pixel.SetB(B_VAL);
    REQUIRE(B_VAL == pixel.B());
  }
  SECTION("Pixel ==")
  {
    static constexpr auto PIXEL1 = Pixel{
        {TEST_R, TEST_G, TEST_B}
    };
    static constexpr auto PIXEL2 = Pixel{PIXEL1};
    static constexpr auto PIXEL3 = Pixel{
        {TEST_R + 1, TEST_G, TEST_B}
    };
    REQUIRE(PIXEL1 == PIXEL2);
    REQUIRE(not(PIXEL1 == PIXEL3));
    static constexpr auto EXPECTED_BLACK = Pixel{
        {0, 0, 0, MAX_ALPHA}
    };
    REQUIRE(EXPECTED_BLACK == BLACK_PIXEL);
    static constexpr auto EXPECTED_WHITE = Pixel{
        {MAX_COLOR_VAL, MAX_COLOR_VAL, MAX_COLOR_VAL, MAX_ALPHA}
    };
    REQUIRE(EXPECTED_WHITE == WHITE_PIXEL);
  }
  SECTION("Pixel Rgba")
  {
    static constexpr auto PIXEL = Pixel{
        {0xFEU, 0xEEU, 0x32U, 0x00FFU}
    };
    static constexpr auto INT_PIXEL = 0x00FE00EE003200FFU;
    REQUIRE(Rgba(PIXEL) == INT_PIXEL);
  }
}
// NOLINTEND(readability-function-cognitive-complexity)
// NOLINTEND(bugprone-chained-comparison)

} // namespace GOOM::UNIT_TESTS
