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

#include "control/goom_sound_events.h"
#include "filter_fx/filter_buffer_striper.h"
#include "filter_fx/filter_buffers.h"
#include "filter_fx/filter_zoom_vector.h"
#include "filter_fx/normalized_coords.h"
#include "goom_config.h"
#include "goom_plugin_info.h"
#include "point2d.h"
#include "sound_info.h"
#include "utils/math/goom_rand.h"
#include "utils/math/misc.h"
#include "utils/parallel_utils.h"

#include <catch2/catch_test_macros.hpp>
#include <cmath>

namespace GOOM::UNIT_TESTS
{

using CONTROL::GoomSoundEvents;
using FILTER_FX::FilterZoomVector;
using FILTER_FX::NormalizedCoords;
using FILTER_FX::NormalizedCoordsConverter;
using FILTER_FX::ZoomFilterBuffers;
using FILTER_FX::ZoomFilterBufferStriper;
using FILTER_FX::ZoomFilterEffectsSettings;
using FILTER_FX::FILTER_BUFFERS::MIN_SCREEN_COORD_ABS_VAL;
using FILTER_FX::FILTER_UTILS::ZoomCoordTransforms;
using FILTER_FX::FILTER_UTILS::ZoomTransformBuffers;
using UTILS::Parallel;
using UTILS::MATH::GoomRand;

using FilterBuffers = ZoomFilterBuffers<ZoomFilterBufferStriper>;

namespace
{
//#define LARGE_SCREEN_TEST
constexpr auto LARGE_WIDTH  = 3840U;
constexpr auto LARGE_HEIGHT = 2160U;
#ifdef LARGE_SCREEN_TEST
static constexpr auto WIDTH  = LARGE_WIDTH;
static constexpr auto HEIGHT = LARGE_HEIGHT;
#else
constexpr auto WIDTH  = LARGE_WIDTH / 10U;
constexpr auto HEIGHT = LARGE_HEIGHT / 10U;
#endif
constexpr auto* RESOURCES_DIRECTORY = "";
const auto GOOM_RAND                = GoomRand{};

const auto SOUND_INFO   = SoundInfo{};
const auto SOUND_EVENTS = GoomSoundEvents{SOUND_INFO};
const auto GOOM_INFO    = PluginInfo{
       {WIDTH, HEIGHT},
       SOUND_EVENTS
};
constexpr auto NORMALIZED_COORDS_CONVERTER = NormalizedCoordsConverter{
    {WIDTH, HEIGHT},
    MIN_SCREEN_COORD_ABS_VAL
};

constexpr auto MID_PT                     = MidpointFromOrigin({WIDTH, HEIGHT});
constexpr auto CONST_ZOOM_VECTOR_COORDS_1 = Point2dInt{16, 40};
constexpr auto CONST_ZOOM_VECTOR_COORDS_2 = Point2dInt{32, 52};
constexpr auto DUMMY_COORDS               = Point2dInt{14, 38};

const auto MAX_TRAN_POINT = ZoomCoordTransforms::ScreenToTranPoint({WIDTH - 1, HEIGHT - 1});
const auto MID_TRAN_POINT = ZoomCoordTransforms::ScreenToTranPoint(MID_PT);

inline auto GetBuffPos(const int32_t x, const int32_t y) -> size_t
{
  return (static_cast<size_t>(y) * WIDTH) + static_cast<size_t>(x);
}

class TestZoomVector : public FilterZoomVector
{
public:
  explicit TestZoomVector(const bool returnConst) noexcept
    : FilterZoomVector{WIDTH, RESOURCES_DIRECTORY, GOOM_RAND, NORMALIZED_COORDS_CONVERTER},
      m_returnConst{returnConst}
  {
  }

  auto SetFilterSettings(const ZoomFilterEffectsSettings& filterEffectsSettings) noexcept
      -> void override;

  [[nodiscard]] auto GetConstCoords() const noexcept -> const Point2dInt& { return m_constCoords; }
  auto SetConstCoords(const Point2dInt& coords) -> void { m_constCoords = coords; }

  auto SetZoomInCoeff(const float val) noexcept -> void { m_zoomInCoeff = val; }

