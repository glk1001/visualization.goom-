#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <map>

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

import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRandBase;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.GoomTypes;

namespace GOOM::UNIT_TESTS
{

using Catch::Approx;
using GOOM::UnderlyingEnumType;
using UTILS::EnumToString;
using UTILS::NUM;
using UTILS::MATH::ConditionalWeights;
using UTILS::MATH::GoomRand;
using UTILS::MATH::Weights;

namespace
{

enum class Events : UnderlyingEnumType
{
  EVENT1,
  EVENT2,
  EVENT3,
  EVENT4,
};
using EventCounts             = std::array<uint32_t, NUM<Events>>;
constexpr auto PREVIOUS_EVENT = Events::EVENT3;

constexpr size_t NUM_LOOPS     = 10000000;
constexpr double DBL_NUM_LOOPS = NUM_LOOPS;

[[nodiscard]] auto GetWeightedCounts(const Weights<Events>& weights) -> EventCounts
{
  EventCounts eventCounts{};
  for (auto i = 0U; i < NUM_LOOPS; ++i)
  {
    const auto event = weights.GetRandomWeighted();
    ++eventCounts.at(static_cast<size_t>(event));
  }

  return eventCounts;
}

[[nodiscard]] auto GetConditionalWeightedCounts(const ConditionalWeights<Events>& weights)
    -> EventCounts
{
  EventCounts eventCounts{};
  for (auto i = 0U; i < NUM_LOOPS; ++i)
  {
    const auto event = weights.GetRandomWeighted(PREVIOUS_EVENT);
    ++eventCounts.at(static_cast<size_t>(event));
  }

  return eventCounts;
}

} // namespace

// NOLINTBEGIN(bugprone-chained-comparison): Catch2 needs to fix this.
// NOLINTBEGIN(readability-function-cognitive-complexity)
TEST_CASE("Weighted Events")
{
  const auto goomRand    = GoomRand{};
  const auto weightPairs = Weights<Events>::EventWeightPairs{
      {Events::EVENT1, 05.0F},
      {Events::EVENT2, 02.0F},
      {Events::EVENT3, 10.0F},
      {Events::EVENT4, 06.0F},
  };
  const auto weightedEvents          = Weights<Events>{goomRand, weightPairs};
  static constexpr auto EXPECTED_SUM = 23.0;

  SECTION("Unconditional weights")
  {
    const auto sumOfWeights = static_cast<double>(weightedEvents.GetSumOfWeights());
    REQUIRE(sumOfWeights == Approx(EXPECTED_SUM));

    const auto eventCounts = GetWeightedCounts(weightedEvents);

    for (auto i = 0U; i < NUM<Events>; ++i)
    {
      const auto fEventCount = static_cast<double>(eventCounts.at(i));
      const auto fEventWeight =
          static_cast<double>(weightedEvents.GetWeight(static_cast<Events>(i)));

      const auto countFraction = fEventCount / DBL_NUM_LOOPS;
      const auto eventFraction = fEventWeight / sumOfWeights;

      UNSCOPED_INFO(std::format(
          "i = {}, countFraction = {}, eventFraction = {}", i, countFraction, eventFraction));
      static constexpr auto CLOSE_ENOUGH = 0.005;
      REQUIRE(countFraction == Approx(eventFraction).epsilon(CLOSE_ENOUGH));
    }
  }

  SECTION("Conditional weights")
  {
    static const auto s_EVENT3_WEIGHT_MULTIPLIERS = std::map<Events, float>{
        {Events::EVENT1, 1.0F},
        {Events::EVENT2, 0.0F},
        {Events::EVENT3, 2.0F},
        {Events::EVENT4, 1.0F},
    };
    static const auto s_WEIGHT_MULTIPLIERS = ConditionalWeights<Events>::EventWeightMultiplierPairs{
        {PREVIOUS_EVENT, s_EVENT3_WEIGHT_MULTIPLIERS}
    };
    const auto conditionalWeightedEvents =
        ConditionalWeights<Events>{goomRand, weightPairs, s_WEIGHT_MULTIPLIERS};
    const auto conditionalSumOfWeights =
        static_cast<double>(conditionalWeightedEvents.GetSumOfWeights(PREVIOUS_EVENT));
    static constexpr auto EXPECTED_SUM_FOR_GIVEN = 5.0 + 2.0 * 10.0 + 6.0;
    REQUIRE(conditionalSumOfWeights == Approx(EXPECTED_SUM_FOR_GIVEN));

    const auto conditionalEventCounts = GetConditionalWeightedCounts(conditionalWeightedEvents);

    for (auto i = 0U; i < NUM<Events>; ++i)
    {
      const auto fConditionalEventCount   = static_cast<double>(conditionalEventCounts.at(i));
      const auto conditionalCountFraction = fConditionalEventCount / DBL_NUM_LOOPS;

      UNSCOPED_INFO(std::format("i:{}, fConditionalEventCount = {}", i, fConditionalEventCount));
      UNSCOPED_INFO(std::format("i:{}, NUM_LOOPS = {}", i, NUM_LOOPS));
      UNSCOPED_INFO(
          std::format("i:{}, conditionalCountFraction = {}", i, conditionalCountFraction));

      const auto fConditionalEventWeight = static_cast<double>(
          conditionalWeightedEvents.GetWeight({PREVIOUS_EVENT, static_cast<Events>(i)}));
      const auto conditionalEventFraction = fConditionalEventWeight / conditionalSumOfWeights;
      UNSCOPED_INFO(std::format("i:{}, fConditionalEventWeight({}) = {}",
                                i,
                                EnumToString(PREVIOUS_EVENT),
                                fConditionalEventWeight));
      UNSCOPED_INFO(std::format("i:{}, conditionalSumOfWeights({}) = {}",
                                i,
                                EnumToString(PREVIOUS_EVENT),
                                conditionalSumOfWeights));
      UNSCOPED_INFO(
          std::format("i:{}, conditionalEventFraction = {}", i, conditionalEventFraction));

      static constexpr auto CLOSE_ENOUGH = 0.005;
      REQUIRE(conditionalCountFraction == Approx(conditionalEventFraction).epsilon(CLOSE_ENOUGH));
    }
  }

  SECTION("DisallowEventsSameAsGiven = true")
  {
    const auto conditionalWeightedEvents = ConditionalWeights<Events>{goomRand, weightPairs, true};

    const auto conditionalEventCounts = GetConditionalWeightedCounts(conditionalWeightedEvents);

    REQUIRE(conditionalEventCounts.at(static_cast<size_t>(PREVIOUS_EVENT)) == 0);
  }
}
// NOLINTEND(readability-function-cognitive-complexity)

// NOLINTBEGIN(readability-function-cognitive-complexity)
TEST_CASE("Weighted Events Corner Cases")
{
  const auto goomRand       = GoomRand{};
  const auto weightedEvents = Weights<Events>{
      goomRand,
      {
        {Events::EVENT1, 0.0F},
        {Events::EVENT2, 1.0F},
        {Events::EVENT3, 0.0F},
        {Events::EVENT4, 0.0F},
        }
  };

  SECTION("Unconditional weights")
  {
    const auto sumOfWeights = static_cast<double>(weightedEvents.GetSumOfWeights());
    REQUIRE(sumOfWeights == Approx(1.0F));

    const auto eventCounts = GetWeightedCounts(weightedEvents);

    for (auto i = 0U; i < NUM<Events>; ++i)
    {
      if (static_cast<Events>(i) != Events::EVENT2)
      {
        REQUIRE(eventCounts.at(i) == 0);
      }
      else
      {
        REQUIRE(eventCounts.at(i) == NUM_LOOPS);
      }
    }
  }
}
// NOLINTEND(readability-function-cognitive-complexity)
// NOLINTEND(bugprone-chained-comparison)

/*** Catch2 can't catch 'assert' calls.
TEST_CASE("Weighted Events Exceptions")
{
  const GoomRand goomRand{};

  REQUIRE_THROWS_WITH(Weights<Events>(goomRand, {}).GetNumElements() > 0,
                      "Sum of weights is zero.");

  const typename Weights<Events>::EventWeightPairs weightPairs1 = {
      {Events::EVENT1, 0.0F},
      {Events::EVENT2, 0.0F},
      {Events::EVENT3, 0.0F},
      {Events::EVENT4, 0.0F},
  };
  REQUIRE_THROWS_WITH(Weights<Events>(goomRand, weightPairs1).GetNumElements() > 0,
                      "Sum of weights is zero.");

  const typename Weights<Events>::EventWeightPairs weightPairs2 = {
      {Events::EVENT1, 0.0F},
      {Events::EVENT3, 0.0F},
  };
  REQUIRE_THROWS_WITH(Weights<Events>(goomRand, weightPairs2).GetNumElements() > 0,
                      "Sum of weights is zero.");
}

TEST_CASE("Conditional Weighted Events Exceptions")
{
  const GoomRand goomRand{};

  const typename Weights<Events>::EventWeightPairs weightPairs1 = {
      {Events::EVENT1, 05.0F},
      {Events::EVENT2, 02.0F},
      {Events::EVENT3, 10.0F},
      {Events::EVENT4, 06.0F},
  };
  const std::map<Events, float> event3WeightMultipliers1 = {
      {Events::EVENT1, 0.0F},
      {Events::EVENT2, 0.0F},
      {Events::EVENT3, 0.0F},
      {Events::EVENT4, 0.0F},
  };
  static constexpr Events GIVEN_EVENT = Events::EVENT3;
  const ConditionalWeights<Events>::EventWeightMultiplierPairs weightMultipliers1 = {
      {GIVEN_EVENT, event3WeightMultipliers1}
  };

  REQUIRE_THROWS_WITH(ConditionalWeights<Events>(goomRand, weightPairs1, weightMultipliers1)
                              .GetSumOfWeights(GIVEN_EVENT) > 0,
                      "Sum of weights is zero.");

  const typename Weights<Events>::EventWeightPairs weightPairs2 = {
      {Events::EVENT1, 05.0F},
      {Events::EVENT3, 10.0F},
  };
  const std::map<Events, float> event3WeightMultipliers2 = {
      {Events::EVENT1, 0.0F},
      {Events::EVENT2, 1.0F},
      {Events::EVENT3, 0.0F},
      {Events::EVENT4, 1.0F},
  };
  const ConditionalWeights<Events>::EventWeightMultiplierPairs weightMultipliers2 = {
      {GIVEN_EVENT, event3WeightMultipliers2}
  };

  REQUIRE_THROWS_WITH(ConditionalWeights<Events>(goomRand, weightPairs2, weightMultipliers2)
                              .GetSumOfWeights(GIVEN_EVENT) == 0.0F,
                      "Sum of weights is zero.");
}
***/

} // namespace GOOM::UNIT_TESTS
