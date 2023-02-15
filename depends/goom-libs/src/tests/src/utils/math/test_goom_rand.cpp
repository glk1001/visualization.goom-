#include "catch2/catch.hpp"
#include "utils/enum_utils.h"
#include "utils/math/goom_rand.h"

#include <array>
#include <format>
#include <map>

namespace GOOM::UNIT_TESTS
{

using UTILS::EnumToString;
using UTILS::NUM;
using UTILS::MATH::ConditionalWeights;
using UTILS::MATH::GoomRand;
using UTILS::MATH::Weights;

enum class Events
{
  EVENT1,
  EVENT2,
  EVENT3,
  EVENT4,
  _num // unused, and marks the enum end
};
using EventCounts = std::array<uint32_t, NUM<Events>>;

namespace
{
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

[[nodiscard]] auto GetConditionalWeightedCounts(const Events givenEvent,
                                                const ConditionalWeights<Events>& weights)
    -> EventCounts
{
  EventCounts eventCounts{};
  for (auto i = 0U; i < NUM_LOOPS; ++i)
  {
    const auto event = weights.GetRandomWeighted(givenEvent);
    ++eventCounts.at(static_cast<size_t>(event));
  }

  return eventCounts;
}

} // namespace

TEST_CASE("Weighted Events")
{
  const auto goomRand                                 = GoomRand{};
  const Weights<Events>::EventWeightPairs weightPairs = {
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

      UNSCOPED_INFO(std20::format(
          "i = {}, countFraction = {}, eventFraction = {}", i, countFraction, eventFraction));
      static constexpr auto CLOSE_ENOUGH = 0.005;
      REQUIRE(countFraction == Approx(eventFraction).epsilon(CLOSE_ENOUGH));
    }
  }

  SECTION("Conditional weights")
  {
    static constexpr auto GIVEN_EVENT = Events::EVENT3;

    static const auto s_EVENT3_WEIGHT_MULTIPLIERS = std::map<Events, float>{
        {Events::EVENT1, 1.0F},
        {Events::EVENT2, 0.0F},
        {Events::EVENT3, 2.0F},
        {Events::EVENT4, 1.0F},
    };
    static const auto s_WEIGHT_MULTIPLIERS = ConditionalWeights<Events>::EventWeightMultiplierPairs{
        {GIVEN_EVENT, s_EVENT3_WEIGHT_MULTIPLIERS}
    };
    const auto conditionalWeightedEvents =
        ConditionalWeights<Events>{goomRand, weightPairs, s_WEIGHT_MULTIPLIERS};
    const auto conditionalSumOfWeights =
        static_cast<double>(conditionalWeightedEvents.GetSumOfWeights(GIVEN_EVENT));
    static constexpr auto EXPECTED_SUM_FOR_GIVEN = 5.0 + 2.0 * 10.0 + 6.0;
    REQUIRE(conditionalSumOfWeights == Approx(EXPECTED_SUM_FOR_GIVEN));

    const auto conditionalEventCounts =
        GetConditionalWeightedCounts(GIVEN_EVENT, conditionalWeightedEvents);

    for (auto i = 0U; i < NUM<Events>; ++i)
    {
      const auto fConditionalEventCount   = static_cast<double>(conditionalEventCounts.at(i));
      const auto conditionalCountFraction = fConditionalEventCount / DBL_NUM_LOOPS;

      UNSCOPED_INFO(std20::format("i:{}, fConditionalEventCount = {}", i, fConditionalEventCount));
      UNSCOPED_INFO(std20::format("i:{}, NUM_LOOPS = {}", i, NUM_LOOPS));
      UNSCOPED_INFO(
          std20::format("i:{}, conditionalCountFraction = {}", i, conditionalCountFraction));

      const auto fConditionalEventWeight = static_cast<double>(
          conditionalWeightedEvents.GetWeight(GIVEN_EVENT, static_cast<Events>(i)));
      const auto conditionalEventFraction = fConditionalEventWeight / conditionalSumOfWeights;
      UNSCOPED_INFO(std20::format("i:{}, fConditionalEventWeight({}) = {}",
                                  i,
                                  EnumToString(GIVEN_EVENT),
                                  fConditionalEventWeight));
      UNSCOPED_INFO(std20::format("i:{}, conditionalSumOfWeights({}) = {}",
                                  i,
                                  EnumToString(GIVEN_EVENT),
                                  conditionalSumOfWeights));
      UNSCOPED_INFO(
          std20::format("i:{}, conditionalEventFraction = {}", i, conditionalEventFraction));

      static constexpr auto CLOSE_ENOUGH = 0.005;
      REQUIRE(conditionalCountFraction == Approx(conditionalEventFraction).epsilon(CLOSE_ENOUGH));
    }
  }

  SECTION("DisallowEventsSameAsGiven = true")
  {
    static constexpr auto GIVEN_EVENT = Events::EVENT3;

    const auto conditionalWeightedEvents = ConditionalWeights<Events>{goomRand, weightPairs, true};

    const auto conditionalEventCounts =
        GetConditionalWeightedCounts(GIVEN_EVENT, conditionalWeightedEvents);

    REQUIRE(conditionalEventCounts.at(static_cast<size_t>(GIVEN_EVENT)) == 0);
  }
}

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
