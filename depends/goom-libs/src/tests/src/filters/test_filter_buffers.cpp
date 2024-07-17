// NOLINTBEGIN(cert-err58-cpp): Catch2 3.6.0 issue

// TODO(glk) - Look at a better way to deal with this - Vitesse::SetVitesse
#if not defined(_MSC_VER)
#else
#pragma warning(push)
#pragma warning(disable : 4296)
#pragma warning(pop)
#endif

#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <span>
#include <vector>

import Goom.Control.GoomSoundEvents;
import Goom.FilterFx.FilterBuffers;
import Goom.FilterFx.FilterSettings;
import Goom.FilterFx.FilterZoomVector;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.Math.GoomRandBase;
import Goom.Utils.GoomTime;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;
import Goom.Lib.SoundInfo;
import Goom.PluginInfo;

namespace GOOM::UNIT_TESTS
{

using CONTROL::GoomSoundEvents;
using FILTER_FX::FilterEffectsSettings;
using FILTER_FX::FilterZoomVector;
using FILTER_FX::NormalizedCoords;
using FILTER_FX::NormalizedCoordsConverter;
using FILTER_FX::ZoomFilterBuffers;
using UTILS::GoomTime;
using UTILS::MATH::IGoomRand;

class TestFilterBuffers : public ZoomFilterBuffers
{
public:
  TestFilterBuffers(const PluginInfo& goomInfo,
                    const NormalizedCoordsConverter& normalizedCoordsConverter,
                    const ZoomFilterBuffers::ZoomPointFunc& getZoomPointFunc) noexcept
    : ZoomFilterBuffers{goomInfo, normalizedCoordsConverter, getZoomPointFunc}
  {
  }

  [[nodiscard]] auto GetBufferBuffMidpoint() const noexcept -> Point2dInt
  {
    return ZoomFilterBuffers::GetTransformBufferMidpoint();
  }

  auto UpdateTransBuffer() noexcept -> void { ZoomFilterBuffers::UpdateTransformBuffer(); }
};

namespace
{

//#define LARGE_SCREEN_TEST
constexpr auto LARGE_WIDTH  = 3840U;
constexpr auto LARGE_HEIGHT = 2160U;
#ifdef LARGE_SCREEN_TEST
constexpr auto WIDTH  = LARGE_WIDTH;
constexpr auto HEIGHT = LARGE_HEIGHT;
#else
constexpr auto WIDTH  = LARGE_WIDTH / 10U;
constexpr auto HEIGHT = LARGE_HEIGHT / 10U;
#endif
constexpr auto* RESOURCES_DIRECTORY = "";
const auto GOOM_RAND                = IGoomRand{};

const auto SOUND_INFO   = SoundInfo{};
const auto GOOM_TIME    = GoomTime{};
const auto SOUND_EVENTS = GoomSoundEvents{GOOM_TIME, SOUND_INFO};
const auto GOOM_INFO    = PluginInfo{
       {WIDTH, HEIGHT},
       GOOM_TIME, SOUND_EVENTS
};
constexpr auto NORMALIZED_COORDS_CONVERTER = NormalizedCoordsConverter{
    {WIDTH, HEIGHT}
};

constexpr auto MID_PT                     = MidpointFromOrigin({WIDTH, HEIGHT});
constexpr auto CONST_ZOOM_VECTOR_COORDS_1 = Point2dInt{16, 40};
constexpr auto CONST_ZOOM_VECTOR_COORDS_2 = Point2dInt{32, 52};
constexpr auto DUMMY_COORDS               = Point2dInt{14, 38};

class TestZoomVector : public FilterZoomVector
{
public:
  explicit TestZoomVector(const bool returnConst) noexcept
    : FilterZoomVector{WIDTH, RESOURCES_DIRECTORY, GOOM_RAND}, m_returnConst{returnConst}
  {
  }

  auto SetFilterEffectsSettings(const FilterEffectsSettings& filterEffectsSettings) noexcept
      -> void override;

  [[nodiscard]] auto GetConstCoords() const noexcept -> const Point2dInt& { return m_constCoords; }
  auto SetConstCoords(const Point2dInt& coords) -> void { m_constCoords = coords; }

