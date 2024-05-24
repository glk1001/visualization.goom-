#ifndef GOOM_DEBUG
#define GOOM_DEBUG
#endif

#include "goom/goom_config.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <cstdint>

import Goom.FilterFx.FilterUtils.GoomLerpData;

namespace GOOM::UNIT_TESTS
{

using Catch::Approx;
using GOOM::FILTER_FX::FILTER_UTILS::GoomLerpData;

namespace
{

auto UpdateLerpData(GoomLerpData& lerpData, const uint32_t num)
{
  for (auto i = 0U; i < num; ++i)
  {
    lerpData.Update();
  }
}

} // namespace

// NOLINTBEGIN(bugprone-chained-comparison): Catch2 needs to fix this.
// NOLINTBEGIN(readability-function-cognitive-complexity)

TEST_CASE("LerpData Simple")
{
  auto lerpData = GoomLerpData{};

  REQUIRE(lerpData.GetLerpFactor() == Approx(0.0F));
  REQUIRE(lerpData.GetIncrement() == Approx(GoomLerpData::DEFAULT_INCREMENT));
  REQUIRE(not lerpData.GetUseSFunction());

  static constexpr auto TEST_INCREMENT = 0.1F;
  lerpData.SetIncrement(TEST_INCREMENT);
  lerpData.Update();
  REQUIRE(lerpData.GetIncrement() == Approx(TEST_INCREMENT));
  REQUIRE(lerpData.GetLerpFactor() == Approx(TEST_INCREMENT));
  lerpData.Update();
  REQUIRE(lerpData.GetIncrement() == Approx(TEST_INCREMENT));
  REQUIRE(lerpData.GetLerpFactor() == Approx(TEST_INCREMENT + TEST_INCREMENT));

  lerpData.Reset();
  REQUIRE(lerpData.GetLerpFactor() == Approx(0.0F));
  REQUIRE(lerpData.GetIncrement() == Approx(GoomLerpData::DEFAULT_INCREMENT));
}

TEST_CASE("LerpData More Complicated")
{
  auto lerpData = GoomLerpData{GoomLerpData::DEFAULT_INCREMENT, true};
  REQUIRE(lerpData.GetIncrement() == Approx(GoomLerpData::DEFAULT_INCREMENT));

  UNSCOPED_INFO("GetSFuncValue = " << GoomLerpData::GetSFuncValue(0.0F));
  UNSCOPED_INFO("lerpFactor = " << std::lerp(0.0F, 1.0F, GoomLerpData::GetSFuncValue(0.0F)));

  lerpData.Update();
  const auto sFunctionTIncrement = lerpData.GetSFunctionTIncrement();
  UNSCOPED_INFO("sFunctionTIncrement = " << sFunctionTIncrement);
  UNSCOPED_INFO("GetSFuncValue = " << GoomLerpData::GetSFuncValue(sFunctionTIncrement));
  UNSCOPED_INFO(
      "lerpFactor = " << std::lerp(0.0F, 1.0F, GoomLerpData::GetSFuncValue(sFunctionTIncrement)));
  REQUIRE(lerpData.GetLerpFactor() == Approx(GoomLerpData::GetSFuncValue(sFunctionTIncrement)));
  REQUIRE(lerpData.GetUseSFunction());

  static constexpr auto TEST_INCREMENT = 0.1F;
  lerpData.SetIncrement(TEST_INCREMENT);
  lerpData.Update();
  UNSCOPED_INFO("GetLerpFactor = " << lerpData.GetLerpFactor());
  REQUIRE(lerpData.GetLerpFactor() ==
          Approx(TEST_INCREMENT +
                 GoomLerpData::GetSFuncValue(sFunctionTIncrement + sFunctionTIncrement)));

  lerpData.Update();
  REQUIRE(lerpData.GetLerpFactor() ==
          Approx(TEST_INCREMENT + TEST_INCREMENT +
                 GoomLerpData::GetSFuncValue(sFunctionTIncrement + sFunctionTIncrement +
                                             sFunctionTIncrement)));

  lerpData.SetLerpToEnd();
  lerpData.Update();
  REQUIRE(lerpData.GetLerpFactor() == Approx(1.0F));
}

TEST_CASE("LerpData Negative Increment")
{
  auto lerpData                  = GoomLerpData{};
  const auto sFunctionTIncrement = lerpData.GetSFunctionTIncrement();

  REQUIRE(lerpData.GetLerpFactor() == Approx(0.0F));
  REQUIRE(lerpData.GetIncrement() == Approx(GoomLerpData::DEFAULT_INCREMENT));
  REQUIRE(not lerpData.GetUseSFunction());

  // Test S function going backwards.
  lerpData.SetUseSFunction(true);
  REQUIRE(lerpData.GetUseSFunction());

  lerpData.SetIncrement(GoomLerpData::DEFAULT_INCREMENT);
  lerpData.Reset();
  REQUIRE(lerpData.GetIncrement() == Approx(GoomLerpData::DEFAULT_INCREMENT));
  REQUIRE(lerpData.GetLerpFactor() == Approx(0.0F));

  auto numToGetToOne = static_cast<uint32_t>(std::round(1.0F / sFunctionTIncrement));
  UpdateLerpData(lerpData, numToGetToOne);
  UNSCOPED_INFO("GetLerpFactor = " << lerpData.GetLerpFactor());
  auto lerpAtMax = lerpData.GetLerpFactor();

  // Should now go backwards.
  lerpData.Update();
  UNSCOPED_INFO("GetLerpFactor = " << lerpData.GetLerpFactor());
  REQUIRE(lerpData.GetLerpFactor() < lerpAtMax);

  // Test increment going backwards.
  lerpData.SetUseSFunction(false);
  REQUIRE(not lerpData.GetUseSFunction());

  lerpData.Reset();
  REQUIRE(lerpData.GetIncrement() == Approx(GoomLerpData::DEFAULT_INCREMENT));
  REQUIRE(lerpData.GetLerpFactor() == Approx(0.0F));

  static constexpr auto TEST_INCREMENT = 0.1F;
  lerpData.SetIncrement(TEST_INCREMENT);
  REQUIRE(lerpData.GetIncrement() == Approx(TEST_INCREMENT));
  REQUIRE(lerpData.GetLerpFactor() == Approx(0.0F));

  numToGetToOne = static_cast<uint32_t>(std::round(1.0F / TEST_INCREMENT));
  UpdateLerpData(lerpData, numToGetToOne);
  UNSCOPED_INFO("GetLerpFactor = " << lerpData.GetLerpFactor());
  REQUIRE(lerpData.GetLerpFactor() == Approx(1.0F));

  // Should now go backwards.
  lerpData.Update();
  UNSCOPED_INFO("GetLerpFactor = " << lerpData.GetLerpFactor());
  REQUIRE(lerpData.GetLerpFactor() == Approx(1.0F - TEST_INCREMENT));
}

// NOLINTEND(readability-function-cognitive-complexity)
// NOLINTEND(bugprone-chained-comparison)

} // namespace GOOM::UNIT_TESTS
