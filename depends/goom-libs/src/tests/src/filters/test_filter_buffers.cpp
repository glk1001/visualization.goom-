#define GOOM_DEBUG

#include "catch2/catch.hpp"
#include "goom_plugin_info.h"
#include "utils/mathutils.h"
#include "utils/parallel_utils.h"
#include "v2d.h"
#include "visual_fx/filters/filter_buffers.h"
#include "visual_fx/filters/filter_settings.h"
#include "visual_fx/filters/filter_zoom_vector.h"
#include "visual_fx/filters/normalized_coords.h"

#include <cmath>

using GOOM::PluginInfo;
using GOOM::V2dInt;
using GOOM::FILTERS::FilterZoomVector;
using GOOM::FILTERS::NormalizedCoords;
using GOOM::FILTERS::ZoomFilterBuffers;
using GOOM::FILTERS::ZoomFilterEffectsSettings;
using GOOM::UTILS::floats_equal;
using GOOM::UTILS::Parallel;

constexpr size_t WIDTH = 120;
constexpr size_t HEIGHT = 70;
constexpr const char* RESOURCES_DIRECTORY = "";

const V2dInt MID_PT = {static_cast<int32_t>(WIDTH) / 2, static_cast<int32_t>(HEIGHT) / 2};
const V2dInt CONST_ZOOM_VECTOR_COORDS_1 = {16, 40};
const V2dInt CONST_ZOOM_VECTOR_COORDS_2 = {32, 52};
const V2dInt DUMMY_COORDS = {14, 38};

inline auto GetBuffPos(const int32_t x, const int32_t y) -> size_t
{
  return (static_cast<size_t>(y) * WIDTH) + static_cast<size_t>(x);
}

class TestZoomVector : public FilterZoomVector
{
public:
  explicit TestZoomVector(const bool returnConst) noexcept
    : FilterZoomVector{WIDTH, RESOURCES_DIRECTORY}, m_ReturnConst{returnConst}
  {
  }

  void SetFilterSettings(const ZoomFilterEffectsSettings& filterEffectsSettings) override;

  [[nodiscard]] auto GetConstCoords() const -> const V2dInt& { return m_constCoords; }
  void SetConstCoords(const V2dInt& coords) { m_constCoords = coords; }

  [[nodiscard]] auto GetZoomPoint(const NormalizedCoords& coords) const
      -> NormalizedCoords override;

private:
  const bool m_ReturnConst;
  V2dInt m_constCoords = CONST_ZOOM_VECTOR_COORDS_1;
};

void TestZoomVector::SetFilterSettings(const ZoomFilterEffectsSettings& filterEffectsSettings)
{
  FilterZoomVector::SetFilterSettings(filterEffectsSettings);
}

auto TestZoomVector::GetZoomPoint(const NormalizedCoords& coords) const -> NormalizedCoords
{
  if (m_ReturnConst)
  {
    return NormalizedCoords{m_constCoords};
  }
  return coords;
}

using CoordTransforms = ZoomFilterBuffers::CoordTransforms;

auto GetSourcePoint(const ZoomFilterBuffers& filterBuffers, const size_t buffPos) -> V2dInt
{
  return filterBuffers.GetSourcePointInfo(buffPos).screenPoint;
}

