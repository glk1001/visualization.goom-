#define CATCH_CONFIG_NO_POSIX_SIGNALS

#ifndef GOOM_DEBUG
#define GOOM_DEBUG
#endif

#include "catch2/catch.hpp"
#include "goom_config.h"
#include "goom_graphic.h"
#include "utils/math/misc.h"

#include <chrono>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

namespace GOOM::UNIT_TESTS
{

using UTILS::MATH::FloatsEqual;
using namespace std::chrono_literals;
using std::chrono::high_resolution_clock;
using std::chrono::microseconds;

static constexpr size_t WIDTH = 1280;
static constexpr size_t HEIGHT = 720;
static constexpr PixelChannelType TEST_R = 200;
static constexpr PixelChannelType TEST_G = 100;
static constexpr PixelChannelType TEST_B = 50;

TEST_CASE("Pixels")
{
  SECTION("Pixel RGB")
  {
    static constexpr auto pixel = Pixel{{TEST_R, TEST_G, TEST_B}};
    REQUIRE(FloatsEqual(static_cast<float>(TEST_R) / channel_limits<float>::max(), pixel.RFlt()));
    REQUIRE(FloatsEqual(static_cast<float>(TEST_G) / channel_limits<float>::max(), pixel.GFlt()));
    REQUIRE(FloatsEqual(static_cast<float>(TEST_B) / channel_limits<float>::max(), pixel.BFlt()));
  }
  SECTION("Pixel uint32_t RGB")
  {
    static constexpr uint32_t RED = 299;
    static constexpr uint32_t GREEN = 200;
    static constexpr uint32_t BLUE = MAX_CHANNEL_VALUE_HDR + 10;
    static constexpr uint32_t ALPHA = 256;
    static constexpr auto pixel = MakePixel(RED, GREEN, BLUE, ALPHA);
    REQUIRE(pixel.R() == RED);
    REQUIRE(pixel.G() == GREEN);
    REQUIRE(pixel.B() == MAX_CHANNEL_VALUE_HDR);
    REQUIRE(pixel.A() == 255);
  }
  SECTION("Pixel Set")
  {
    auto pixel = Pixel{{TEST_R, TEST_G, TEST_B}};
    pixel.SetA(1);
    REQUIRE(1 == pixel.A());
    pixel.SetR(10);
    REQUIRE(10 == pixel.R());
    pixel.SetG(20);
    REQUIRE(20 == pixel.G());
    pixel.SetB(30);
    REQUIRE(30 == pixel.B());
  }
  SECTION("Pixel ==")
  {
    static constexpr auto pixel1 = Pixel{{TEST_R, TEST_G, TEST_B}};
    static constexpr auto pixel2 = Pixel{pixel1};
    static constexpr auto pixel3 = Pixel{{TEST_R + 1, TEST_G, TEST_B}};
    REQUIRE(pixel1 == pixel2);
    REQUIRE(!(pixel1 == pixel3));
    static constexpr auto pixelBlack = Pixel{
        {0, 0, 0, 255}
    };
    REQUIRE(pixelBlack == BLACK_PIXEL);
    static constexpr auto pixelWhite = Pixel{{MAX_COLOR_VAL, MAX_COLOR_VAL, MAX_COLOR_VAL, MAX_ALPHA}};
    REQUIRE(pixelWhite == WHITE_PIXEL);
  }
  SECTION("Pixel Multiply Scalar")
  {
    static constexpr auto pixel = Pixel{{TEST_R, TEST_G, TEST_B}};
    static constexpr auto rChannel = pixel.R();
    static constexpr uint32_t SCALAR = 32;
    REQUIRE(MultiplyChannelColorByScalar(SCALAR, pixel.R()) ==
            ((SCALAR * rChannel) / MAX_COLOR_VAL));
  }
  SECTION("Pixel Multiply Channels")
  {
    static constexpr auto pixel = Pixel{{TEST_R, TEST_G, TEST_B}};
    static constexpr auto rChannel = pixel.R();
    static constexpr auto gChannel = pixel.G();
    static constexpr auto rgProduct = rChannel * gChannel;
    REQUIRE(MultiplyColorChannels(pixel.R(), pixel.G()) == rgProduct / MAX_COLOR_VAL);
  }
}

auto GetPixelCount(const PixelBuffer& buffer, const Pixel& pixel) -> uint32_t
{
  auto count = 0U;
  for (size_t y = 0; y < HEIGHT; ++y)
  {
    for (size_t x = 0; x < WIDTH; ++x)
    {
      if (buffer(x, y) == pixel)
      {
        count++;
      }
    }
  }
  return count;
}

TEST_CASE("PixelBuffers")
{
  SECTION("PixelBuffer copy")
  {
    auto srceBuffer = PixelBuffer{{WIDTH, HEIGHT}};
    REQUIRE(srceBuffer.GetWidth() == WIDTH);
    REQUIRE(srceBuffer.GetHeight() == HEIGHT);
    static constexpr auto testPixel = Pixel{{TEST_R, TEST_G, TEST_B}};
    srceBuffer.Fill(testPixel);

    auto destBuffer = PixelBuffer{{WIDTH, HEIGHT}};
    srceBuffer.CopyTo(destBuffer);

    REQUIRE(destBuffer.GetWidth() == WIDTH);
    REQUIRE(destBuffer.GetHeight() == HEIGHT);
    REQUIRE(GetPixelCount(srceBuffer, testPixel) == GetPixelCount(destBuffer, testPixel));
  }

  SECTION("PixelBuffer copy time")
  {
    static constexpr size_t NUM_LOOPS = 100;
    static constexpr auto testPixel = Pixel{{TEST_R, TEST_G, TEST_B}};

    auto intSrceBuff = std::make_unique<std::vector<PixelIntType>>(WIDTH * HEIGHT);
    const auto intTestPixel = testPixel.Rgba();
    std::fill(intSrceBuff->begin(), intSrceBuff->end(), intTestPixel);
    auto intDestBuff = std::make_unique<std::vector<PixelIntType>>(WIDTH * HEIGHT);

    auto startTime = high_resolution_clock::now();
    for (auto i = 0U; i < NUM_LOOPS; ++i)
    {
      std::memmove(intDestBuff->data(), intSrceBuff->data(), WIDTH * HEIGHT * sizeof(PixelIntType));
    }
    auto finishTime = high_resolution_clock::now();
    const auto durationMemmove = std::chrono::duration_cast<microseconds>(finishTime - startTime);

    REQUIRE(std::count(intSrceBuff->cbegin(), intSrceBuff->cend(), intTestPixel) ==
    std::count(intDestBuff->cbegin(), intDestBuff->cend(), intTestPixel));

    auto srceBuffer = std::make_unique<PixelBuffer>(Dimensions{WIDTH, HEIGHT});
    srceBuffer->Fill(testPixel);
    auto destBuffer = std::make_unique<PixelBuffer>(Dimensions{WIDTH, HEIGHT});

    startTime = high_resolution_clock::now();
    for (auto i = 0U; i < NUM_LOOPS; ++i)
    {
      srceBuffer->CopyTo(*destBuffer);
    }
    finishTime = high_resolution_clock::now();
    const auto durationCopyTo = std::chrono::duration_cast<microseconds>(finishTime - startTime);

    REQUIRE(GetPixelCount(*srceBuffer, testPixel) == GetPixelCount(*destBuffer, testPixel));

    const auto tolerance = (durationCopyTo * 10) / 100;
    UNSCOPED_INFO("durationCopyTo.count() = " << durationCopyTo.count());
    UNSCOPED_INFO("durationMemmove.count() = " << durationMemmove.count());
    UNSCOPED_INFO("tolerance.count() = " << tolerance.count());
    REQUIRE(durationCopyTo < (durationMemmove + tolerance));
  }

  SECTION("PixelBuffer Get4RHBNeighbours")
  {
    auto buffer = PixelBuffer{{WIDTH, HEIGHT}};
    static constexpr auto testPixel1 = Pixel{{TEST_R+0, TEST_G+0, TEST_B+0}};
    static constexpr auto testPixel2 = Pixel{{TEST_R+1, TEST_G+1, TEST_B+1}};
    static constexpr auto testPixel3 = Pixel{{TEST_R+2, TEST_G+2, TEST_B+2}};
    static constexpr auto testPixel4 = Pixel{{TEST_R+3, TEST_G+3, TEST_B+3}};
    buffer.Fill(WHITE_PIXEL);

    static constexpr size_t X = 20;
    static constexpr size_t Y = 40;
    buffer(X, Y) = testPixel1;
    buffer(X+1, Y) = testPixel2;
    buffer(X, Y+1) = testPixel3;
    buffer(X+1, Y+1) = testPixel4;

    const auto pixel4RHBNeighbours = buffer.Get4RHBNeighbours(X, Y);

    REQUIRE(testPixel1 == pixel4RHBNeighbours[0]);
    REQUIRE(testPixel2 == pixel4RHBNeighbours[1]);
    REQUIRE(testPixel3 == pixel4RHBNeighbours[2]);
    REQUIRE(testPixel4 == pixel4RHBNeighbours[3]);
  }

  SECTION("PixelBuffer RowIter")
  {
    static constexpr auto testPixel = Pixel{{TEST_R, TEST_G, TEST_B}};
    auto buffer = PixelBuffer{{WIDTH, HEIGHT}};
    buffer.Fill(WHITE_PIXEL);
    static constexpr size_t Y = 10;
    for (auto x = 0U; x < WIDTH; ++x)
    {
      buffer(x, Y) = testPixel;
    }

    uint32_t count = 0;
    const auto destRowIter = buffer.GetRowIter(Y);
    const auto destRowBegin = std::get<0>(destRowIter);
    const auto destRowEnd = std::get<1>(destRowIter);
    for (auto destRowBuff = destRowBegin; destRowBuff != destRowEnd; ++destRowBuff)
    {
      if (*destRowBuff == testPixel)
      {
        count++;
      }
    }
    REQUIRE(count == WIDTH);
  }
}

} // namespace GOOM::UNIT_TESTS
