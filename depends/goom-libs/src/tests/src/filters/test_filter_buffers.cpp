#ifndef GOOM_DEBUG
#define GOOM_DEBUG
#endif

#include "catch2/catch.hpp"
#include "control/goom_sound_events.h"
#include "filter_fx/filter_buffers.h"
#include "filter_fx/filter_settings.h"
#include "filter_fx/filter_zoom_vector.h"
#include "filter_fx/normalized_coords.h"
#include "goom_config.h"
#include "goom_plugin_info.h"
#include "point2d.h"
#include "sound_info.h"
#include "utils/math/goom_rand.h"
#include "utils/math/misc.h"
#include "utils/parallel_utils.h"

#include <cmath>

namespace GOOM::UNIT_TESTS
{

using CONTROL::GoomSoundEvents;
using FILTER_FX::FilterZoomVector;
using FILTER_FX::NormalizedCoords;
using FILTER_FX::NormalizedCoordsConverter;
using FILTER_FX::ZoomFilterBuffers;
using FILTER_FX::ZoomFilterEffectsSettings;
using UTILS::Parallel;
using UTILS::MATH::FloatsEqual;
using UTILS::MATH::GoomRand;

static constexpr size_t WIDTH = 120;
static constexpr size_t HEIGHT = 70;
static constexpr const char* RESOURCES_DIRECTORY = "";
static const GoomRand GOOM_RAND{};
static const NormalizedCoordsConverter NORMALIZED_COORDS_CONVERTER{
    {WIDTH, HEIGHT},
    ZoomFilterBuffers::MIN_SCREEN_COORD_ABS_VAL
};

const Point2dInt MID_PT = {static_cast<int32_t>(WIDTH) / 2, static_cast<int32_t>(HEIGHT) / 2};
const Point2dInt CONST_ZOOM_VECTOR_COORDS_1 = {16, 40};
const Point2dInt CONST_ZOOM_VECTOR_COORDS_2 = {32, 52};
const Point2dInt DUMMY_COORDS = {14, 38};

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

  void SetFilterSettings(const ZoomFilterEffectsSettings& filterEffectsSettings) override;

  [[nodiscard]] auto GetConstCoords() const -> const Point2dInt& { return m_constCoords; }
  void SetConstCoords(const Point2dInt& coords) { m_constCoords = coords; }

  [[nodiscard]] auto GetZoomPoint(const NormalizedCoords& coords) const
      -> NormalizedCoords override;

private:
  const bool m_returnConst;
  Point2dInt m_constCoords = CONST_ZOOM_VECTOR_COORDS_1;
};

void TestZoomVector::SetFilterSettings(const ZoomFilterEffectsSettings& filterEffectsSettings)
{
  FilterZoomVector::SetFilterSettings(filterEffectsSettings);
}

auto TestZoomVector::GetZoomPoint(const NormalizedCoords& coords) const -> NormalizedCoords
{
  if (m_returnConst)
  {
    return NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(m_constCoords);
  }
  return coords;
}

using CoordTransforms = ZoomFilterBuffers::CoordTransforms;
static const CoordTransforms COORD_TRANSFORMS{NORMALIZED_COORDS_CONVERTER};

auto GetSourcePoint(const ZoomFilterBuffers& filterBuffers, const size_t buffPos) -> Point2dInt
{
  return filterBuffers.GetSourcePointInfo(buffPos).screenPoint;
}