TEST_CASE("ZoomFilterBuffers Basic", "[ZoomFilterBuffers]")
{
  constexpr int32_t TEST_X = 10;
  constexpr int32_t TEST_Y = 50;
  const V2dInt TEST_SRCE_POINT = {TEST_X, TEST_Y};
  static_assert((0 <= TEST_X) && (TEST_X < WIDTH), "Invalid X");
  static_assert((0 <= TEST_Y) && (TEST_Y < WIDTH), "Invalid Y");

  Parallel parallel{-1};
  const PluginInfo goomInfo{WIDTH, HEIGHT};
  TestZoomVector identityZoomVector{false};
  ZoomFilterBuffers filterBuffers{parallel, goomInfo,
                                  [&](const NormalizedCoords& normalizedCoords) {
                                    return identityZoomVector.GetZoomPoint(normalizedCoords);
                                  }};

  filterBuffers.SetBuffMidPoint(MID_PT);
  filterBuffers.Start();

  const NormalizedCoords DUMMY_NML_COORDS{DUMMY_COORDS};

  SECTION("Correct Starting TranBuffersState")
  {
    REQUIRE(ZoomFilterBuffers::TranBuffersState::START_FRESH_TRAN_BUFFERS ==
            filterBuffers.GetTranBuffersState());
  }
  SECTION("Correct Starting BuffYLineStart")
  {
    REQUIRE(0 == filterBuffers.GetTranBuffYLineStart());
  }
  SECTION("Correct Starting BuffMidPoint()") { REQUIRE(MID_PT == filterBuffers.GetBuffMidPoint()); }
  SECTION("Correct Starting HaveFilterSettingsChanged")
  {
    REQUIRE(!filterBuffers.HaveFilterSettingsChanged());
  }
  SECTION("Correct Starting TranLerpFactor") { REQUIRE(0 == filterBuffers.GetTranLerpFactor()); }
  SECTION("Correct Starting ZoomBufferTranPoint")
  {
    REQUIRE(DUMMY_NML_COORDS.equals(identityZoomVector.GetZoomPoint(DUMMY_NML_COORDS)));

    // TranLerpFactor is zero so starting tranPoint should be srce identity buffer
    const V2dInt expectedSrceIdentityPoint = TEST_SRCE_POINT;
    const V2dInt srcePoint = GetSourcePoint(filterBuffers, GetBuffPos(TEST_X, TEST_Y));
    UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
    UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);
    REQUIRE(expectedSrceIdentityPoint == srcePoint);
  }
  SECTION("Correct Dest ZoomBufferTranPoint")
  {
    // Lerp to the dest buffer only
    filterBuffers.SetTranLerpFactor(filterBuffers.GetMaxTranLerpFactor());
    REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());

    REQUIRE(DUMMY_NML_COORDS.equals(identityZoomVector.GetZoomPoint(DUMMY_NML_COORDS)));

    // tranPoint comes solely from the dest Zoom buffer which because we are using an
    // identity ZoomVectorFunc should be the identity point.
    const V2dInt expectedSrceIdentityPoint = TEST_SRCE_POINT;
    const V2dInt srcePoint = GetSourcePoint(filterBuffers, GetBuffPos(TEST_X, TEST_Y));
    UNSCOPED_INFO("expectedSrceIdentityPoint.x = " << expectedSrceIdentityPoint.x);
    UNSCOPED_INFO("expectedSrceIdentityPoint.y = " << expectedSrceIdentityPoint.y);
    UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
    UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);
    REQUIRE(expectedSrceIdentityPoint == srcePoint);
  }
  SECTION("Correct Lerped ZoomBufferTranPoint")
  {
    constexpr float tLerp = 0.5F;
    const auto tranLerpFactor = static_cast<int32_t>(
        std::round(tLerp * static_cast<float>(filterBuffers.GetMaxTranLerpFactor())));

    filterBuffers.SetTranLerpFactor(tranLerpFactor);
    REQUIRE(tranLerpFactor == filterBuffers.GetTranLerpFactor());

    REQUIRE(DUMMY_NML_COORDS.equals(identityZoomVector.GetZoomPoint(DUMMY_NML_COORDS)));

    // tranPoint comes solely from the lerp middle of srce and dest Zoom buffer which
    // because we are using an identity ZoomVectorFunc should still be the identity point.
    const V2dInt expectedSrceIdentityPoint = TEST_SRCE_POINT;
    const V2dInt srcePoint = GetSourcePoint(filterBuffers, GetBuffPos(TEST_X, TEST_Y));
    UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
    UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);
    REQUIRE(expectedSrceIdentityPoint == srcePoint);
  }
}

