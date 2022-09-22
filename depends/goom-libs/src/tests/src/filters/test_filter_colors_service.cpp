#include "catch2/catch.hpp"
#include "filter_fx/filter_buffers.h"
#include "filter_fx/filter_colors_service.h"
#include "filter_fx/filter_utils/zoom_filter_coefficients.h"
#include "goom_graphic.h"
#include "utils/math/goom_rand.h"

#include <array>
#include <numeric>

namespace GOOM::UNIT_TESTS
{

using FILTER_FX::FilterColorsService;
using FILTER_FX::ZoomFilterBuffers;
using FILTER_FX::FILTER_UTILS::ZOOM_FILTER_COEFFS::NeighborhoodCoeffArray;
using UTILS::MATH::GoomRand;

static constexpr size_t WIDTH  = 120;
static constexpr size_t HEIGHT = 70;
static const auto GOOM_RAND    = GoomRand{};

inline auto GetColor(const PixelChannelType red,
                     const PixelChannelType green,
                     const PixelChannelType blue) -> GOOM::Pixel
{
  return Pixel{red, green, blue, MAX_ALPHA};
}

TEST_CASE("FilterColorsService", "[FilterColorsService]")
{
  auto filterColorsService = FilterColorsService{GOOM_RAND};

  auto pixelBuffer = PixelBuffer{
      {WIDTH, HEIGHT}
  };

  static constexpr auto RED1   = 50U;
  static constexpr auto GREEN1 = 150U;
  static constexpr auto BLUE1  = 200U;
  static constexpr auto RED2   = 80U;
  static constexpr auto GREEN2 = 120U;
  static constexpr auto BLUE2  = 50U;
  static constexpr auto RED3   = 120U;
  static constexpr auto GREEN3 = 200U;
  static constexpr auto BLUE3  = 150U;
  static constexpr auto RED4   = 120U;
  static constexpr auto GREEN4 = 200U;
  static constexpr auto BLUE4  = 60U;

  static constexpr auto MAX_SUM_COEFF = channel_limits<uint32_t>::max() + 1U;

  const auto coeffs = NeighborhoodCoeffArray{
      {50, 60, 70, 76}
  };
  REQUIRE(MAX_SUM_COEFF == std::accumulate(cbegin(coeffs), cend(coeffs), 0U));
  REQUIRE(4 == coeffs.size());
  // GCC Won't link with this:  REQUIRE(PixelBuffer::NUM_NBRS == coeffs.val.size());

  SECTION("Correct new color")
  {
    static constexpr auto X = 5;
    static constexpr auto Y = 5;

    const auto sourcePointInfo = ZoomFilterBuffers::SourcePointInfo{
        {static_cast<int32_t>(X), static_cast<int32_t>(Y)},
        &coeffs, false
    };

    pixelBuffer(X, Y)         = GetColor(RED1, GREEN1, BLUE1);
    pixelBuffer(X + 1, Y)     = GetColor(RED2, GREEN2, BLUE2);
    pixelBuffer(X, Y + 1)     = GetColor(RED3, GREEN3, BLUE3);
    pixelBuffer(X + 1, Y + 1) = GetColor(RED4, GREEN4, BLUE4);

    const auto expectedR =
        (coeffs[0] * RED1 + coeffs[1] * RED2 + coeffs[2] * RED3 + coeffs[3] * RED4) / MAX_SUM_COEFF;
    const auto expectedG =
        (coeffs[0] * GREEN1 + coeffs[1] * GREEN2 + coeffs[2] * GREEN3 + coeffs[3] * GREEN4) /
        MAX_SUM_COEFF;
    const auto expectedB =
        (coeffs[0] * BLUE1 + coeffs[1] * BLUE2 + coeffs[2] * BLUE3 + coeffs[3] * BLUE4) /
        MAX_SUM_COEFF;

    const auto expectedColor = GetColor(expectedR, expectedG, expectedB);
    const auto pixelNeighbours =
        pixelBuffer.Get4RHBNeighbours(sourcePointInfo.screenPoint.x, sourcePointInfo.screenPoint.y);
    const auto newColor = filterColorsService.GetNewColor(sourcePointInfo, pixelNeighbours);

    UNSCOPED_INFO("expectedColor = " << expectedColor.ToString());
    UNSCOPED_INFO("newColor = " << newColor.ToString());
    REQUIRE(expectedColor == newColor);
  }
}

} // namespace GOOM::UNIT_TESTS
