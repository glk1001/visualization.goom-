#include "catch2/catch.hpp"
#include "utils/enumutils.h"
#include "utils/goom_rand.h"
#include "utils/mathutils.h"

#include <cstdint>
#include <format>
#include <string>

using GOOM::UTILS::floats_equal;
using GOOM::UTILS::GoomRand;
using GOOM::UTILS::NUM;
using GOOM::UTILS::Weights;

TEST_CASE("weighted events", "[weightedEvents]")
{
  enum class Events
  {
    EVENT1,
    EVENT2,
    EVENT3,
    EVENT4,
    _NUM
  };
  GoomRand goomRand{};
  const Weights<Events> WEIGHTED_EVENTS{goomRand,
                                        {
                                            {Events::EVENT1, 5},
                                            {Events::EVENT2, 2},
                                            {Events::EVENT3, 10},
                                            {Events::EVENT4, 6},
                                        }};
  constexpr size_t NUM_EVENTS = NUM<Events>;

  constexpr size_t NUM_LOOP = 10000000;
  std::array<uint32_t, NUM_EVENTS> counts{};
  for (size_t i = 0; i < NUM_LOOP; ++i)
  {
    const auto event = WEIGHTED_EVENTS.GetRandomWeighted();
    counts[static_cast<size_t>(event)]++;
  }

  const size_t sumOfWeights = WEIGHTED_EVENTS.GetSumOfWeights();
  for (size_t i = 0; i < NUM_EVENTS; ++i)
  {
    const double countFraction = static_cast<double>(counts[i]) / static_cast<double>(NUM_LOOP);
    const double eventFraction =
        static_cast<double>(WEIGHTED_EVENTS.GetWeight(static_cast<Events>(i))) /
        static_cast<double>(sumOfWeights);
    REQUIRE(
        floats_equal(static_cast<float>(countFraction), static_cast<float>(eventFraction), 0.001F));
    UNSCOPED_INFO(
        std20::format("i = {}, countFrac = {}, eventFrac = {}", i, countFraction, eventFraction));
  }
}