TEST_CASE("ZoomFilterBuffers Calculations", "[ZoomFilterBuffersCalcs]")
{
  Parallel parallel{-1};
  const PluginInfo goomInfo{WIDTH, HEIGHT};
  TestZoomVector constantZoomVector{true};
  ZoomFilterBuffers filterBuffers{parallel, goomInfo,
                                  [&](const NormalizedCoords& normalizedCoords) {
                                    return constantZoomVector.GetZoomPoint(normalizedCoords);
                                  }};

  filterBuffers.SetBuffMidPoint(MID_PT);
  filterBuffers.Start();

  const NormalizedCoords DUMMY_NML_COORDS{DUMMY_COORDS};
  const NormalizedCoords NML_CONST_ZOOM_VECTOR_COORDS_1{CONST_ZOOM_VECTOR_COORDS_1};

  SECTION("Correct Dest ZoomBufferTranPoint")
  {
    REQUIRE(CONST_ZOOM_VECTOR_COORDS_1 == constantZoomVector.GetConstCoords());
    REQUIRE(MID_PT == filterBuffers.GetBuffMidPoint());

    // Lerp to the dest buffer only
    filterBuffers.SetTranLerpFactor(filterBuffers.GetMaxTranLerpFactor());
    REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());

    // tranPoint comes solely from the dest Zoom buffer which because we are using a
    // const ZoomVectorFunc, returns a const normalized value
    UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_1.x = " << NML_CONST_ZOOM_VECTOR_COORDS_1.GetX());
    UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_1.y = " << NML_CONST_ZOOM_VECTOR_COORDS_1.GetY());
    UNSCOPED_INFO("GetZoomPoint(DUMMY_NML_COORDS).x = " << constantZoomVector.GetZoomPoint(DUMMY_NML_COORDS).GetX());
    UNSCOPED_INFO("GetZoomPoint(DUMMY_NML_COORDS).y = " << constantZoomVector.GetZoomPoint(DUMMY_NML_COORDS).GetY());
    REQUIRE(NML_CONST_ZOOM_VECTOR_COORDS_1.equals(constantZoomVector.GetZoomPoint(DUMMY_NML_COORDS)));
    const NormalizedCoords normalizedMidPt{filterBuffers.GetBuffMidPoint()};
    const V2dInt expectedTranPoint = CoordTransforms::NormalizedToTranPoint(
        normalizedMidPt + NormalizedCoords{CONST_ZOOM_VECTOR_COORDS_1});
    UNSCOPED_INFO("filterBuffers.GetBuffMidPoint().x = " << filterBuffers.GetBuffMidPoint().x);
    UNSCOPED_INFO("filterBuffers.GetBuffMidPoint().y = " << filterBuffers.GetBuffMidPoint().y);
    UNSCOPED_INFO("normalizedMidPt.x = " << normalizedMidPt.GetX());
    UNSCOPED_INFO("normalizedMidPt.y = " << normalizedMidPt.GetY());
    UNSCOPED_INFO("expectedTranPoint.x = " << expectedTranPoint.x);
    UNSCOPED_INFO("expectedTranPoint.y = " << expectedTranPoint.y);

    const V2dInt expectedSrcePoint = CoordTransforms::TranToScreenPoint(expectedTranPoint);
    UNSCOPED_INFO("expectedSrcePoint.x = " << expectedSrcePoint.x);
    UNSCOPED_INFO("expectedSrcePoint.y = " << expectedSrcePoint.y);

    for (size_t buffPos = 0; buffPos < WIDTH * HEIGHT; buffPos++)
    {
      const V2dInt srcePoint = GetSourcePoint(filterBuffers, buffPos);
      UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
      UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);

      REQUIRE(expectedSrcePoint == srcePoint);
    }
  }

  SECTION("Correct Srce/Dest ZoomBufferTranPoint")
  {
    REQUIRE(CONST_ZOOM_VECTOR_COORDS_1 == constantZoomVector.GetConstCoords());
    REQUIRE(MID_PT == filterBuffers.GetBuffMidPoint());

    constexpr float tLerp = 0.5F;
    const auto tranLerpFactor = static_cast<int32_t>(
        std::round(tLerp * static_cast<float>(filterBuffers.GetMaxTranLerpFactor())));

    filterBuffers.SetTranLerpFactor(tranLerpFactor);
    REQUIRE(tranLerpFactor == filterBuffers.GetTranLerpFactor());

    for (int32_t y = 0; y < static_cast<int32_t>(HEIGHT); y++)
    {
      for (int32_t x = 0; x < static_cast<int32_t>(WIDTH); x++)
      {
        // tranPoint comes from halfway between srce and dest Zoom buffer.
        REQUIRE(NML_CONST_ZOOM_VECTOR_COORDS_1.equals(
            constantZoomVector.GetZoomPoint(DUMMY_NML_COORDS)));
        const NormalizedCoords normalizedSrcePt{V2dInt{x, y}};
        const V2dInt expectedSrceTranPoint =
            CoordTransforms::NormalizedToTranPoint(normalizedSrcePt);
        const NormalizedCoords normalizedMidPt{filterBuffers.GetBuffMidPoint()};
        UNSCOPED_INFO("filterBuffers.GetBuffMidPoint().x = " << filterBuffers.GetBuffMidPoint().x);
        UNSCOPED_INFO("filterBuffers.GetBuffMidPoint().y = " << filterBuffers.GetBuffMidPoint().y);
        UNSCOPED_INFO("normalizedMidPt.x = " << normalizedMidPt.GetX());
        UNSCOPED_INFO("normalizedMidPt.y = " << normalizedMidPt.GetY());
        const V2dInt expectedDestTranPoint = CoordTransforms::NormalizedToTranPoint(
            normalizedMidPt + NormalizedCoords{CONST_ZOOM_VECTOR_COORDS_1});
        const V2dInt expectedTranPoint = {
            stdnew::lerp(expectedSrceTranPoint.x, expectedDestTranPoint.x, tLerp),
            stdnew::lerp(expectedSrceTranPoint.y, expectedDestTranPoint.y, tLerp)};
        UNSCOPED_INFO("expectedTranPoint.x = " << expectedTranPoint.x);
        UNSCOPED_INFO("expectedTranPoint.y = " << expectedTranPoint.y);

        const V2dInt expectedScreenPoint = CoordTransforms::TranToScreenPoint(expectedTranPoint);
        UNSCOPED_INFO("expectedScreenPoint.x = " << expectedScreenPoint.x);
        UNSCOPED_INFO("expectedScreenPoint.y = " << expectedScreenPoint.y);

        const V2dInt screenPoint = GetSourcePoint(filterBuffers, GetBuffPos(x, y));
        UNSCOPED_INFO("srcePoint.x = " << screenPoint.x);
        UNSCOPED_INFO("srcePoint.y = " << screenPoint.y);

        REQUIRE(expectedScreenPoint == screenPoint);
      }
    }
  }
}