  auto SetZoomAdjustment(const float val) noexcept -> void { m_zoomAdjustment = val; }

  [[nodiscard]] auto GetZoomPoint(const NormalizedCoords& coords) const noexcept
      -> NormalizedCoords override;

private:
  bool m_returnConst;
  Point2dInt m_constCoords = CONST_ZOOM_VECTOR_COORDS_1;
  float m_zoomAdjustment   = 0.0F;
};

auto TestZoomVector::SetFilterEffectsSettings(
    const FilterEffectsSettings& filterEffectsSettings) noexcept -> void
{
  FilterZoomVector::SetFilterEffectsSettings(filterEffectsSettings);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto TestZoomVector::GetZoomPoint(const NormalizedCoords& coords) const noexcept -> NormalizedCoords
{
  if (m_returnConst)
  {
    return NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(m_constCoords);
  }
  return (1.0F - m_zoomAdjustment) * coords;
}

const auto IDENTITY_ZOOM_VECTOR = TestZoomVector{false};
const auto CONSTANT_ZOOM_VECTOR = TestZoomVector{true};

[[nodiscard]] auto GetFilterBuffers(const TestZoomVector& zoomVector) noexcept -> TestFilterBuffers
{
  return TestFilterBuffers{GOOM_INFO,
                           NORMALIZED_COORDS_CONVERTER,
                           [&zoomVector](const NormalizedCoords& normalizedCoords)
                           { return zoomVector.GetZoomPoint(normalizedCoords); }};
}

constexpr auto TEST_X          = 10;
constexpr auto TEST_Y          = 50;
constexpr auto TEST_SRCE_POINT = Point2dInt{TEST_X, TEST_Y};
static_assert((0 <= TEST_X) && (TEST_X < WIDTH), "Invalid X");
static_assert((0 <= TEST_Y) && (TEST_Y < WIDTH), "Invalid Y");

constexpr auto DUMMY_NML_COORDS = NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(DUMMY_COORDS);

} // namespace

// NOLINTBEGIN(bugprone-chained-comparison): Catch2 needs to fix this.
// NOLINTBEGIN(readability-function-cognitive-complexity)
TEST_CASE("ZoomFilterBuffers Basic")
{
  auto filterBuffers = GetFilterBuffers(IDENTITY_ZOOM_VECTOR);

  filterBuffers.SetTransformBufferMidpoint(MID_PT);
  filterBuffers.Start();

  SECTION("Correct Starting TranBuffersState")
  {
    REQUIRE(ZoomFilterBuffers::UpdateStatus::IN_PROGRESS == filterBuffers.GetUpdateStatus());
  }
  SECTION("Correct Starting BuffMidpoint()")
  {
    REQUIRE(MID_PT == filterBuffers.GetBufferBuffMidpoint());
  }
}

TEST_CASE("ZoomFilterBuffers Calculations - Correct Dest ZoomBufferTranPoint")
{
  auto filterBuffers = GetFilterBuffers(CONSTANT_ZOOM_VECTOR);
  filterBuffers.SetTransformBufferMidpoint(MID_PT);
  filterBuffers.Start();
  REQUIRE(ZoomFilterBuffers::UpdateStatus::IN_PROGRESS == filterBuffers.GetUpdateStatus());

  static constexpr auto NML_CONST_ZOOM_VECTOR_COORDS_1 =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1);

  REQUIRE(CONST_ZOOM_VECTOR_COORDS_1 == CONSTANT_ZOOM_VECTOR.GetConstCoords());
  REQUIRE(MID_PT == filterBuffers.GetBufferBuffMidpoint());