  [[nodiscard]] auto GetZoomInPoint(const NormalizedCoords& coords,
                                    const NormalizedCoords& filterViewportCoords) const noexcept
      -> NormalizedCoords override;

private:
  bool m_returnConst;
  Point2dInt m_constCoords = CONST_ZOOM_VECTOR_COORDS_1;
  float m_zoomInCoeff      = 0.0F;
};

auto TestZoomVector::SetFilterSettings(
    const ZoomFilterEffectsSettings& filterEffectsSettings) noexcept -> void
{
  FilterZoomVector::SetFilterSettings(filterEffectsSettings);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto TestZoomVector::GetZoomInPoint(const NormalizedCoords& coords,
                                    [[maybe_unused]] const NormalizedCoords& filterViewportCoords)
    const noexcept -> NormalizedCoords
{
  if (m_returnConst)
  {
    return NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(m_constCoords);
  }
  return (1.0F - m_zoomInCoeff) * coords;
}

const ZoomCoordTransforms COORD_TRANSFORMS{
    {WIDTH, HEIGHT}
};
const auto IDENTITY_ZOOM_VECTOR = TestZoomVector{false};
const auto CONSTANT_ZOOM_VECTOR = TestZoomVector{true};

auto GetSourcePoint(const FilterBuffers& filterBuffers, const size_t buffPos) -> Point2dInt
{
  return filterBuffers.GetSourcePointInfo(buffPos).screenPoint;
}

auto FullyUpdateDestBuffer(FilterBuffers& filterBuffers) noexcept -> void
{
  REQUIRE(FilterBuffers::TranBuffersState::START_FRESH_TRAN_BUFFERS ==
          filterBuffers.GetTranBuffersState());

  filterBuffers.UpdateTranBuffers();
  REQUIRE(FilterBuffers::TranBuffersState::TRAN_BUFFERS_READY ==
          filterBuffers.GetTranBuffersState());

  filterBuffers.UpdateTranBuffers();
  while (true)
  {
    filterBuffers.UpdateTranBuffers();
    if (0 == filterBuffers.GetTranBuffYLineStart())
    {
      break;
    }
  }
  REQUIRE(FilterBuffers::TranBuffersState::RESET_TRAN_BUFFERS ==
          filterBuffers.GetTranBuffersState());

  filterBuffers.UpdateTranBuffers();
}

[[nodiscard]] auto GetFilterBuffers(Parallel& parallel, const TestZoomVector& zoomVector) noexcept
    -> FilterBuffers
{
  auto filterBufferStriper = std::make_unique<ZoomFilterBufferStriper>(
      parallel,
      GOOM_INFO,
      NORMALIZED_COORDS_CONVERTER,
      [&zoomVector](const NormalizedCoords& normalizedCoords,
                    const NormalizedCoords& viewportCoords)
      { return zoomVector.GetZoomInPoint(normalizedCoords, viewportCoords); });

  return FilterBuffers{GOOM_INFO, std::move(filterBufferStriper)};
}

constexpr auto TEST_X          = 10;
constexpr auto TEST_Y          = 50;
constexpr auto TEST_SRCE_POINT = Point2dInt{TEST_X, TEST_Y};
static_assert((0 <= TEST_X) && (TEST_X < WIDTH), "Invalid X");
static_assert((0 <= TEST_Y) && (TEST_Y < WIDTH), "Invalid Y");

constexpr auto DUMMY_NML_COORDS = NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(DUMMY_COORDS);

} // namespace

// NOLINTBEGIN(readability-function-cognitive-complexity)
TEST_CASE("ZoomFilterBuffers Basic")
{
  auto parallel      = Parallel{-1};
  auto filterBuffers = GetFilterBuffers(parallel, IDENTITY_ZOOM_VECTOR);

  filterBuffers.SetBuffMidpoint(MID_PT);
  filterBuffers.Start();

  SECTION("Correct Starting TranBuffersState")
  {
    REQUIRE(FilterBuffers::TranBuffersState::START_FRESH_TRAN_BUFFERS ==
            filterBuffers.GetTranBuffersState());
  }
  SECTION("Correct Starting BuffYLineStart")
  {
    REQUIRE(0 == filterBuffers.GetTranBuffYLineStart());
  }
  SECTION("Correct Starting BuffMidpoint()")
  {
    REQUIRE(MID_PT == filterBuffers.GetBuffMidpoint());
  }
  SECTION("Correct Starting HaveFilterSettingsChanged")
  {
    REQUIRE(!filterBuffers.HaveFilterSettingsChanged());
  }
  SECTION("Correct Starting TranLerpFactor")
  {
    REQUIRE(0 == filterBuffers.GetTranLerpFactor());
  }
}

TEST_CASE("ZoomFilterBuffers - Correct Starting ZoomBufferTranPoint")
{
  auto parallel      = Parallel{-1};
  auto filterBuffers = GetFilterBuffers(parallel, IDENTITY_ZOOM_VECTOR);
  filterBuffers.SetBuffMidpoint(MID_PT);
  filterBuffers.Start();

  REQUIRE(DUMMY_NML_COORDS.Equals(
      IDENTITY_ZOOM_VECTOR.GetZoomInPoint(DUMMY_NML_COORDS, DUMMY_NML_COORDS)));

  // TranLerpFactor is zero so starting tranPoint should be srce identity buffer
  static constexpr auto EXPECTED_SRCE_IDENTITY_POINT = TEST_SRCE_POINT;
  const auto srcePoint = GetSourcePoint(filterBuffers, GetBuffPos(TEST_X, TEST_Y));
  UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
  UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);
  REQUIRE(EXPECTED_SRCE_IDENTITY_POINT == srcePoint);
}

TEST_CASE("ZoomFilterBuffers - Correct Dest ZoomBufferTranPoint")
{
  auto parallel      = Parallel{-1};
  auto filterBuffers = GetFilterBuffers(parallel, IDENTITY_ZOOM_VECTOR);
  filterBuffers.SetBuffMidpoint(MID_PT);
  filterBuffers.Start();

  // Lerp to the dest buffer only (by using max lerp).
  filterBuffers.SetTranLerpFactor(FilterBuffers::GetMaxTranLerpFactor());
  REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());

  REQUIRE(DUMMY_NML_COORDS.Equals(
      IDENTITY_ZOOM_VECTOR.GetZoomInPoint(DUMMY_NML_COORDS, DUMMY_NML_COORDS)));