TEST_CASE("ZoomFilterBuffers Stripes", "[ZoomFilterBuffersStripes]")
{
  constexpr int32_t TEST_X = 10;
  constexpr int32_t TEST_Y = 50;
  static_assert((0 <= TEST_X) && (TEST_X < WIDTH), "Invalid X");
  static_assert((0 <= TEST_Y) && (TEST_Y < WIDTH), "Invalid Y");

  Parallel parallel{0};
  const PluginInfo goomInfo{WIDTH, HEIGHT};
  TestZoomVector constantZoomVector{true};
  ZoomFilterBuffers filterBuffers{parallel, goomInfo,
                                  [&](const NormalizedCoords& normalizedCoords) {
                                    return constantZoomVector.GetZoomPoint(normalizedCoords);
                                  }};

  filterBuffers.SetBuffMidPoint(MID_PT);
  filterBuffers.Start();

  const NormalizedCoords DUMMY_NML_COORDS{DUMMY_COORDS};
  const NormalizedCoords NML_CONST_ZOOM_VECTOR_COORDS_1{CONST_ZOOM_VECTOR_COORDS_1};
  const NormalizedCoords NML_CONST_ZOOM_VECTOR_COORDS_2{CONST_ZOOM_VECTOR_COORDS_2};

  SECTION("ZoomBuffer Stripe")
  {
    REQUIRE(ZoomFilterBuffers::TranBuffersState::START_FRESH_TRAN_BUFFERS ==
            filterBuffers.GetTranBuffersState());
    REQUIRE(!filterBuffers.HaveFilterSettingsChanged());

    REQUIRE(CONST_ZOOM_VECTOR_COORDS_1 == constantZoomVector.GetConstCoords());
    REQUIRE(MID_PT == filterBuffers.GetBuffMidPoint());
    REQUIRE(NML_CONST_ZOOM_VECTOR_COORDS_1.equals(constantZoomVector.GetZoomPoint(DUMMY_NML_COORDS)));

    // Make sure dest buffer is completely copied to srce buffer at end of update.
    filterBuffers.SetTranLerpFactor(filterBuffers.GetMaxTranLerpFactor());
    REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());

    constantZoomVector.SetConstCoords(CONST_ZOOM_VECTOR_COORDS_2);
    REQUIRE(CONST_ZOOM_VECTOR_COORDS_2 == constantZoomVector.GetConstCoords());
    REQUIRE(NML_CONST_ZOOM_VECTOR_COORDS_2.equals(constantZoomVector.GetZoomPoint(DUMMY_NML_COORDS)));
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
    REQUIRE(MID_PT == filterBuffers.GetBuffMidPoint());

    const NormalizedCoords normalizedMidPt{filterBuffers.GetBuffMidPoint()};

    // Get srce buffer points - should be all CONST_ZOOM_VECTOR_COORDS_1
    filterBuffers.SetTranLerpFactor(0);
    REQUIRE(0 == filterBuffers.GetTranLerpFactor());
    const V2dInt expectedSrceTranPoint = CoordTransforms::NormalizedToTranPoint(
        normalizedMidPt + NormalizedCoords{CONST_ZOOM_VECTOR_COORDS_1});
    const V2dInt expectedSrcePoint = CoordTransforms::TranToScreenPoint(expectedSrceTranPoint);
    UNSCOPED_INFO("expectedSrceTranPoint.x = " << expectedSrceTranPoint.x);
    UNSCOPED_INFO("expectedSrceTranPoint.y = " << expectedSrceTranPoint.y);
    UNSCOPED_INFO("expectedSrcePoint.x = " << expectedSrcePoint.x);
    UNSCOPED_INFO("expectedSrcePoint.y = " << expectedSrcePoint.y);

    for (size_t buffPos = 0; buffPos < WIDTH * HEIGHT; buffPos++)
    {
      const V2dInt srcePoint = GetSourcePoint(filterBuffers, GetBuffPos(TEST_X, TEST_Y));
      UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
      UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);

      REQUIRE(expectedSrcePoint == srcePoint);
    }

    // Get dest buffer points - should be all CONST_ZOOM_VECTOR_COORDS_2
    filterBuffers.SetTranLerpFactor(filterBuffers.GetMaxTranLerpFactor());
    REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());
    const V2dInt expectedDestTranPoint = CoordTransforms::NormalizedToTranPoint(
        normalizedMidPt + NML_CONST_ZOOM_VECTOR_COORDS_2);
    const V2dInt expectedDestPoint = CoordTransforms::TranToScreenPoint(expectedDestTranPoint);
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
      const V2dInt destPoint = GetSourcePoint(filterBuffers, buffPos);
      UNSCOPED_INFO("destPoint.x = " << destPoint.x);
      UNSCOPED_INFO("destPoint.y = " << destPoint.y);

      REQUIRE(expectedDestPoint == destPoint);
    }
  }
}