  // tranPoint comes solely from the dest Zoom buffer which because we are using a
  // const ZoomVectorFunc, returns a const normalized value.
  UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_1.x = " << NML_CONST_ZOOM_VECTOR_COORDS_1.GetX());
  UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_1.y = " << NML_CONST_ZOOM_VECTOR_COORDS_1.GetY());
  UNSCOPED_INFO("GetZoomPoint(DUMMY_NML_COORDS).x = "
                << CONSTANT_ZOOM_VECTOR.GetZoomPoint(DUMMY_NML_COORDS).GetX());
  UNSCOPED_INFO("GetZoomPoint(DUMMY_NML_COORDS).y = "
                << CONSTANT_ZOOM_VECTOR.GetZoomPoint(DUMMY_NML_COORDS).GetY());
  REQUIRE(
      NML_CONST_ZOOM_VECTOR_COORDS_1.Equals(CONSTANT_ZOOM_VECTOR.GetZoomPoint(DUMMY_NML_COORDS)));
}

namespace
{

auto TestCorrectStripesBasicValues(const TestFilterBuffers& filterBuffers) -> void
{
  static constexpr auto NML_CONST_ZOOM_VECTOR_COORDS1 =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1);

  REQUIRE(CONST_ZOOM_VECTOR_COORDS_1 == CONSTANT_ZOOM_VECTOR.GetConstCoords());
  REQUIRE(MID_PT == filterBuffers.GetBufferBuffMidpoint());
  REQUIRE(
      NML_CONST_ZOOM_VECTOR_COORDS1.Equals(CONSTANT_ZOOM_VECTOR.GetZoomPoint(DUMMY_NML_COORDS)));
}

} // namespace

TEST_CASE("ZoomFilterBuffers Stripes")
{
  auto constantZoomVector = TestZoomVector{true};

  auto filterBuffers = GetFilterBuffers(constantZoomVector);
  filterBuffers.SetTransformBufferMidpoint(MID_PT);
  filterBuffers.Start();
  REQUIRE(ZoomFilterBuffers::UpdateStatus::IN_PROGRESS == filterBuffers.GetUpdateStatus());

  TestCorrectStripesBasicValues(filterBuffers);

  constantZoomVector.SetConstCoords(CONST_ZOOM_VECTOR_COORDS_2);
  REQUIRE(CONST_ZOOM_VECTOR_COORDS_2 == constantZoomVector.GetConstCoords());

  // Make sure dest buffer is completely copied to srce buffer at end of update.
  REQUIRE(ZoomFilterBuffers::UpdateStatus::IN_PROGRESS == filterBuffers.GetUpdateStatus());
  filterBuffers.UpdateTransBuffer();
  REQUIRE(ZoomFilterBuffers::UpdateStatus::AT_END == filterBuffers.GetUpdateStatus());

  REQUIRE(CONST_ZOOM_VECTOR_COORDS_2 == constantZoomVector.GetConstCoords());
  REQUIRE(MID_PT == filterBuffers.GetBufferBuffMidpoint());

  std::vector<Point2dFlt> destBuffVec((GOOM_INFO.GetDimensions().GetSize()));
  const std::span<Point2dFlt> destBuff{destBuffVec};
  filterBuffers.CopyTransformBuffer(destBuff);
  REQUIRE(ZoomFilterBuffers::UpdateStatus::HAS_BEEN_COPIED == filterBuffers.GetUpdateStatus());

  static constexpr auto NML_CONST_ZOOM_VECTOR_COORDS_2 =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_2);
  static constexpr auto NML_MID_PT = NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(MID_PT);
  static constexpr auto NML_UNCENTERED_ZOOM_VECTOR_COORDS_2 =
      NML_MID_PT + NML_CONST_ZOOM_VECTOR_COORDS_2;

  UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_2.x = " << NML_CONST_ZOOM_VECTOR_COORDS_2.GetX());
  UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_2.y = " << NML_CONST_ZOOM_VECTOR_COORDS_2.GetY());
  UNSCOPED_INFO("NML_MID_PT.x = " << NML_MID_PT.GetX());
  UNSCOPED_INFO("NML_MID_PT.y = " << NML_MID_PT.GetY());
  UNSCOPED_INFO(
      "NML_UNCENTERED_ZOOM_VECTOR_COORDS_2.x = " << NML_UNCENTERED_ZOOM_VECTOR_COORDS_2.GetX());
  UNSCOPED_INFO(
      "NML_UNCENTERED_ZOOM_VECTOR_COORDS_2.y = " << NML_UNCENTERED_ZOOM_VECTOR_COORDS_2.GetY());
  UNSCOPED_INFO("destBuff[0].x = " << destBuff[0].x);
  UNSCOPED_INFO("destBuff[0].y = " << destBuff[0].y);
  for (const auto& destVal : destBuff)
  {
    REQUIRE(destVal.x == NML_UNCENTERED_ZOOM_VECTOR_COORDS_2.GetX());
    REQUIRE(destVal.y == NML_UNCENTERED_ZOOM_VECTOR_COORDS_2.GetY());
  }

  filterBuffers.ResetTransformBufferToStart();
  REQUIRE(ZoomFilterBuffers::UpdateStatus::AT_START == filterBuffers.GetUpdateStatus());
  filterBuffers.StartTransformBufferUpdates();
  REQUIRE(ZoomFilterBuffers::UpdateStatus::IN_PROGRESS == filterBuffers.GetUpdateStatus());
}

