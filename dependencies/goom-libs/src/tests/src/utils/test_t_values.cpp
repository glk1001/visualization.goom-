#include "catch2/catch.hpp"
#include "utils/mathutils.h"
#include "utils/t_values.h"

#include <string>
#include <vector>

using namespace GOOM::UTILS;

TEST_CASE("TValue SINGLE_CYCLE")
{
  constexpr uint32_t NUM_STEPS = 10;
  TValue tValue{TValue::StepType::SINGLE_CYCLE, NUM_STEPS};
  REQUIRE(tValue() == Approx(0.0F));

  constexpr float STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(tValue.GetStepSize() == Approx(STEP_SIZE));

  tValue.Increment();
  REQUIRE(tValue() == Approx(STEP_SIZE));

  while (!tValue.IsStopped())
  {
    tValue.Increment();
  }
  REQUIRE(tValue() > 1.0F);
}

TEST_CASE("TValue CONTINUOUS_REPEATABLE")
{
  constexpr uint32_t NUM_STEPS = 10;
  TValue tValue{TValue::StepType::CONTINUOUS_REPEATABLE, NUM_STEPS};
  REQUIRE(tValue() == Approx(0.0F));

  constexpr float STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(tValue.GetStepSize() == Approx(STEP_SIZE));

  // One step
  tValue.Increment();
  REQUIRE(tValue() == Approx(STEP_SIZE));

  // Step till 1.0
  for (size_t i = 1; i < NUM_STEPS; ++i)
  {
    tValue.Increment();
  }
  REQUIRE(tValue() == Approx(1.0F));

  // Go back to start
  tValue.Increment();
  REQUIRE(tValue() == Approx(0.0F));
}

TEST_CASE("TValue CONTINUOUS_REVERSIBLE")
{
  constexpr uint32_t NUM_STEPS = 10;
  TValue tValue{TValue::StepType::CONTINUOUS_REVERSIBLE, NUM_STEPS};
  REQUIRE(tValue() == Approx(0.0F));

  constexpr float STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(tValue.GetStepSize() == Approx(STEP_SIZE));

  // One step
  tValue.Increment();
  REQUIRE(tValue() == Approx(STEP_SIZE));

  // Step till 1.0
  for (size_t i = 1; i < NUM_STEPS; ++i)
  {
    tValue.Increment();
  }
  REQUIRE(tValue() == Approx(1.0F));

  // Back down
  tValue.Increment();
  REQUIRE(tValue() == Approx(1.0F - STEP_SIZE));

  // Keep going down
  for (size_t i = 1; i < NUM_STEPS; ++i)
  {
    tValue.Increment();
  }
  REQUIRE(tValue() == Approx(0.0F).margin(SMALL_FLOAT));

  // Back up
  tValue.Increment();
  REQUIRE(tValue() == Approx(STEP_SIZE));
}

TEST_CASE("TValue CONTINUOUS_REPEATABLE with delay")
{
  constexpr uint32_t NUM_STEPS = 10;
  constexpr uint32_t T_DELAY_TIME = 5;
  TValue tValue{
      TValue::StepType::CONTINUOUS_REPEATABLE,
      NUM_STEPS,
      {
          {0.0F, T_DELAY_TIME},
          {1.0F, T_DELAY_TIME}
      }
  };
  REQUIRE(tValue() == Approx(0.0F));

  constexpr float STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(tValue.GetStepSize() == Approx(STEP_SIZE));

  // Should be delayed here
  for (size_t i = 0; i < T_DELAY_TIME; ++i)
  {
    tValue.Increment();
    REQUIRE(tValue() == Approx(0.0F));
  }

  // Normal incrementing
  float val = 0.0F;
  for (size_t i = 0; i < NUM_STEPS; ++i)
  {
    tValue.Increment();
    val += STEP_SIZE;
    REQUIRE(tValue() == Approx(val));
  }

  // Should be delayed here
  for (size_t i = 0; i < T_DELAY_TIME; ++i)
  {
    tValue.Increment();
    REQUIRE(tValue() == Approx(1.0F));
  }

  // Back to the start
  tValue.Increment();
  REQUIRE(tValue() == Approx(0.0F));
}

TEST_CASE("TValue CONTINUOUS_REVERSIBLE with delay")
{
  constexpr uint32_t NUM_STEPS = 10;
  constexpr uint32_t T_DELAY_TIME = 5;
  TValue tValue{
      TValue::StepType::CONTINUOUS_REVERSIBLE,
      NUM_STEPS,
      {
          {0.0F, T_DELAY_TIME},
          {1.0F, T_DELAY_TIME}
      }
  };
  REQUIRE(tValue() == Approx(0.0F));

  constexpr float STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(tValue.GetStepSize() == Approx(STEP_SIZE));

  // Should be delayed here
  for (size_t i = 0; i < T_DELAY_TIME; ++i)
  {
    tValue.Increment();
    REQUIRE(tValue() == Approx(0.0F));
  }

  // Normal incrementing
  float val = 0.0F;
  for (size_t i = 0; i < NUM_STEPS; ++i)
  {
    tValue.Increment();
    val += STEP_SIZE;
    REQUIRE(tValue() == Approx(val));
  }

  // Should be delayed here
  for (size_t i = 0; i < T_DELAY_TIME; ++i)
  {
    tValue.Increment();
    REQUIRE(tValue() == Approx(1.0F));
  }

  // Back down
  tValue.Increment();
  REQUIRE(tValue() == Approx(1.0F - STEP_SIZE));
}