  // GetSourcePoint uses tranPoint which comes solely from the dest Zoom buffer.
  // Because we are using an identity ZoomVectorFunc, tranPoint should be the identity point.
  static constexpr auto EXPECTED_SRCE_IDENTITY_POINT = TEST_SRCE_POINT;
  const auto srcePoint = GetSourcePoint(filterBuffers, GetBuffPos(TEST_X, TEST_Y));
  UNSCOPED_INFO("expectedSrceIdentityPoint.x = " << EXPECTED_SRCE_IDENTITY_POINT.x);
  UNSCOPED_INFO("expectedSrceIdentityPoint.y = " << EXPECTED_SRCE_IDENTITY_POINT.y);
  UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
  UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);
  REQUIRE(EXPECTED_SRCE_IDENTITY_POINT == srcePoint);
}

TEST_CASE("ZoomFilterBuffers - Correct Lerped ZoomBufferTranPoint")
{
  auto parallel      = Parallel{-1};
  auto filterBuffers = GetFilterBuffers(parallel, IDENTITY_ZOOM_VECTOR);
  filterBuffers.SetBuffMidpoint(MID_PT);
  filterBuffers.Start();

  static constexpr auto T_LERP = 0.5F;
  const auto tranLerpFactor    = static_cast<uint32_t>(
      std::round(T_LERP * static_cast<float>(FilterBuffers::GetMaxTranLerpFactor())));

  filterBuffers.SetTranLerpFactor(tranLerpFactor);
  REQUIRE(tranLerpFactor == filterBuffers.GetTranLerpFactor());

  REQUIRE(DUMMY_NML_COORDS.Equals(
      IDENTITY_ZOOM_VECTOR.GetZoomInPoint(DUMMY_NML_COORDS, DUMMY_NML_COORDS)));

  // tranPoint comes solely from the lerp middle of srce and dest Zoom buffer which
  // because we are using an identity ZoomVectorFunc should still be the identity point.
  static constexpr auto EXPECTED_SRCE_IDENTITY_POINT = TEST_SRCE_POINT;
  const auto srcePoint = GetSourcePoint(filterBuffers, GetBuffPos(TEST_X, TEST_Y));
  UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
  UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);
  REQUIRE(EXPECTED_SRCE_IDENTITY_POINT == srcePoint);
}

TEST_CASE("ZoomFilterBuffers Calculations - Correct Dest ZoomBufferTranPoint")
{
  auto parallel      = Parallel{-1};
  auto filterBuffers = GetFilterBuffers(parallel, CONSTANT_ZOOM_VECTOR);
  filterBuffers.SetBuffMidpoint(MID_PT);
  filterBuffers.Start();

  static constexpr auto NML_CONST_ZOOM_VECTOR_COORDS_1 =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1);

  REQUIRE(CONST_ZOOM_VECTOR_COORDS_1 == CONSTANT_ZOOM_VECTOR.GetConstCoords());
  REQUIRE(MID_PT == filterBuffers.GetBuffMidpoint());

  // Lerp to the dest buffer only
  filterBuffers.SetTranLerpFactor(FilterBuffers::GetMaxTranLerpFactor());
  REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());

  // tranPoint comes solely from the dest Zoom buffer which because we are using a
  // const ZoomVectorFunc, returns a const normalized value
  UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_1.x = " << NML_CONST_ZOOM_VECTOR_COORDS_1.GetX());
  UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_1.y = " << NML_CONST_ZOOM_VECTOR_COORDS_1.GetY());
  UNSCOPED_INFO("GetZoomInPoint(DUMMY_NML_COORDS).x = "
                << CONSTANT_ZOOM_VECTOR.GetZoomInPoint(DUMMY_NML_COORDS, DUMMY_NML_COORDS).GetX());
  UNSCOPED_INFO("GetZoomInPoint(DUMMY_NML_COORDS).y = "
                << CONSTANT_ZOOM_VECTOR.GetZoomInPoint(DUMMY_NML_COORDS, DUMMY_NML_COORDS).GetY());
  REQUIRE(NML_CONST_ZOOM_VECTOR_COORDS_1.Equals(
      CONSTANT_ZOOM_VECTOR.GetZoomInPoint(DUMMY_NML_COORDS, DUMMY_NML_COORDS)));
  const auto normalizedMidPt =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(filterBuffers.GetBuffMidpoint());
  const auto expectedNmlCoord1 = NormalizedCoords{
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1)};
  const auto expectedUnclippedTranPoint =
      COORD_TRANSFORMS.NormalizedToTranPoint(normalizedMidPt + expectedNmlCoord1);
  const auto expectedTranPoint =
      Point2dInt{std::clamp(expectedUnclippedTranPoint.x, 0, MAX_TRAN_POINT.x),
                 std::clamp(expectedUnclippedTranPoint.y, 0, MAX_TRAN_POINT.y)};
  UNSCOPED_INFO("filterBuffers.GetBuffMidpoint().x = " << filterBuffers.GetBuffMidpoint().x);
  UNSCOPED_INFO("filterBuffers.GetBuffMidpoint().y = " << filterBuffers.GetBuffMidpoint().y);
  UNSCOPED_INFO("normalizedMidPt.x = " << normalizedMidPt.GetX());
  UNSCOPED_INFO("normalizedMidPt.y = " << normalizedMidPt.GetY());
  UNSCOPED_INFO("expectedNmlCoord1.x = " << expectedNmlCoord1.GetX());
  UNSCOPED_INFO("expectedNmlCoord1.y = " << expectedNmlCoord1.GetY());
  UNSCOPED_INFO("expectedUnclippedTranPoint.x = " << expectedUnclippedTranPoint.x);
  UNSCOPED_INFO("expectedUnclippedTranPoint.y = " << expectedUnclippedTranPoint.y);
  UNSCOPED_INFO("expectedTranPoint.x = " << expectedTranPoint.x);
  UNSCOPED_INFO("expectedTranPoint.y = " << expectedTranPoint.y);

  const auto expectedSrcePoint = ZoomCoordTransforms::TranToScreenPoint(expectedTranPoint);
  UNSCOPED_INFO("expectedSrcePoint.x = " << expectedSrcePoint.x);
  UNSCOPED_INFO("expectedSrcePoint.y = " << expectedSrcePoint.y);

  for (auto buffPos = 0U; buffPos < (WIDTH * HEIGHT); ++buffPos)
  {
    const auto srcePoint = GetSourcePoint(filterBuffers, buffPos);
    UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
    UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);

    REQUIRE(expectedSrcePoint == srcePoint);
  }
}