TEST_CASE("ZoomFilterBuffers Basic", "[ZoomFilterBuffers]")
{
  static constexpr int32_t TEST_X = 10;
  static constexpr int32_t TEST_Y = 50;
  const Point2dInt TEST_SRCE_POINT = {TEST_X, TEST_Y};
  static_assert((0 <= TEST_X) && (TEST_X < WIDTH), "Invalid X");
  static_assert((0 <= TEST_Y) && (TEST_Y < WIDTH), "Invalid Y");

  Parallel parallel{-1};
  SoundInfo soundInfo{};
  GoomSoundEvents goomSoundEvents{soundInfo};

  const PluginInfo goomInfo{{WIDTH, HEIGHT}, goomSoundEvents};
  TestZoomVector identityZoomVector{false};
  ZoomFilterBuffers filterBuffers{parallel, goomInfo, NORMALIZED_COORDS_CONVERTER,
                                  [&](const NormalizedCoords& normalizedCoords)
                                  { return identityZoomVector.GetZoomPoint(normalizedCoords); }};

  filterBuffers.SetBuffMidpoint(MID_PT);
  filterBuffers.Start();

  const NormalizedCoords DUMMY_NML_COORDS{
      NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(DUMMY_COORDS)};

  SECTION("Correct Starting TranBuffersState")
  {
    REQUIRE(ZoomFilterBuffers::TranBuffersState::START_FRESH_TRAN_BUFFERS ==
            filterBuffers.GetTranBuffersState());
  }
  SECTION("Correct Starting BuffYLineStart")
  {
    REQUIRE(0 == filterBuffers.GetTranBuffYLineStart());
  }
  SECTION("Correct Starting BuffMidpoint()") { REQUIRE(MID_PT == filterBuffers.GetBuffMidpoint()); }
  SECTION("Correct Starting HaveFilterSettingsChanged")
  {
    REQUIRE(!filterBuffers.HaveFilterSettingsChanged());
  }
  SECTION("Correct Starting TranLerpFactor") { REQUIRE(0 == filterBuffers.GetTranLerpFactor()); }
  SECTION("Correct Starting ZoomBufferTranPoint")
  {
    REQUIRE(DUMMY_NML_COORDS.Equals(identityZoomVector.GetZoomPoint(DUMMY_NML_COORDS)));

    // TranLerpFactor is zero so starting tranPoint should be srce identity buffer
    const Point2dInt expectedSrceIdentityPoint = TEST_SRCE_POINT;
    const Point2dInt srcePoint = GetSourcePoint(filterBuffers, GetBuffPos(TEST_X, TEST_Y));
    UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
    UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);
    REQUIRE(expectedSrceIdentityPoint == srcePoint);
  }
  SECTION("Correct Dest ZoomBufferTranPoint")
  {
    // Lerp to the dest buffer only
    filterBuffers.SetTranLerpFactor(filterBuffers.GetMaxTranLerpFactor());
    REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());

    REQUIRE(DUMMY_NML_COORDS.Equals(identityZoomVector.GetZoomPoint(DUMMY_NML_COORDS)));

    // tranPoint comes solely from the dest Zoom buffer which because we are using an
    // identity ZoomVectorFunc should be the identity point.
    const Point2dInt expectedSrceIdentityPoint = TEST_SRCE_POINT;
    const Point2dInt srcePoint = GetSourcePoint(filterBuffers, GetBuffPos(TEST_X, TEST_Y));
    UNSCOPED_INFO("expectedSrceIdentityPoint.x = " << expectedSrceIdentityPoint.x);
    UNSCOPED_INFO("expectedSrceIdentityPoint.y = " << expectedSrceIdentityPoint.y);
    UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
    UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);
    REQUIRE(expectedSrceIdentityPoint == srcePoint);
  }
  SECTION("Correct Lerped ZoomBufferTranPoint")
  {
    static constexpr float tLerp = 0.5F;
    const auto tranLerpFactor = static_cast<int32_t>(
        std::round(tLerp * static_cast<float>(filterBuffers.GetMaxTranLerpFactor())));

    filterBuffers.SetTranLerpFactor(tranLerpFactor);
    REQUIRE(tranLerpFactor == filterBuffers.GetTranLerpFactor());

    REQUIRE(DUMMY_NML_COORDS.Equals(identityZoomVector.GetZoomPoint(DUMMY_NML_COORDS)));

    // tranPoint comes solely from the lerp middle of srce and dest Zoom buffer which
    // because we are using an identity ZoomVectorFunc should still be the identity point.
    const Point2dInt expectedSrceIdentityPoint = TEST_SRCE_POINT;
    const Point2dInt srcePoint = GetSourcePoint(filterBuffers, GetBuffPos(TEST_X, TEST_Y));
    UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
    UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);
    REQUIRE(expectedSrceIdentityPoint == srcePoint);
  }
}

