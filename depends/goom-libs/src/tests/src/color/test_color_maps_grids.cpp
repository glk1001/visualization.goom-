#include "color/color_data/color_map_enums.h"
#include "color/color_maps.h"
#include "color/color_maps_base.h"
#include "color/color_maps_grids.h"
#include "color/color_utils.h"
#include "goom/goom_graphic.h"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#if not defined(_MSC_VER)
#include <vivid.h>
#else
#pragma warning(push)
#pragma warning(disable : 4201)
#pragma warning(disable : 4242)
#pragma warning(disable : 4244)
#include <vivid.h>
#pragma warning(pop)
#endif

#if __clang_major__ >= 16 // NOLINT: Can't include header for this.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wunsafe-buffer-usage"
#endif
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#if __clang_major__ >= 16 // NOLINT: Can't include header for this.
#pragma GCC diagnostic pop
#endif

namespace GOOM::UNIT_TESTS
{

using Catch::Approx;
using COLOR::ColorMapPtrWrapper;
using COLOR::ColorMapsGrid;
using COLOR::GetRgbColorLerp;
using COLOR::IColorMap;
using COLOR::COLOR_DATA::ColorMapName;
using UTILS::MATH::TValue;

namespace
{

class TestColorMap : public IColorMap
{
public:
  TestColorMap(const std::string& mapName, vivid::ColorMap colorMap) noexcept;

  [[nodiscard]] auto GetNumStops() const -> size_t override { return m_vividColorMap.numStops(); }
  [[nodiscard]] auto GetMapName() const -> ColorMapName override { return ColorMapName::ACCENT; }
  [[nodiscard]] auto GetColor(float t) const -> Pixel override;