namespace
{

auto TestCorrectSrceDestPoint(FilterBuffers& filterBuffers, const int32_t x, const int32_t y)
    -> void
{
  static constexpr auto NML_CONST_ZOOM_VECTOR_COORDS_1 =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1);

  REQUIRE(NML_CONST_ZOOM_VECTOR_COORDS_1.Equals(
      CONSTANT_ZOOM_VECTOR.GetZoomInPoint(DUMMY_NML_COORDS, DUMMY_NML_COORDS)));

  const auto codeSrceTranPoint = filterBuffers.GetTransformBuffers().GetTranSrce(GetBuffPos(x, y));
  UNSCOPED_INFO("codeSrceTranPoint.x = " << codeSrceTranPoint.x);
  UNSCOPED_INFO("codeSrceTranPoint.y = " << codeSrceTranPoint.y);
  const auto expectedSrceTranPoint = COORD_TRANSFORMS.ScreenToTranPoint({x, y});
  UNSCOPED_INFO("expectedSrceTranPoint.x = " << expectedSrceTranPoint.x);
  UNSCOPED_INFO("expectedSrceTranPoint.y = " << expectedSrceTranPoint.y);
  REQUIRE(codeSrceTranPoint == expectedSrceTranPoint);

  const auto codeDestTranPoint = filterBuffers.GetTransformBuffers().GetTranDest(GetBuffPos(x, y));
  UNSCOPED_INFO("codeDestTranPoint.x = " << codeDestTranPoint.x);
  UNSCOPED_INFO("codeDestTranPoint.y = " << codeDestTranPoint.y);
  const auto normalizedMidPt =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(filterBuffers.GetBuffMidpoint());
  UNSCOPED_INFO("filterBuffers.GetBuffMidpoint().x = " << filterBuffers.GetBuffMidpoint().x);
  UNSCOPED_INFO("filterBuffers.GetBuffMidpoint().y = " << filterBuffers.GetBuffMidpoint().y);
  UNSCOPED_INFO("normalizedMidPt.x = " << normalizedMidPt.GetX());
  UNSCOPED_INFO("normalizedMidPt.y = " << normalizedMidPt.GetY());
  const auto expectedDestTranPoint = COORD_TRANSFORMS.NormalizedToTranPoint(
      normalizedMidPt + NormalizedCoords{NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(
                            CONST_ZOOM_VECTOR_COORDS_1)});
  UNSCOPED_INFO("expectedDestTranPoint.x = " << expectedDestTranPoint.x);
  UNSCOPED_INFO("expectedDestTranPoint.y = " << expectedDestTranPoint.y);
  REQUIRE(codeDestTranPoint == expectedDestTranPoint);
}

} // namespace

TEST_CASE("ZoomFilterBuffers - Correct Srce/Dest Points")
{
  auto parallel      = Parallel{-1};
  auto filterBuffers = GetFilterBuffers(parallel, CONSTANT_ZOOM_VECTOR);
  filterBuffers.SetBuffMidpoint(MID_PT);
  filterBuffers.Start();

  REQUIRE(CONST_ZOOM_VECTOR_COORDS_1 == CONSTANT_ZOOM_VECTOR.GetConstCoords());
  REQUIRE(MID_PT == filterBuffers.GetBuffMidpoint());

  for (auto y = 0; y < static_cast<int32_t>(HEIGHT); ++y)
  {
    for (auto x = 0; x < static_cast<int32_t>(WIDTH); ++x)
    {
      TestCorrectSrceDestPoint(filterBuffers, x, y);
    }
  }
}

namespace
{

constexpr auto CORRECT_SRCE_DEST_TRAN_POINT_T_LERP = 0.5F;

auto TestCorrectSrceDestTranPoint(const uint32_t tranLerpFactor,
                                  FilterBuffers& filterBuffers,
                                  const int32_t x,
                                  const int32_t y) -> void
{
  // tranPoint comes from halfway between srce and dest Zoom buffer.
  static constexpr auto NML_CONST_ZOOM_VECTOR_COORDS_1 =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1);
  REQUIRE(NML_CONST_ZOOM_VECTOR_COORDS_1.Equals(
      CONSTANT_ZOOM_VECTOR.GetZoomInPoint(DUMMY_NML_COORDS, DUMMY_NML_COORDS)));