TEST_CASE("ZoomFilterBuffers Clipping", "[ZoomFilterBuffersClipping]")
{
  Parallel parallel{-1};
  const PluginInfo goomInfo{WIDTH, HEIGHT};
  TestZoomVector constantZoomVector{true};
  ZoomFilterBuffers filterBuffers{parallel, goomInfo,
                                  [&](const NormalizedCoords& normalizedCoords) {
                                    return constantZoomVector.GetZoomPoint(normalizedCoords);
                                  }};

  filterBuffers.SetBuffMidPoint({0, 0});
  filterBuffers.Start();

  const NormalizedCoords DUMMY_NML_COORDS{DUMMY_COORDS};
  const NormalizedCoords NML_CONST_ZOOM_VECTOR_COORDS_1{CONST_ZOOM_VECTOR_COORDS_1};

  SECTION("Clipped ZoomBufferTranPoint")
  {
    REQUIRE(V2dInt{0, 0} == filterBuffers.GetBuffMidPoint());

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
        NML_CONST_ZOOM_VECTOR_COORDS_1.equals(constantZoomVector.GetZoomPoint(DUMMY_NML_COORDS)));
    const NormalizedCoords normalizedMidPt{filterBuffers.GetBuffMidPoint()};
    const V2dInt expectedTranPoint = CoordTransforms::NormalizedToTranPoint(
        normalizedMidPt + NormalizedCoords{CONST_ZOOM_VECTOR_COORDS_1});
    // Because mid point is zero, the trans point is negative and therefore clipped.
    REQUIRE(expectedTranPoint.x < 0);
    REQUIRE(expectedTranPoint.y < 0);

    UNSCOPED_INFO("filterBuffers.GetBuffMidPoint().x = " << filterBuffers.GetBuffMidPoint().x);
    UNSCOPED_INFO("filterBuffers.GetBuffMidPoint().y = " << filterBuffers.GetBuffMidPoint().y);
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