TEST_CASE("ZoomFilterBuffers Adjustment")
{
  static constexpr auto TEST_SRCE_NML_COORDS =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(TEST_SRCE_POINT);

  auto zoomVector = TestZoomVector{false};

  auto filterBuffers = GetFilterBuffers(zoomVector);

  filterBuffers.SetTransformBufferMidpoint(MID_PT);
  filterBuffers.Start();
  REQUIRE(ZoomFilterBuffers::UpdateStatus::IN_PROGRESS == filterBuffers.GetUpdateStatus());

  SECTION("Correct Zoomed In Dest ZoomBufferTranPoint")
  {
    REQUIRE(TEST_SRCE_NML_COORDS.Equals(zoomVector.GetZoomPoint(TEST_SRCE_NML_COORDS)));

    static constexpr auto ZOOM_ADJUSTMENT1 = 0.2F;
    static constexpr auto ZOOM_FACTOR1     = 1.0F - ZOOM_ADJUSTMENT1;
    zoomVector.SetZoomAdjustment(ZOOM_ADJUSTMENT1);
    REQUIRE((ZOOM_FACTOR1 * TEST_SRCE_NML_COORDS)
                .Equals(zoomVector.GetZoomPoint(TEST_SRCE_NML_COORDS)));

    // GetSourcePoint uses tranPoint which comes solely from the dest Zoom buffer.
    // Because we are using a zoomed in ZoomVectorFunc, tranPoint should be zoomed in.
    REQUIRE(ZoomFilterBuffers::UpdateStatus::IN_PROGRESS == filterBuffers.GetUpdateStatus());
    filterBuffers.UpdateTransBuffer();
    REQUIRE(ZoomFilterBuffers::UpdateStatus::AT_END == filterBuffers.GetUpdateStatus());

    //    const auto expectedTranPoint = ZoomCoordTransforms::ScreenToTranPoint(TEST_SRCE_POINT);
    //    const auto expectedZoomedInTranPoint = Point2dInt{
    //        static_cast<int32_t>(ZOOM_FACTOR1 *
    //                             static_cast<float>(expectedTranPoint.x - MID_TRAN_POINT.x)) +
    //            MID_TRAN_POINT.x,
    //        static_cast<int32_t>(ZOOM_FACTOR1 *
    //                             static_cast<float>(expectedTranPoint.y - MID_TRAN_POINT.y)) +
    //            MID_TRAN_POINT.y};
    //    const auto expectedZoomedInSrcePoint =
    //        ZoomCoordTransforms::TranToScreenPoint(expectedZoomedInTranPoint);
    //    const auto srcePoint = GetSourcePoint(filterBuffers, GetBuffPos(TEST_X, TEST_Y));
    //    UNSCOPED_INFO("expectedTranPoint.x = " << expectedTranPoint.x);
    //    UNSCOPED_INFO("expectedTranPoint.y = " << expectedTranPoint.y);
    //    UNSCOPED_INFO("expectedZoomedInTranPoint.x = " << expectedZoomedInTranPoint.x);
    //    UNSCOPED_INFO("expectedZoomedInTranPoint.y = " << expectedZoomedInTranPoint.y);
    //    UNSCOPED_INFO("expectedZoomedInSrcePoint.x = " << expectedZoomedInSrcePoint.x);
    //    UNSCOPED_INFO("expectedZoomedInSrcePoint.y = " << expectedZoomedInSrcePoint.y);
    //    UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
    //    UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);
    //    REQUIRE(expectedZoomedInSrcePoint == srcePoint);
  }
}