  const auto codeSrceTranPoint = filterBuffers.GetTransformBuffers().GetTranSrce(GetBuffPos(x, y));
  UNSCOPED_INFO("codeSrceTranPoint.x = " << codeSrceTranPoint.x);
  UNSCOPED_INFO("codeSrceTranPoint.y = " << codeSrceTranPoint.y);
  const auto expectedSrceTranPoint = COORD_TRANSFORMS.ScreenToTranPoint({x, y});
  UNSCOPED_INFO("expectedSrceTranPoint.x = " << expectedSrceTranPoint.x);
  UNSCOPED_INFO("expectedSrceTranPoint.y = " << expectedSrceTranPoint.y);

  const auto normalizedMidPt =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(filterBuffers.GetBuffMidpoint());
  UNSCOPED_INFO("filterBuffers.GetBuffMidpoint().x = " << filterBuffers.GetBuffMidpoint().x);
  UNSCOPED_INFO("filterBuffers.GetBuffMidpoint().y = " << filterBuffers.GetBuffMidpoint().y);
  UNSCOPED_INFO("normalizedMidPt.x = " << normalizedMidPt.GetX());
  UNSCOPED_INFO("normalizedMidPt.y = " << normalizedMidPt.GetY());
  const auto codeDestTranPoint = filterBuffers.GetTransformBuffers().GetTranDest(GetBuffPos(x, y));
  UNSCOPED_INFO("codeDestTranPoint.x = " << codeDestTranPoint.x);
  UNSCOPED_INFO("codeDestTranPoint.y = " << codeDestTranPoint.y);
  const auto expectedDestTranPoint = COORD_TRANSFORMS.NormalizedToTranPoint(
      normalizedMidPt + NormalizedCoords{NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(
                            CONST_ZOOM_VECTOR_COORDS_1)});
  UNSCOPED_INFO("expectedDestTranPoint.x = " << expectedDestTranPoint.x);
  UNSCOPED_INFO("expectedDestTranPoint.y = " << expectedDestTranPoint.y);

  auto isClipped = false;
  const auto codeClippedTranPoint =
      filterBuffers.GetZoomBufferTranPoint(GetBuffPos(x, y), isClipped);
  UNSCOPED_INFO("codeClippedTranPoint.x = " << codeClippedTranPoint.x);
  UNSCOPED_INFO("codeClippedTranPoint.y = " << codeClippedTranPoint.y);
  const auto calculatedUnclippedTranPoint = ZoomTransformBuffers::GetTranBuffLerpPoint(
      expectedSrceTranPoint, expectedDestTranPoint, tranLerpFactor);
  UNSCOPED_INFO("calculatedUnclippedTranPoint.x = " << calculatedUnclippedTranPoint.x);
  UNSCOPED_INFO("calculatedUnclippedTranPoint.y = " << calculatedUnclippedTranPoint.y);
  const auto expectedUnclippedTranPoint = Point2dInt{
      STD20::lerp(
          expectedSrceTranPoint.x, expectedDestTranPoint.x, CORRECT_SRCE_DEST_TRAN_POINT_T_LERP),
      STD20::lerp(
          expectedSrceTranPoint.y, expectedDestTranPoint.y, CORRECT_SRCE_DEST_TRAN_POINT_T_LERP)};
  UNSCOPED_INFO("expectedUnclippedTranPoint.x = " << expectedUnclippedTranPoint.x);
  UNSCOPED_INFO("expectedUnclippedTranPoint.y = " << expectedUnclippedTranPoint.y);

  const auto expectedTranPoint =
      Point2dInt{std::clamp(expectedUnclippedTranPoint.x, 0, MAX_TRAN_POINT.x),
                 std::clamp(expectedUnclippedTranPoint.y, 0, MAX_TRAN_POINT.y)};
  UNSCOPED_INFO("expectedTranPoint.x = " << expectedTranPoint.x);
  UNSCOPED_INFO("expectedTranPoint.y = " << expectedTranPoint.y);

  const auto expectedScreenPoint = ZoomCoordTransforms::TranToScreenPoint(expectedTranPoint);
  UNSCOPED_INFO("expectedScreenPoint.x = " << expectedScreenPoint.x);
  UNSCOPED_INFO("expectedScreenPoint.y = " << expectedScreenPoint.y);

  const auto screenPoint = GetSourcePoint(filterBuffers, GetBuffPos(x, y));
  UNSCOPED_INFO("srcePoint.x = " << screenPoint.x);
  UNSCOPED_INFO("srcePoint.y = " << screenPoint.y);

  REQUIRE(expectedScreenPoint == screenPoint);
}

} // namespace

TEST_CASE("ZoomFilterBuffers - Correct Srce/Dest ZoomBufferTranPoint")
{
  auto parallel      = Parallel{-1};
  auto filterBuffers = GetFilterBuffers(parallel, CONSTANT_ZOOM_VECTOR);
  filterBuffers.SetBuffMidpoint(MID_PT);
  filterBuffers.Start();

  REQUIRE(CONST_ZOOM_VECTOR_COORDS_1 == CONSTANT_ZOOM_VECTOR.GetConstCoords());
  REQUIRE(MID_PT == filterBuffers.GetBuffMidpoint());

  const auto tranLerpFactor =
      static_cast<uint32_t>(std::round(CORRECT_SRCE_DEST_TRAN_POINT_T_LERP *
                                       static_cast<float>(FilterBuffers::GetMaxTranLerpFactor())));

  filterBuffers.SetTranLerpFactor(tranLerpFactor);
  REQUIRE(tranLerpFactor == filterBuffers.GetTranLerpFactor());

  for (auto y = 0; y < static_cast<int32_t>(HEIGHT); ++y)
  {
    for (auto x = 0; x < static_cast<int32_t>(WIDTH); ++x)
    {
      TestCorrectSrceDestTranPoint(tranLerpFactor, filterBuffers, x, y);
    }
  }
}

