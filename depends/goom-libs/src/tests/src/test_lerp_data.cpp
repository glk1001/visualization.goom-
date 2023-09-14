#ifndef GOOM_DEBUG
#define GOOM_DEBUG
#endif

#include "goom/goom_config.h"
#include "goom/goom_lerp_data.h"
#include "goom/math20.h"

#if __clang_major__ >= 16 // NOLINT: Can't include header for this.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wunsafe-buffer-usage"
#endif
#include <catch2/catch_approx.hpp>
#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#if __clang_major__ >= 16 // NOLINT: Can't include header for this.
#pragma GCC diagnostic pop
#endif

#include <cstdint>

namespace GOOM::UNIT_TESTS
{

using Catch::Approx;

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

// NOLINTBEGIN(readability-function-cognitive-complexity)

TEST_CASE("LerpData Simple")
{
  auto lerpData = GoomLerpData{};

  REQUIRE(lerpData.GetLerpFactor() == Approx(0.0F));
  REQUIRE(lerpData.GetIncrement() == Approx(0.0F));
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
  REQUIRE(lerpData.GetIncrement() == Approx(0.0F));
}

TEST_CASE("LerpData More Complicated")
{
  auto lerpData = GoomLerpData{0.0F, true};
  REQUIRE(lerpData.GetIncrement() == Approx(0.0F));

  UNSCOPED_INFO("GetSFuncValue = " << GoomLerpData::GetSFuncValue(0.0F));
  UNSCOPED_INFO("lerpFactor = " << STD20::lerp(0.0F, 1.0F, GoomLerpData::GetSFuncValue(0.0F)));

  lerpData.Update();
  const auto sFunctionTIncrement = lerpData.GetSFunctionTIncrement();
  UNSCOPED_INFO("sFunctionTIncrement = " << sFunctionTIncrement);
  UNSCOPED_INFO("GetSFuncValue = " << GoomLerpData::GetSFuncValue(sFunctionTIncrement));
  UNSCOPED_INFO(
      "lerpFactor = " << STD20::lerp(0.0F, 1.0F, GoomLerpData::GetSFuncValue(sFunctionTIncrement)));
  REQUIRE(lerpData.GetLerpFactor() == Approx(GoomLerpData::GetSFuncValue(sFunctionTIncrement)));
  REQUIRE(lerpData.GetUseSFunction());

  static constexpr auto TEST_INCREMENT = 0.1F;
  lerpData.SetIncrement(TEST_INCREMENT);
  lerpData.Update();
  REQUIRE(lerpData.GetLerpFactor() ==
          Approx(TEST_INCREMENT +
                 GoomLerpData::GetSFuncValue(sFunctionTIncrement + sFunctionTIncrement)));

  lerpData.Update();
  REQUIRE(lerpData.GetLerpFactor() ==
          Approx(TEST_INCREMENT + TEST_INCREMENT +
                 GoomLerpData::GetSFuncValue(sFunctionTIncrement + sFunctionTIncrement +
                                             sFunctionTIncrement)));

  lerpData.SetIncrement(0.0F);
  static constexpr auto NUM_TO_GET_TO_ONE = 265U;
  UpdateLerpData(lerpData, NUM_TO_GET_TO_ONE);
  REQUIRE(lerpData.GetLerpFactor() == Approx(1.0F));

  lerpData.Reset();
  REQUIRE(lerpData.GetIncrement() == Approx(0.0F));
  REQUIRE(lerpData.GetLerpFactor() == Approx(0.0F));

  lerpData.SetLerpToEnd();
  lerpData.Update();
  REQUIRE(lerpData.GetLerpFactor() == Approx(1.0F));
}

TEST_CASE("LerpData Negative Increment")
{
  auto lerpData = GoomLerpData{};

  REQUIRE(lerpData.GetLerpFactor() == Approx(0.0F));
  REQUIRE(lerpData.GetIncrement() == Approx(0.0F));
  REQUIRE(not lerpData.GetUseSFunction());

  lerpData.Reset();
  static constexpr auto TEST_INCREMENT          = 0.1F;
  static constexpr auto TEST_NEGATIVE_INCREMENT = -TEST_INCREMENT;

  static constexpr auto NUM_INCS_TO_ONE = static_cast<uint32_t>(1.0F / TEST_INCREMENT);
  lerpData.SetIncrement(TEST_INCREMENT);
  UpdateLerpData(lerpData, NUM_INCS_TO_ONE);
  REQUIRE(lerpData.GetLerpFactor() == Approx(1.0F));

  lerpData.SetIncrement(TEST_NEGATIVE_INCREMENT);
  lerpData.Update();
  REQUIRE(lerpData.GetLerpFactor() == Approx(1.0F + TEST_NEGATIVE_INCREMENT));

  static constexpr auto NUM_INCS_TO_START =
      static_cast<uint32_t>(1.0F / (-TEST_NEGATIVE_INCREMENT)) - 2U;
  UpdateLerpData(lerpData, NUM_INCS_TO_START);
  REQUIRE(lerpData.GetLerpFactor() == Approx(-TEST_NEGATIVE_INCREMENT));

  lerpData.Update();
  REQUIRE(lerpData.GetLerpFactor() == Approx(0.0F));
  lerpData.Update();
  REQUIRE(lerpData.GetLerpFactor() == Approx(0.0F));
}

// NOLINTEND(readability-function-cognitive-complexity)

} // namespace GOOM::UNIT_TESTS