TEST_CASE("ZoomFilterBuffers Calculations")
{
  Parallel parallel{-1};
  SoundInfo soundInfo{};
  GoomSoundEvents goomSoundEvents{soundInfo};

  const PluginInfo goomInfo{{WIDTH, HEIGHT}, goomSoundEvents};
  TestZoomVector constantZoomVector{true};
  ZoomFilterBuffers filterBuffers{parallel, goomInfo, NORMALIZED_COORDS_CONVERTER,
                                  [&](const NormalizedCoords& normalizedCoords)
                                  { return constantZoomVector.GetZoomPoint(normalizedCoords); }};

  filterBuffers.SetBuffMidpoint(MID_PT);
  filterBuffers.Start();

  const NormalizedCoords DUMMY_NML_COORDS{
      NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(DUMMY_COORDS)};
  const NormalizedCoords NML_CONST_ZOOM_VECTOR_COORDS_1{
      NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1)};

  SECTION("Correct Dest ZoomBufferTranPoint")
  {
    REQUIRE(CONST_ZOOM_VECTOR_COORDS_1 == constantZoomVector.GetConstCoords());
    REQUIRE(MID_PT == filterBuffers.GetBuffMidpoint());

    // Lerp to the dest buffer only
    filterBuffers.SetTranLerpFactor(filterBuffers.GetMaxTranLerpFactor());
    REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());

    // tranPoint comes solely from the dest Zoom buffer which because we are using a
    // const ZoomVectorFunc, returns a const normalized value
    UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_1.x = " << NML_CONST_ZOOM_VECTOR_COORDS_1.GetX());
    UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_1.y = " << NML_CONST_ZOOM_VECTOR_COORDS_1.GetY());
    UNSCOPED_INFO("GetZoomPoint(DUMMY_NML_COORDS).x = " << constantZoomVector.GetZoomPoint(DUMMY_NML_COORDS).GetX());
    UNSCOPED_INFO("GetZoomPoint(DUMMY_NML_COORDS).y = " << constantZoomVector.GetZoomPoint(DUMMY_NML_COORDS).GetY());
    REQUIRE(
        NML_CONST_ZOOM_VECTOR_COORDS_1.Equals(constantZoomVector.GetZoomPoint(DUMMY_NML_COORDS)));
    const NormalizedCoords normalizedMidPt{
        NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(filterBuffers.GetBuffMidpoint())};
    const Point2dInt expectedTranPoint = COORD_TRANSFORMS.NormalizedToTranPoint(
        normalizedMidPt + NormalizedCoords{NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(
                              CONST_ZOOM_VECTOR_COORDS_1)});
    UNSCOPED_INFO("filterBuffers.GetBuffMidpoint().x = " << filterBuffers.GetBuffMidpoint().x);
    UNSCOPED_INFO("filterBuffers.GetBuffMidpoint().y = " << filterBuffers.GetBuffMidpoint().y);
    UNSCOPED_INFO("normalizedMidPt.x = " << normalizedMidPt.GetX());
    UNSCOPED_INFO("normalizedMidPt.y = " << normalizedMidPt.GetY());
    UNSCOPED_INFO("expectedTranPoint.x = " << expectedTranPoint.x);
    UNSCOPED_INFO("expectedTranPoint.y = " << expectedTranPoint.y);

    const Point2dInt expectedSrcePoint = CoordTransforms::TranToScreenPoint(expectedTranPoint);
    UNSCOPED_INFO("expectedSrcePoint.x = " << expectedSrcePoint.x);
    UNSCOPED_INFO("expectedSrcePoint.y = " << expectedSrcePoint.y);

    for (size_t buffPos = 0; buffPos < WIDTH * HEIGHT; buffPos++)
    {
      const Point2dInt srcePoint = GetSourcePoint(filterBuffers, buffPos);
      UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
      UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);

      REQUIRE(expectedSrcePoint == srcePoint);
    }
  }

  SECTION("Correct Srce/Dest ZoomBufferTranPoint")
  {
    REQUIRE(CONST_ZOOM_VECTOR_COORDS_1 == constantZoomVector.GetConstCoords());
    REQUIRE(MID_PT == filterBuffers.GetBuffMidpoint());

    static constexpr float tLerp = 0.5F;
    const auto tranLerpFactor = static_cast<int32_t>(
        std::round(tLerp * static_cast<float>(filterBuffers.GetMaxTranLerpFactor())));

    filterBuffers.SetTranLerpFactor(tranLerpFactor);
    REQUIRE(tranLerpFactor == filterBuffers.GetTranLerpFactor());

    for (int32_t y = 0; y < static_cast<int32_t>(HEIGHT); y++)
    {
      for (int32_t x = 0; x < static_cast<int32_t>(WIDTH); x++)
      {
        // tranPoint comes from halfway between srce and dest Zoom buffer.
        REQUIRE(NML_CONST_ZOOM_VECTOR_COORDS_1.Equals(
            constantZoomVector.GetZoomPoint(DUMMY_NML_COORDS)));
        const NormalizedCoords normalizedSrcePt{
            NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(Point2dInt{x, y})};
        const Point2dInt expectedSrceTranPoint =
            COORD_TRANSFORMS.NormalizedToTranPoint(normalizedSrcePt);
        const NormalizedCoords normalizedMidPt{
            NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(filterBuffers.GetBuffMidpoint())};
        UNSCOPED_INFO("filterBuffers.GetBuffMidpoint().x = " << filterBuffers.GetBuffMidpoint().x);
        UNSCOPED_INFO("filterBuffers.GetBuffMidpoint().y = " << filterBuffers.GetBuffMidpoint().y);
        UNSCOPED_INFO("normalizedMidPt.x = " << normalizedMidPt.GetX());
        UNSCOPED_INFO("normalizedMidPt.y = " << normalizedMidPt.GetY());
        const Point2dInt expectedDestTranPoint = COORD_TRANSFORMS.NormalizedToTranPoint(
            normalizedMidPt + NormalizedCoords{NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(
                                  CONST_ZOOM_VECTOR_COORDS_1)});
        const Point2dInt expectedTranPoint = {
            STD20::lerp(expectedSrceTranPoint.x, expectedDestTranPoint.x, tLerp),
            STD20::lerp(expectedSrceTranPoint.y, expectedDestTranPoint.y, tLerp)};
        UNSCOPED_INFO("expectedTranPoint.x = " << expectedTranPoint.x);
        UNSCOPED_INFO("expectedTranPoint.y = " << expectedTranPoint.y);

        const Point2dInt expectedScreenPoint =
            CoordTransforms::TranToScreenPoint(expectedTranPoint);
        UNSCOPED_INFO("expectedScreenPoint.x = " << expectedScreenPoint.x);
        UNSCOPED_INFO("expectedScreenPoint.y = " << expectedScreenPoint.y);

        const Point2dInt screenPoint = GetSourcePoint(filterBuffers, GetBuffPos(x, y));
        UNSCOPED_INFO("srcePoint.x = " << screenPoint.x);
        UNSCOPED_INFO("srcePoint.y = " << screenPoint.y);

        REQUIRE(expectedScreenPoint == screenPoint);
      }
    }
  }
}