namespace
{

auto TestCorrectStripesBasicValues(const FilterBuffers& filterBuffers) -> void
{
  static constexpr auto NML_CONST_ZOOM_VECTOR_COORDS1 =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1);

  REQUIRE(CONST_ZOOM_VECTOR_COORDS_1 == CONSTANT_ZOOM_VECTOR.GetConstCoords());
  REQUIRE(MID_PT == filterBuffers.GetBuffMidpoint());
  REQUIRE(NML_CONST_ZOOM_VECTOR_COORDS1.Equals(
      CONSTANT_ZOOM_VECTOR.GetZoomInPoint(DUMMY_NML_COORDS, DUMMY_NML_COORDS)));

  REQUIRE(FilterBuffers::TranBuffersState::START_FRESH_TRAN_BUFFERS ==
          filterBuffers.GetTranBuffersState());
  REQUIRE(not filterBuffers.HaveFilterSettingsChanged());
}

auto TestCorrectStripesFullyUpdate(FilterBuffers& filterBuffers,
                                   const TestZoomVector& constantZoomVector) -> void
{
  // Make sure dest buffer is completely copied to srce buffer at end of update.
  filterBuffers.NotifyFilterSettingsHaveChanged();
  REQUIRE(filterBuffers.HaveFilterSettingsChanged());

  FullyUpdateDestBuffer(filterBuffers);
  REQUIRE(FilterBuffers::TranBuffersState::START_FRESH_TRAN_BUFFERS ==
          filterBuffers.GetTranBuffersState());
  REQUIRE(0 == filterBuffers.GetTranLerpFactor());
  REQUIRE(0 == filterBuffers.GetTranBuffYLineStart());
  REQUIRE(CONST_ZOOM_VECTOR_COORDS_2 == constantZoomVector.GetConstCoords());
  REQUIRE(MID_PT == filterBuffers.GetBuffMidpoint());
}

[[nodiscard]] auto TestCorrectStripesGetExpectedSrcePoint(const FilterBuffers& filterBuffers)
    -> Point2dInt
{
  // Get srce buffer points - should be all CONST_ZOOM_VECTOR_COORDS_1

  const auto normalizedMidPt =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(filterBuffers.GetBuffMidpoint());

  const auto expectedSrceTranPoint = COORD_TRANSFORMS.NormalizedToTranPoint(
      normalizedMidPt +
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1));
  const auto expectedUnclippedSrcePoint =
      ZoomCoordTransforms::TranToScreenPoint(expectedSrceTranPoint);
  const auto expectedSrcePoint =
      Point2dInt{std::clamp(expectedUnclippedSrcePoint.x, 0, static_cast<int32_t>(WIDTH - 1U)),
                 std::clamp(expectedUnclippedSrcePoint.y, 0, static_cast<int32_t>(HEIGHT - 1U))};
  UNSCOPED_INFO("expectedSrceTranPoint.x = " << expectedSrceTranPoint.x);
  UNSCOPED_INFO("expectedSrceTranPoint.y = " << expectedSrceTranPoint.y);
  UNSCOPED_INFO("expectedUnclippedSrcePoint.x = " << expectedSrcePoint.x);
  UNSCOPED_INFO("expectedUnclippedSrcePoint.y = " << expectedSrcePoint.y);
  UNSCOPED_INFO("expectedSrcePoint.x = " << expectedSrcePoint.x);
  UNSCOPED_INFO("expectedSrcePoint.y = " << expectedSrcePoint.y);

  return expectedSrcePoint;
}

[[nodiscard]] auto TestCorrectStripesGetExpectedDestPoint(const FilterBuffers& filterBuffers,
                                                          const TestZoomVector& constantZoomVector)
    -> Point2dInt
{
  // Get dest buffer points - should be all CONST_ZOOM_VECTOR_COORDS_2

  const auto normalizedMidPt =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(filterBuffers.GetBuffMidpoint());

  static constexpr auto NML_CONST_ZOOM_VECTOR_COORDS2 =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_2);
  REQUIRE(NML_CONST_ZOOM_VECTOR_COORDS2.Equals(
      constantZoomVector.GetZoomInPoint(DUMMY_NML_COORDS, DUMMY_NML_COORDS)));

  const auto expectedDestTranPoint =
      COORD_TRANSFORMS.NormalizedToTranPoint(normalizedMidPt + NML_CONST_ZOOM_VECTOR_COORDS2);
  const auto expectedUnclippedDestPoint =
      ZoomCoordTransforms::TranToScreenPoint(expectedDestTranPoint);
  const auto expectedDestPoint =
      Point2dInt{std::clamp(expectedUnclippedDestPoint.x, 0, static_cast<int32_t>(WIDTH - 1U)),
                 std::clamp(expectedUnclippedDestPoint.y, 0, static_cast<int32_t>(HEIGHT - 1U))};
  UNSCOPED_INFO("normalizedMidPt.x = " << normalizedMidPt.GetX());
  UNSCOPED_INFO("normalizedMidPt.y = " << normalizedMidPt.GetY());
  UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_2.x = " << NML_CONST_ZOOM_VECTOR_COORDS2.GetX());
  UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_2.y = " << NML_CONST_ZOOM_VECTOR_COORDS2.GetY());
  UNSCOPED_INFO("expectedDestTranPoint.x = " << expectedDestTranPoint.x);
  UNSCOPED_INFO("expectedDestTranPoint.y = " << expectedDestTranPoint.y);
  UNSCOPED_INFO("expectedUnclippedDestPoint.x = " << expectedUnclippedDestPoint.x);
  UNSCOPED_INFO("expectedUnclippedDestPoint.y = " << expectedUnclippedDestPoint.y);
  UNSCOPED_INFO("expectedDestPoint.x = " << expectedDestPoint.x);
  UNSCOPED_INFO("expectedDestPoint.y = " << expectedDestPoint.y);

  return expectedDestPoint;
}

