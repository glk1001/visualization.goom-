#include "catch2/catch.hpp"
#include "filter_fx/filter_buffers.h"
#include "filter_fx/filter_colors_service.h"
#include "goom_graphic.h"

#include <array>
#include <numeric>

namespace GOOM::UNIT_TESTS
{

using FILTER_FX::FilterColorsService;
using FILTER_FX::ZoomFilterBuffers;

static constexpr size_t WIDTH  = 120;
static constexpr size_t HEIGHT = 70;

inline auto GetColor(const PixelChannelType red,
                     const PixelChannelType green,
                     const PixelChannelType blue) -> GOOM::Pixel
{
  return Pixel{red, green, blue, MAX_ALPHA};
}

TEST_CASE("FilterColorsService", "[FilterColorsService]")
{
  auto filterColorsService = FilterColorsService{};

  auto pixelBuffer = PixelBuffer{
      {WIDTH, HEIGHT}
  };

  static constexpr auto R1 = 50U;
  static constexpr auto G1 = 150U;
  static constexpr auto B1 = 200U;
  static constexpr auto R2 = 80U;
  static constexpr auto G2 = 120U;
  static constexpr auto B2 = 50U;
  static constexpr auto R3 = 120U;
  static constexpr auto G3 = 200U;
  static constexpr auto B3 = 150U;
  static constexpr auto R4 = 120U;
  static constexpr auto G4 = 200U;
  static constexpr auto B4 = 60U;

  static constexpr auto MAX_SUM_COEFF = channel_limits<uint32_t>::max() + 1U;

  const auto coeffs = ZoomFilterBuffers::NeighborhoodCoeffArray{
      {50, 60, 70, 76},
      false
  };
  REQUIRE(MAX_SUM_COEFF == std::accumulate(cbegin(coeffs.val), cend(coeffs.val), 0U));
  REQUIRE(4 == coeffs.val.size());
  // GCC Won't link with this:  REQUIRE(PixelBuffer::NUM_NBRS == coeffs.val.size());

  SECTION("Correct new color")
  {
    static constexpr auto X = 5;
    static constexpr auto Y = 5;

    const auto sourcePointInfo = ZoomFilterBuffers::SourcePointInfo{
        {static_cast<int32_t>(X), static_cast<int32_t>(Y)},
        coeffs, false
    };

    pixelBuffer(X, Y)         = GetColor(R1, G1, B1);
    pixelBuffer(X + 1, Y)     = GetColor(R2, G2, B2);
    pixelBuffer(X, Y + 1)     = GetColor(R3, G3, B3);
    pixelBuffer(X + 1, Y + 1) = GetColor(R4, G4, B4);

    const auto expectedR =
        (coeffs.val[0] * R1 + coeffs.val[1] * R2 + coeffs.val[2] * R3 + coeffs.val[3] * R4) /
        MAX_SUM_COEFF;
    const auto expectedG =
        (coeffs.val[0] * G1 + coeffs.val[1] * G2 + coeffs.val[2] * G3 + coeffs.val[3] * G4) /
        MAX_SUM_COEFF;
    const auto expectedB =
        (coeffs.val[0] * B1 + coeffs.val[1] * B2 + coeffs.val[2] * B3 + coeffs.val[3] * B4) /
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