TEST_CASE("ZoomFilterBuffers Stripes", "[ZoomFilterBuffersStripes]")
{
  static constexpr int32_t TEST_X = 10;
  static constexpr int32_t TEST_Y = 50;
  static_assert((0 <= TEST_X) && (TEST_X < WIDTH), "Invalid X");
  static_assert((0 <= TEST_Y) && (TEST_Y < WIDTH), "Invalid Y");

  Parallel parallel{0};
  SoundInfo soundInfo{};
  GoomSoundEvents goomSoundEvents{soundInfo};

  const PluginInfo goomInfo{{WIDTH, HEIGHT}, goomSoundEvents};
  TestZoomVector constantZoomVector{true};
  ZoomFilterBuffers filterBuffers{parallel, goomInfo, NORMALIZED_COORDS_CONVERTER,
                                  [&](const NormalizedCoords& normalizedCoords)
                                  { return constantZoomVector.GetZoomPoint(normalizedCoords); }};

  filterBuffers.SetBuffMidpoint(MID_PT);
  filterBuffers.Start();

  const NormalizedCoords DUMMY_NML_COORDS{
      NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(DUMMY_COORDS)};
  const NormalizedCoords NML_CONST_ZOOM_VECTOR_COORDS_1{
      NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1)};
  const NormalizedCoords NML_CONST_ZOOM_VECTOR_COORDS_2{
      NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_2)};

  SECTION("ZoomBuffer Stripe")
  {
    REQUIRE(ZoomFilterBuffers::TranBuffersState::START_FRESH_TRAN_BUFFERS ==
            filterBuffers.GetTranBuffersState());
    REQUIRE(!filterBuffers.HaveFilterSettingsChanged());

    REQUIRE(CONST_ZOOM_VECTOR_COORDS_1 == constantZoomVector.GetConstCoords());
    REQUIRE(MID_PT == filterBuffers.GetBuffMidpoint());
    REQUIRE(
        NML_CONST_ZOOM_VECTOR_COORDS_1.Equals(constantZoomVector.GetZoomPoint(DUMMY_NML_COORDS)));

    // Make sure dest buffer is completely copied to srce buffer at end of update.
    filterBuffers.SetTranLerpFactor(filterBuffers.GetMaxTranLerpFactor());
    REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());

    constantZoomVector.SetConstCoords(CONST_ZOOM_VECTOR_COORDS_2);
    REQUIRE(CONST_ZOOM_VECTOR_COORDS_2 == constantZoomVector.GetConstCoords());
    REQUIRE(
        NML_CONST_ZOOM_VECTOR_COORDS_2.Equals(constantZoomVector.GetZoomPoint(DUMMY_NML_COORDS)));
    filterBuffers.NotifyFilterSettingsHaveChanged();
    REQUIRE(filterBuffers.HaveFilterSettingsChanged());
    filterBuffers.UpdateTranBuffers();
    REQUIRE(ZoomFilterBuffers::TranBuffersState::TRAN_BUFFERS_READY ==
            filterBuffers.GetTranBuffersState());
    while (true)
    {
      filterBuffers.UpdateTranBuffers();
      if (filterBuffers.GetTranBuffYLineStart() == 0)
      {
        break;
      }
    }
    REQUIRE(ZoomFilterBuffers::TranBuffersState::RESET_TRAN_BUFFERS ==
            filterBuffers.GetTranBuffersState());
    REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());

    filterBuffers.UpdateTranBuffers();
    REQUIRE(ZoomFilterBuffers::TranBuffersState::START_FRESH_TRAN_BUFFERS ==
            filterBuffers.GetTranBuffersState());
    REQUIRE(0 == filterBuffers.GetTranLerpFactor());
    REQUIRE(0 == filterBuffers.GetTranBuffYLineStart());
    REQUIRE(CONST_ZOOM_VECTOR_COORDS_2 == constantZoomVector.GetConstCoords());
    REQUIRE(MID_PT == filterBuffers.GetBuffMidpoint());

    const NormalizedCoords normalizedMidPt{
        NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(filterBuffers.GetBuffMidpoint())};

    // Get srce buffer points - should be all CONST_ZOOM_VECTOR_COORDS_1
    filterBuffers.SetTranLerpFactor(0);
    REQUIRE(0 == filterBuffers.GetTranLerpFactor());
    const Point2dInt expectedSrceTranPoint = COORD_TRANSFORMS.NormalizedToTranPoint(
        normalizedMidPt +
        NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1));
    const Point2dInt expectedSrcePoint = CoordTransforms::TranToScreenPoint(expectedSrceTranPoint);
    UNSCOPED_INFO("expectedSrceTranPoint.x = " << expectedSrceTranPoint.x);
    UNSCOPED_INFO("expectedSrceTranPoint.y = " << expectedSrceTranPoint.y);
    UNSCOPED_INFO("expectedSrcePoint.x = " << expectedSrcePoint.x);
    UNSCOPED_INFO("expectedSrcePoint.y = " << expectedSrcePoint.y);

    for (size_t buffPos = 0; buffPos < WIDTH * HEIGHT; buffPos++)
    {
      const Point2dInt srcePoint = GetSourcePoint(filterBuffers, GetBuffPos(TEST_X, TEST_Y));
      UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
      UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);

      REQUIRE(expectedSrcePoint == srcePoint);
    }

    // Get dest buffer points - should be all CONST_ZOOM_VECTOR_COORDS_2
    filterBuffers.SetTranLerpFactor(filterBuffers.GetMaxTranLerpFactor());
    REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());
    const Point2dInt expectedDestTranPoint =
        COORD_TRANSFORMS.NormalizedToTranPoint(normalizedMidPt + NML_CONST_ZOOM_VECTOR_COORDS_2);
    const Point2dInt expectedDestPoint = CoordTransforms::TranToScreenPoint(expectedDestTranPoint);
    UNSCOPED_INFO("normalizedMidPt.x = " << normalizedMidPt.GetX());
    UNSCOPED_INFO("normalizedMidPt.y = " << normalizedMidPt.GetY());
    UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_2.x = " << NML_CONST_ZOOM_VECTOR_COORDS_2.GetX());
    UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_2.y = " << NML_CONST_ZOOM_VECTOR_COORDS_2.GetY());
    UNSCOPED_INFO("expectedDestTranPoint.x = " << expectedDestTranPoint.x);
    UNSCOPED_INFO("expectedDestTranPoint.y = " << expectedDestTranPoint.y);
    UNSCOPED_INFO("expectedDestPoint.x = " << expectedDestPoint.x);
    UNSCOPED_INFO("expectedDestPoint.y = " << expectedDestPoint.y);

    for (size_t buffPos = 0; buffPos < WIDTH * HEIGHT; buffPos++)
    {
      const Point2dInt destPoint = GetSourcePoint(filterBuffers, buffPos);
      UNSCOPED_INFO("destPoint.x = " << destPoint.x);
      UNSCOPED_INFO("destPoint.y = " << destPoint.y);

      REQUIRE(expectedDestPoint == destPoint);
    }
  }
}