auto TestCorrectStripesSrcePoint(FilterBuffers& filterBuffers,
                                 const uint32_t buffPos,
                                 const Point2dInt& expectedSrcePoint) -> void
{
  const auto srcePoint = GetSourcePoint(filterBuffers, buffPos);
  UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
  UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);

  REQUIRE(expectedSrcePoint == srcePoint);
}

auto TestCorrectStripesDestPoint(FilterBuffers& filterBuffers,
                                 const uint32_t buffPos,
                                 const Point2dInt& expectedDestPoint) -> void
{
  const auto destPoint = GetSourcePoint(filterBuffers, buffPos);
  UNSCOPED_INFO("destPoint.x = " << destPoint.x);
  UNSCOPED_INFO("destPoint.y = " << destPoint.y);

  REQUIRE(expectedDestPoint == destPoint);
}

} // namespace

TEST_CASE("ZoomFilterBuffers Stripes")
{
  auto constantZoomVector = TestZoomVector{true};

  auto parallel      = Parallel{-1};
  auto filterBuffers = GetFilterBuffers(parallel, constantZoomVector);
  filterBuffers.SetBuffMidpoint(MID_PT);
  filterBuffers.Start();

  TestCorrectStripesBasicValues(filterBuffers);

  filterBuffers.SetTranLerpFactor(FilterBuffers::GetMaxTranLerpFactor());
  REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());
  constantZoomVector.SetConstCoords(CONST_ZOOM_VECTOR_COORDS_2);
  REQUIRE(CONST_ZOOM_VECTOR_COORDS_2 == constantZoomVector.GetConstCoords());
  TestCorrectStripesFullyUpdate(filterBuffers, constantZoomVector);

  filterBuffers.SetTranLerpFactor(0);
  REQUIRE(0 == filterBuffers.GetTranLerpFactor());
  const auto expectedSrcePoint = TestCorrectStripesGetExpectedSrcePoint(filterBuffers);
  for (auto buffPos = 0U; buffPos < (WIDTH * HEIGHT); ++buffPos)
  {
    TestCorrectStripesSrcePoint(filterBuffers, buffPos, expectedSrcePoint);
  }

  filterBuffers.SetTranLerpFactor(FilterBuffers::GetMaxTranLerpFactor());
  REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());
  const auto expectedDestPoint =
      TestCorrectStripesGetExpectedDestPoint(filterBuffers, constantZoomVector);
  for (auto buffPos = 0U; buffPos < (WIDTH * HEIGHT); ++buffPos)
  {
    TestCorrectStripesDestPoint(filterBuffers, buffPos, expectedDestPoint);
  }
}

TEST_CASE("ZoomFilterBuffers ZoomIn")
{
  static constexpr auto TEST_SRCE_NML_COORDS =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(TEST_SRCE_POINT);

  auto zoomVector = TestZoomVector{false};

  auto parallel      = Parallel{-1};
  auto filterBuffers = GetFilterBuffers(parallel, zoomVector);

  filterBuffers.SetBuffMidpoint(MID_PT);
  filterBuffers.Start();

  SECTION("Correct Zoomed In Dest ZoomBufferTranPoint")
  {
    // Lerp to the dest buffer only (by using max lerp).
    filterBuffers.SetTranLerpFactor(FilterBuffers::GetMaxTranLerpFactor());
    REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());

    REQUIRE(TEST_SRCE_NML_COORDS.Equals(
        zoomVector.GetZoomInPoint(TEST_SRCE_NML_COORDS, TEST_SRCE_NML_COORDS)));

    static constexpr auto ZOOM_IN_COEFF1  = 0.2F;
    static constexpr auto ZOOM_IN_FACTOR1 = 1.0F - ZOOM_IN_COEFF1;
    zoomVector.SetZoomInCoeff(ZOOM_IN_COEFF1);
    REQUIRE((ZOOM_IN_FACTOR1 * TEST_SRCE_NML_COORDS)
                .Equals(zoomVector.GetZoomInPoint(TEST_SRCE_NML_COORDS, TEST_SRCE_NML_COORDS)));

    // GetSourcePoint uses tranPoint which comes solely from the dest Zoom buffer.
    // Because we are using a zoomed in ZoomVectorFunc, tranPoint should be zoomed in.
    filterBuffers.NotifyFilterSettingsHaveChanged();
    REQUIRE(filterBuffers.HaveFilterSettingsChanged());
    FullyUpdateDestBuffer(filterBuffers);
    REQUIRE(FilterBuffers::TranBuffersState::START_FRESH_TRAN_BUFFERS ==
            filterBuffers.GetTranBuffersState());
    filterBuffers.SetTranLerpFactor(FilterBuffers::GetMaxTranLerpFactor());
    REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());

    const auto expectedTranPoint         = ZoomCoordTransforms::ScreenToTranPoint(TEST_SRCE_POINT);
    const auto expectedZoomedInTranPoint = Point2dInt{
        static_cast<int32_t>(ZOOM_IN_FACTOR1 *
                             static_cast<float>(expectedTranPoint.x - MID_TRAN_POINT.x)) +
            MID_TRAN_POINT.x,
        static_cast<int32_t>(ZOOM_IN_FACTOR1 *
                             static_cast<float>(expectedTranPoint.y - MID_TRAN_POINT.y)) +
            MID_TRAN_POINT.y};
    const auto expectedZoomedInSrcePoint =
        ZoomCoordTransforms::TranToScreenPoint(expectedZoomedInTranPoint);
    const auto srcePoint = GetSourcePoint(filterBuffers, GetBuffPos(TEST_X, TEST_Y));
    UNSCOPED_INFO("expectedTranPoint.x = " << expectedTranPoint.x);
    UNSCOPED_INFO("expectedTranPoint.y = " << expectedTranPoint.y);
    UNSCOPED_INFO("expectedZoomedInTranPoint.x = " << expectedZoomedInTranPoint.x);
    UNSCOPED_INFO("expectedZoomedInTranPoint.y = " << expectedZoomedInTranPoint.y);
    UNSCOPED_INFO("expectedZoomedInSrcePoint.x = " << expectedZoomedInSrcePoint.x);
    UNSCOPED_INFO("expectedZoomedInSrcePoint.y = " << expectedZoomedInSrcePoint.y);
    UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
    UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);
    REQUIRE(expectedZoomedInSrcePoint == srcePoint);
  }
}