TEST_CASE("ZoomFilterBuffers Clipping")
{
  auto filterBuffers = GetFilterBuffers(CONSTANT_ZOOM_VECTOR);
  filterBuffers.SetTransformBufferMidpoint({0, 0});
  filterBuffers.Start();
  REQUIRE(ZoomFilterBuffers::UpdateStatus::IN_PROGRESS == filterBuffers.GetUpdateStatus());

  static constexpr auto NML_CONST_ZOOM_VECTOR_COORDS1 =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1);

  SECTION("Clipped ZoomBufferTranPoint")
  {
    REQUIRE(Point2dInt{0, 0} == filterBuffers.GetBufferBuffMidpoint());

    // tranPoint comes solely from the dest Zoom buffer which because we are using a
    // const ZoomVectorFunc, returns a const normalized value
    UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_1.x = " << NML_CONST_ZOOM_VECTOR_COORDS1.GetX());
    UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_1.y = " << NML_CONST_ZOOM_VECTOR_COORDS1.GetY());
    UNSCOPED_INFO("GetZoomPoint(DUMMY_NML_COORDS).x = "
                  << CONSTANT_ZOOM_VECTOR.GetZoomPoint(DUMMY_NML_COORDS).GetX());
    UNSCOPED_INFO("GetZoomPoint(DUMMY_NML_COORDS).y = "
                  << CONSTANT_ZOOM_VECTOR.GetZoomPoint(DUMMY_NML_COORDS).GetY());
    REQUIRE(
        NML_CONST_ZOOM_VECTOR_COORDS1.Equals(CONSTANT_ZOOM_VECTOR.GetZoomPoint(DUMMY_NML_COORDS)));
    const auto normalizedMidPt =
        NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(filterBuffers.GetBufferBuffMidpoint());
    //    const auto expectedTranPoint = COORD_TRANSFORMS.NormalizedToTranPoint(
    //        normalizedMidPt +
    //        NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1));
    //    // Because mid-point is zero, the trans point is negative and therefore clipped.
    //    REQUIRE(expectedTranPoint.x < 0);
    //    REQUIRE(expectedTranPoint.y < 0);

    UNSCOPED_INFO("filterBuffers.GetTransformBufferBuffMidpoint().x = "
                  << filterBuffers.GetBufferBuffMidpoint().x);
    UNSCOPED_INFO("filterBuffers.GetTransformBufferBuffMidpoint().y = "
                  << filterBuffers.GetBufferBuffMidpoint().y);
    UNSCOPED_INFO("normalizedMidPt.x = " << normalizedMidPt.GetX());
    UNSCOPED_INFO("normalizedMidPt.y = " << normalizedMidPt.GetY());
    //    UNSCOPED_INFO("expectedTranPoint.x = " << expectedTranPoint.x);
    //    UNSCOPED_INFO("expectedTranPoint.y = " << expectedTranPoint.y);

    //    const auto destPoint = filterBuffers.GetSourcePointInfo(0);
    //    UNSCOPED_INFO("destPoint.isClipped = " << destPoint.isClipped);
    //    UNSCOPED_INFO("destPoint.screenPoint.x = " << destPoint.screenPoint.x);
    //    UNSCOPED_INFO("destPoint.screenPoint.y = " << destPoint.screenPoint.y);
    //    REQUIRE(destPoint.isClipped);
    //    REQUIRE(0 == destPoint.screenPoint.x);
    //    REQUIRE(0 == destPoint.screenPoint.y);

    // TODO(glk) Test coeff values
  }
}
// NOLINTEND(readability-function-cognitive-complexity)
// NOLINTEND(bugprone-chained-comparison)

} // namespace GOOM::UNIT_TESTS

// NOLINTEND(cert-err58-cpp): Catch2 3.6.0 issue
