#include "catch2/catch.hpp"
#include "filter_fx/filter_buffers.h"
#include "filter_fx/filter_colors_service.h"
#include "goom/goom_graphic.h"

#include <array>
#include <numeric>

namespace GOOM::UNIT_TESTS
{

using FILTER_FX::FilterColorsService;
using FILTER_FX::ZoomFilterBuffers;

static constexpr size_t WIDTH = 120;
static constexpr size_t HEIGHT = 70;

inline auto GetColor(const uint32_t red, const uint32_t green, const uint32_t blue) -> GOOM::Pixel
{
  return Pixel{red, green, blue, MAX_ALPHA};
}

TEST_CASE("FilterColorsService", "[FilterColorsService]")
{
  FilterColorsService filterColorsService{};
  PixelBuffer pixelBuffer{{WIDTH, HEIGHT}};

  static constexpr uint32_t R1 = 50;
  static constexpr uint32_t G1 = 150;
  static constexpr uint32_t B1 = 200;
  static constexpr uint32_t R2 = 80;
  static constexpr uint32_t G2 = 120;
  static constexpr uint32_t B2 = 50;
  static constexpr uint32_t R3 = 120;
  static constexpr uint32_t G3 = 200;
  static constexpr uint32_t B3 = 150;
  static constexpr uint32_t R4 = 120;
  static constexpr uint32_t G4 = 200;
  static constexpr uint32_t B4 = 60;

  static constexpr uint32_t MAX_SUM_COEFF = channel_limits<uint32_t>::max() + 1;

  const ZoomFilterBuffers::NeighborhoodCoeffArray coeffs = {{50, 60, 70, 76}, false};
  REQUIRE(MAX_SUM_COEFF == std::accumulate(cbegin(coeffs.val), cend(coeffs.val), 0U));
  REQUIRE(4 == coeffs.val.size());
  // GCC Won't link with this:  REQUIRE(PixelBuffer::NUM_NBRS == coeffs.val.size());

  SECTION("Correct new color")
  {
    static constexpr size_t X = 5;
    static constexpr size_t Y = 5;

    const ZoomFilterBuffers::SourcePointInfo sourcePointInfo = {
        {static_cast<int32_t>(X), static_cast<int32_t>(Y)}, coeffs, false};

    pixelBuffer(X, Y) = GetColor(R1, G1, B1);
    pixelBuffer(X + 1, Y) = GetColor(R2, G2, B2);
    pixelBuffer(X, Y + 1) = GetColor(R3, G3, B3);
    pixelBuffer(X + 1, Y + 1) = GetColor(R4, G4, B4);

    const uint32_t expectedR =
        (coeffs.val[0] * R1 + coeffs.val[1] * R2 + coeffs.val[2] * R3 + coeffs.val[3] * R4) /
        MAX_SUM_COEFF;
    const uint32_t expectedG =
        (coeffs.val[0] * G1 + coeffs.val[1] * G2 + coeffs.val[2] * G3 + coeffs.val[3] * G4) /
        MAX_SUM_COEFF;
    const uint32_t expectedB =
        (coeffs.val[0] * B1 + coeffs.val[1] * B2 + coeffs.val[2] * B3 + coeffs.val[3] * B4) /
        MAX_SUM_COEFF;

    const Pixel expectedColor = GetColor(expectedR, expectedG, expectedB);
    const Pixel newColor = filterColorsService.GetNewColor(pixelBuffer, sourcePointInfo);

    UNSCOPED_INFO("expectedColor = " << expectedColor.ToString());
    UNSCOPED_INFO("newColor = " << newColor.ToString());
    REQUIRE(expectedColor == newColor);
  }
}

} // namespace GOOM::UNIT_TESTS