TEST_CASE("ZoomFilterBuffers Clipping")
{
  auto parallel      = Parallel{-1};
  auto filterBuffers = GetFilterBuffers(parallel, CONSTANT_ZOOM_VECTOR);
  filterBuffers.SetBuffMidpoint({0, 0});
  filterBuffers.Start();

  static constexpr auto NML_CONST_ZOOM_VECTOR_COORDS1 =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1);

  SECTION("Clipped ZoomBufferTranPoint")
  {
    REQUIRE(Point2dInt{0, 0} == filterBuffers.GetBuffMidpoint());

    // Lerp to the dest buffer only
    filterBuffers.SetTranLerpFactor(FilterBuffers::GetMaxTranLerpFactor());
    REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());

    // tranPoint comes solely from the dest Zoom buffer which because we are using a
    // const ZoomVectorFunc, returns a const normalized value
    UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_1.x = " << NML_CONST_ZOOM_VECTOR_COORDS1.GetX());
    UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_1.y = " << NML_CONST_ZOOM_VECTOR_COORDS1.GetY());
    UNSCOPED_INFO(
        "GetZoomInPoint(DUMMY_NML_COORDS).x = "
        << CONSTANT_ZOOM_VECTOR.GetZoomInPoint(DUMMY_NML_COORDS, DUMMY_NML_COORDS).GetX());
    UNSCOPED_INFO(
        "GetZoomInPoint(DUMMY_NML_COORDS).y = "
        << CONSTANT_ZOOM_VECTOR.GetZoomInPoint(DUMMY_NML_COORDS, DUMMY_NML_COORDS).GetY());
    REQUIRE(NML_CONST_ZOOM_VECTOR_COORDS1.Equals(
        CONSTANT_ZOOM_VECTOR.GetZoomInPoint(DUMMY_NML_COORDS, DUMMY_NML_COORDS)));
    const auto normalizedMidPt =
        NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(filterBuffers.GetBuffMidpoint());
    const auto expectedTranPoint = COORD_TRANSFORMS.NormalizedToTranPoint(
        normalizedMidPt +
        NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1));
    // Because mid-point is zero, the trans point is negative and therefore clipped.
    REQUIRE(expectedTranPoint.x < 0);
    REQUIRE(expectedTranPoint.y < 0);

    UNSCOPED_INFO("filterBuffers.GetBuffMidpoint().x = " << filterBuffers.GetBuffMidpoint().x);
    UNSCOPED_INFO("filterBuffers.GetBuffMidpoint().y = " << filterBuffers.GetBuffMidpoint().y);
    UNSCOPED_INFO("normalizedMidPt.x = " << normalizedMidPt.GetX());
    UNSCOPED_INFO("normalizedMidPt.y = " << normalizedMidPt.GetY());
    UNSCOPED_INFO("expectedTranPoint.x = " << expectedTranPoint.x);
    UNSCOPED_INFO("expectedTranPoint.y = " << expectedTranPoint.y);

    const auto destPoint = filterBuffers.GetSourcePointInfo(0);
    UNSCOPED_INFO("destPoint.isClipped = " << destPoint.isClipped);
    UNSCOPED_INFO("destPoint.screenPoint.x = " << destPoint.screenPoint.x);
    UNSCOPED_INFO("destPoint.screenPoint.y = " << destPoint.screenPoint.y);
    REQUIRE(destPoint.isClipped);
    REQUIRE(0 == destPoint.screenPoint.x);
    REQUIRE(0 == destPoint.screenPoint.y);

    // TODO(glk) Test coeff values
  }
}
// NOLINTEND(readability-function-cognitive-complexity)

} // namespace GOOM::UNIT_TESTS