  // static auto GetColorMix(const Pixel& col1, const Pixel& col2, float t) -> Pixel;

private:
  std::string m_mapName;
  vivid::ColorMap m_vividColorMap;
};

TestColorMap::TestColorMap(const std::string& mapName, vivid::ColorMap colorMap) noexcept
  : m_mapName{mapName}, m_vividColorMap{std::move(colorMap)}
{
}

inline auto TestColorMap::GetColor(const float t) const -> Pixel
{
  const auto rgb8 = vivid::col8_t{vivid::rgb8::fromRgb(m_vividColorMap.at(t))};
  return Pixel{
      {rgb8.r, rgb8.g, rgb8.b, MAX_ALPHA}  // NOLINT: union hard to fix here
  };
}

/*
inline auto TestColorMap::GetColorMix(const Pixel& col1, const Pixel& col2, const float t) -> Pixel
{
  return GetRgbColorLerp(col1, col2, t);
}
 */

// NOLINTBEGIN(cert-err58-cpp): Not sure how to fix this.
inline const auto SIMPLE_WHITE_MAP_STOPS = std::vector<vivid::srgb_t>{
    {BLACK_PIXEL.RFlt(), BLACK_PIXEL.GFlt(), BLACK_PIXEL.BFlt()},
    {WHITE_PIXEL.RFlt(), WHITE_PIXEL.GFlt(), WHITE_PIXEL.BFlt()},
};
inline const auto SIMPLE_WHITE_MAP = TestColorMap{"SimpleWhiteMap", SIMPLE_WHITE_MAP_STOPS};

inline const auto SIMPLE_RED_MAP_STOPS = std::vector<vivid::srgb_t>{
    {BLACK_PIXEL.RFlt(), BLACK_PIXEL.GFlt(), BLACK_PIXEL.BFlt()},
    {              1.0F,               0.0F,               0.0F},
};
inline const auto SIMPLE_RED_MAP = TestColorMap{"SimpleRedMap", SIMPLE_RED_MAP_STOPS};

inline const auto SIMPLE_GREEN_MAP_STOPS = std::vector<vivid::srgb_t>{
    {BLACK_PIXEL.RFlt(), BLACK_PIXEL.GFlt(), BLACK_PIXEL.BFlt()},
    {              0.0F,               1.0F,               0.0F},
};
inline const auto SIMPLE_GREEN_MAP = TestColorMap{"SimpleGreenMap", SIMPLE_GREEN_MAP_STOPS};

inline const auto SIMPLE_BLUE_MAP_STOPS = std::vector<vivid::srgb_t>{
    {BLACK_PIXEL.RFlt(), BLACK_PIXEL.GFlt(), BLACK_PIXEL.BFlt()},
    {              0.0F,               0.0F,               1.0F},
};
inline const auto SIMPLE_BLUE_MAP = TestColorMap{"SimpleBlueMap", SIMPLE_BLUE_MAP_STOPS};

inline const auto VERTICAL_BASE_COLORS = std::vector<Pixel>{
    Pixel{MAX_COLOR_VAL,            0U,            0U, MAX_ALPHA},
    Pixel{           0U, MAX_COLOR_VAL,            0U, MAX_ALPHA},
    Pixel{           0U,            0U, MAX_COLOR_VAL, MAX_ALPHA},
};
// NOLINTEND(cert-err58-cpp)

constexpr auto NUM_VERTICAL_COLORS = 3U;

[[nodiscard]] auto GetColorMapsGrid(
    const UTILS::MATH::TValue& verticalT,
    const ColorMapsGrid::ColorMixingTFunc& colorMixingTFunc) noexcept -> ColorMapsGrid
{
  const auto horizontalColorMaps = std::vector<ColorMapPtrWrapper>{
      ColorMapPtrWrapper{&SIMPLE_WHITE_MAP},
  };
  const auto verticalColorMaps = std::vector<ColorMapPtrWrapper>{
      ColorMapPtrWrapper{&SIMPLE_RED_MAP},
      ColorMapPtrWrapper{&SIMPLE_GREEN_MAP},
      ColorMapPtrWrapper{&SIMPLE_BLUE_MAP},
  };

  return ColorMapsGrid{horizontalColorMaps, verticalT, verticalColorMaps, colorMixingTFunc};
}

constexpr auto COLOR_MIX_T = 0.5F;

// NOLINTBEGIN(bugprone-chained-comparison): Catch2 needs to fix this.

auto TestColorElement(const Pixel& gridColor,
                      const float tHorizontal,
                      const Pixel& verticalBaseColor,
                      const float tVertical) noexcept -> void
{
  static constexpr auto LERP_ERROR = 1.0F / static_cast<float>(MAX_COLOR_VAL);

  const auto horizontalColor = GetRgbColorLerp(BLACK_PIXEL, WHITE_PIXEL, tHorizontal);
  REQUIRE(horizontalColor.RFlt() == Approx(tHorizontal).margin(LERP_ERROR));
  REQUIRE(horizontalColor.GFlt() == Approx(tHorizontal).margin(LERP_ERROR));
  REQUIRE(horizontalColor.BFlt() == Approx(tHorizontal).margin(LERP_ERROR));

  const auto verticalColor = GetRgbColorLerp(BLACK_PIXEL, verticalBaseColor, tVertical);
  const auto mixedColor    = GetRgbColorLerp(verticalColor, horizontalColor, COLOR_MIX_T);
  REQUIRE(gridColor.RFlt() == Approx(mixedColor.RFlt()).margin(LERP_ERROR));
  REQUIRE(gridColor.GFlt() == Approx(mixedColor.GFlt()).margin(LERP_ERROR));
  REQUIRE(gridColor.BFlt() == Approx(mixedColor.BFlt()).margin(LERP_ERROR));
}

} // namespace

TEST_CASE("Test GetCurrentHorizontalLineColors")
{
  static constexpr auto NUM_VERTICAL_STEPS = 4U;

  auto verticalT = TValue{
      {TValue::StepType::SINGLE_CYCLE, NUM_VERTICAL_STEPS}
  };
  const auto colorMixingTFunc = []([[maybe_unused]] const float tX, [[maybe_unused]] const float tY)
  { return COLOR_MIX_T; };
  static constexpr auto HORIZONTAL_T_STEP = 1.0F / static_cast<float>(NUM_VERTICAL_COLORS);
  const auto colorMapsGrid                = GetColorMapsGrid(verticalT, colorMixingTFunc);

  REQUIRE(VERTICAL_BASE_COLORS.size() == NUM_VERTICAL_COLORS);

  verticalT.Reset();
  for (auto j = 0U; j < NUM_VERTICAL_STEPS; ++j)
  {
    for (auto i = 0U; i < NUM_VERTICAL_COLORS; ++i)
    {
      const auto gridColors  = colorMapsGrid.GetCurrentHorizontalLineColors();
      const auto tHorizontal = static_cast<float>(i) * HORIZONTAL_T_STEP;

      REQUIRE(gridColors.size() == NUM_VERTICAL_COLORS);

      TestColorElement(gridColors.at(i), tHorizontal, VERTICAL_BASE_COLORS.at(i), verticalT());
    }
    verticalT.Increment();
  }
}

// NOLINTEND(bugprone-chained-comparison)

} // namespace GOOM::UNIT_TESTS