TEST_CASE("ZoomFilterBuffers Clipping", "[ZoomFilterBuffersClipping]")
{
  Parallel parallel{-1};
  SoundInfo soundInfo{};
  GoomSoundEvents goomSoundEvents{soundInfo};

  const PluginInfo goomInfo{{WIDTH, HEIGHT}, goomSoundEvents};
  TestZoomVector constantZoomVector{true};
  ZoomFilterBuffers filterBuffers{parallel, goomInfo, NORMALIZED_COORDS_CONVERTER,
                                  [&](const NormalizedCoords& normalizedCoords)
                                  { return constantZoomVector.GetZoomPoint(normalizedCoords); }};

  filterBuffers.SetBuffMidpoint({0, 0});
  filterBuffers.Start();

  const NormalizedCoords DUMMY_NML_COORDS{
      NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(DUMMY_COORDS)};
  const NormalizedCoords NML_CONST_ZOOM_VECTOR_COORDS_1{
      NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1)};

  SECTION("Clipped ZoomBufferTranPoint")
  {
    REQUIRE(Point2dInt{0, 0} == filterBuffers.GetBuffMidpoint());

    // Lerp to the dest buffer only
    filterBuffers.SetTranLerpFactor(filterBuffers.GetMaxTranLerpFactor());
    REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());

    // tranPoint comes solely from the dest Zoom buffer which because we are using a
    // const ZoomVectorFunc, returns a const normalized value
    UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_1.x = " << NML_CONST_ZOOM_VECTOR_COORDS_1.GetX());
    UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_1.y = " << NML_CONST_ZOOM_VECTOR_COORDS_1.GetY());
    UNSCOPED_INFO("GetZoomPoint(DUMMY_NML_COORDS).x = "
                  << constantZoomVector.GetZoomPoint(DUMMY_NML_COORDS).GetX());
    UNSCOPED_INFO("GetZoomPoint(DUMMY_NML_COORDS).y = "
                  << constantZoomVector.GetZoomPoint(DUMMY_NML_COORDS).GetY());
    REQUIRE(
        NML_CONST_ZOOM_VECTOR_COORDS_1.Equals(constantZoomVector.GetZoomPoint(DUMMY_NML_COORDS)));
    const NormalizedCoords normalizedMidPt{
        NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(filterBuffers.GetBuffMidpoint())};
    const Point2dInt expectedTranPoint = COORD_TRANSFORMS.NormalizedToTranPoint(
        normalizedMidPt +
        NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1));
    // Because mid-point is zero, the trans point is negative and therefore clipped.
    REQUIRE(expectedTranPoint.x < 0);
    REQUIRE(expectedTranPoint.y < 0);

    UNSCOPED_INFO("filterBuffers.GetBuffMidpoint().x = " << filterBuffers.GetBuffMidpoint().x);
    UNSCOPED_INFO("filterBuffers.GetBuffMidpoint().y = " << filterBuffers.GetBuffMidpoint().y);
    UNSCOPED_INFO("normalizedMidPt.x = " << normalizedMidPt.GetX());
    UNSCOPED_INFO("normalizedMidPt.y = " << normalizedMidPt.GetY());
    UNSCOPED_INFO("expectedTranPoint.x = " << expectedTranPoint.x);
    UNSCOPED_INFO("expectedTranPoint.y = " << expectedTranPoint.y);

    const ZoomFilterBuffers::SourcePointInfo destPoint = filterBuffers.GetSourcePointInfo(0);
    UNSCOPED_INFO("destPoint.isClipped = " << destPoint.isClipped);
    UNSCOPED_INFO("destPoint.screenPoint.x = " << destPoint.screenPoint.x);
    UNSCOPED_INFO("destPoint.screenPoint.y = " << destPoint.screenPoint.y);
    REQUIRE(destPoint.isClipped);
    REQUIRE(0 == destPoint.screenPoint.x);
    REQUIRE(0 == destPoint.screenPoint.y);

    // TODO Test coeff values
  }
}

} // namespace GOOM::UNIT_TESTS
