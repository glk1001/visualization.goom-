#include "goom_graphic.h"
#include "utils/graphics/pixel_utils.h"

#include <cstdint>

#if __clang_major__ >= 16 // NOLINT: Can't include header for this.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wunsafe-buffer-usage"
#endif
#include <catch2/catch_test_macros.hpp>
#if __clang_major__ >= 16 // NOLINT: Can't include header for this.
#pragma GCC diagnostic pop
#endif

namespace GOOM::UNIT_TESTS
{

using UTILS::GRAPHICS::GetChannelColorMultiplyByScalar;
using UTILS::GRAPHICS::GetColorAdd;
using UTILS::GRAPHICS::GetColorChannelAdd;
using UTILS::GRAPHICS::GetColorChannelMultiply;
using UTILS::GRAPHICS::GetColorMultiply;
using UTILS::GRAPHICS::MakePixel;

TEST_CASE("Make Pixel")
{
  static constexpr auto RED   = 299U;
  static constexpr auto GREEN = 200U;
  static constexpr auto BLUE  = MAX_CHANNEL_VALUE_HDR + 10U;
  static constexpr auto ALPHA = 256U;
  static constexpr auto PIXEL = MakePixel(RED, GREEN, BLUE, ALPHA);
  REQUIRE(PIXEL.R() == RED);
  REQUIRE(PIXEL.G() == GREEN);
  REQUIRE(PIXEL.B() == MAX_CHANNEL_VALUE_HDR);
  REQUIRE(PIXEL.A() == ALPHA);
}

TEST_CASE("Pixel Channels Add")
{
  REQUIRE(GetColorChannelAdd(100, 120) == 220);
  REQUIRE(GetColorChannelAdd(200, 120) == 320);
  REQUIRE(GetColorChannelAdd(0, 120) == 120);
  REQUIRE(GetColorChannelAdd(0, 0) == 0);
}

TEST_CASE("Pixel Add")
{
  static constexpr auto ALPHA  = 128U;
  static constexpr auto COLOR1 = Pixel{
      {100, 50, 20}
  };
  static constexpr auto COLOR2 = Pixel{
      {120, 250, 70}
  };
  static constexpr auto COLOR3 = GetColorAdd(COLOR1, COLOR2, ALPHA);
  REQUIRE(static_cast<uint32_t>(COLOR3.R()) == 220);
  REQUIRE(static_cast<uint32_t>(COLOR3.G()) == 300);
  REQUIRE(static_cast<uint32_t>(COLOR3.B()) == 90);
  REQUIRE(static_cast<uint32_t>(COLOR3.A()) == ALPHA);
}

TEST_CASE("Pixel Channels Multiply")
{
  static constexpr PixelChannelType TEST_R = 200;
  static constexpr PixelChannelType TEST_G = 100;
  static constexpr PixelChannelType TEST_B = 50;

  SECTION("Pixel Multiply Scalar")
  {
    static constexpr auto PIXEL = Pixel{
        {TEST_R, TEST_G, TEST_B}
    };
    static constexpr auto R_CHANNEL = PIXEL.R();
    static constexpr auto SCALAR    = 32U;
    REQUIRE(GetChannelColorMultiplyByScalar(SCALAR, PIXEL.R()) ==
            ((SCALAR * R_CHANNEL) / MAX_COLOR_VAL));
  }
  SECTION("Pixel Multiply Channels")
  {
    static constexpr auto PIXEL = Pixel{
        {TEST_R, TEST_G, TEST_B}
    };
    static constexpr auto R_CHANNEL  = PIXEL.R();
    static constexpr auto G_CHANNEL  = PIXEL.G();
    static constexpr auto RG_PRODUCT = R_CHANNEL * G_CHANNEL;
    REQUIRE(GetColorChannelMultiply(PIXEL.R(), PIXEL.G()) == RG_PRODUCT / MAX_COLOR_VAL);
  }
}

TEST_CASE("Pixel Multiply")
{
  static constexpr auto ALPHA  = 128U;
  static constexpr auto COLOR1 = Pixel{
      {100, 50, 20}
  };
  static constexpr auto COLOR2 = Pixel{
      {120, 250, 70}
  };

  static constexpr auto COLOR3 = GetColorMultiply(COLOR1, COLOR2, ALPHA);
  REQUIRE(static_cast<uint32_t>(COLOR3.R()) ==
          (COLOR1.R() * COLOR2.R()) / static_cast<uint32_t>(MAX_COLOR_VAL));
  REQUIRE(static_cast<uint32_t>(COLOR3.G()) ==
          (COLOR1.G() * COLOR2.G()) / static_cast<uint32_t>(MAX_COLOR_VAL));
  REQUIRE(static_cast<uint32_t>(COLOR3.B()) ==
          (COLOR1.B() * COLOR2.B()) / static_cast<uint32_t>(MAX_COLOR_VAL));
  REQUIRE(static_cast<uint32_t>(COLOR3.A()) == ALPHA);
}

} // namespace GOOM::UNIT_TESTS
