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

  constexpr float STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(tValue.GetStepSize() == Approx(STEP_SIZE));

  // Zero steps
  REQUIRE(tValue() == Approx(0.0F));

  // One step
  tValue.Increment();
  REQUIRE(tValue() == Approx(STEP_SIZE));

  while (!tValue.IsStopped())
  {
    tValue.Increment();
  }
  REQUIRE(tValue() >= 1.0F);
}

TEST_CASE("TValue CONTINUOUS_REPEATABLE")
{
  constexpr uint32_t NUM_STEPS = 10;
  TValue tValue{TValue::StepType::CONTINUOUS_REPEATABLE, NUM_STEPS};

  constexpr float STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(tValue.GetStepSize() == Approx(STEP_SIZE));

  // Zero steps
  REQUIRE(tValue() == Approx(0.0F));

  // One step
  tValue.Increment();
  REQUIRE(tValue() == Approx(STEP_SIZE));

  // Step till 1.0
  for (size_t i = 1; i < NUM_STEPS; ++i)
  {
    tValue.Increment();
  }
  REQUIRE(tValue() == Approx(1.0F));

  // Should go back to start
  tValue.Increment();
  REQUIRE(tValue() == Approx(0.0F));
}

TEST_CASE("TValue CONTINUOUS_REVERSIBLE")
{
  constexpr uint32_t NUM_STEPS = 10;
  TValue tValue{TValue::StepType::CONTINUOUS_REVERSIBLE, NUM_STEPS};

  constexpr float STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(tValue.GetStepSize() == Approx(STEP_SIZE));

  // Zero steps
  REQUIRE(tValue() == Approx(0.0F));

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
  constexpr uint32_t T_DELAY_TIME = 6;
  TValue tValue{TValue::StepType::CONTINUOUS_REPEATABLE,
                NUM_STEPS,
                {{0.0F, T_DELAY_TIME}, {0.5F, T_DELAY_TIME}, {1.0F, T_DELAY_TIME}}};
  REQUIRE((NUM_STEPS % 2) == 0);
  REQUIRE((T_DELAY_TIME % 2) == 0);

  constexpr float STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(tValue.GetStepSize() == Approx(STEP_SIZE));

  // Zero steps
  REQUIRE(tValue() == Approx(0.0F));

  // Should be delayed here
  tValue.Increment();
  for (size_t i = 1; i < T_DELAY_TIME; ++i)
  {
    if (i == (T_DELAY_TIME - 1))
    {
      REQUIRE(tValue.DelayJustFinishing());
    }

    REQUIRE(tValue.IsDelayed());
    tValue.Increment();
    REQUIRE(tValue() == Approx(0.0F));
  }
  REQUIRE(!tValue.IsDelayed());

  // Go to 0.5
  while (tValue() < 0.5F)
  {
    tValue.Increment();
  }
  REQUIRE(tValue() == Approx(0.5F));

  // Should be delayed here
  tValue.Increment();
  for (size_t i = 1; i < T_DELAY_TIME; ++i)
  {
    if (i == (T_DELAY_TIME - 1))
    {
      REQUIRE(tValue.DelayJustFinishing());
    }

    REQUIRE(tValue.IsDelayed());
    tValue.Increment();
    REQUIRE(tValue() == Approx(0.5F));
  }
  REQUIRE(!tValue.IsDelayed());

  // Normal incrementing up
  float val = 0.5F;
  for (size_t i = 0; i < NUM_STEPS / 2; ++i)
  {
    tValue.Increment();
    val += STEP_SIZE;
    INFO("Step = " << i << " val = " << val);
    REQUIRE(tValue() == Approx(val));
  }

  // Should be delayed here
  tValue.Increment();
  for (size_t i = 1; i < T_DELAY_TIME; ++i)
  {
    if (i == (T_DELAY_TIME - 1))
    {
      REQUIRE(tValue.DelayJustFinishing());
    }

    REQUIRE(tValue.IsDelayed());
    tValue.Increment();
    REQUIRE(tValue() == Approx(1.0F));
  }
  REQUIRE(!tValue.IsDelayed());

  // Back to the start
  tValue.Increment();
  REQUIRE(tValue() == Approx(0.0F));
}

TEST_CASE("TValue CONTINUOUS_REVERSIBLE with delay")
{
  constexpr uint32_t NUM_STEPS = 10;
  constexpr uint32_t T_DELAY_TIME = 6;
  TValue tValue{TValue::StepType::CONTINUOUS_REVERSIBLE,
                NUM_STEPS,
                {{0.0F, T_DELAY_TIME}, {0.5F, T_DELAY_TIME}, {1.0F, T_DELAY_TIME}}};
  REQUIRE((NUM_STEPS % 2) == 0);
  REQUIRE((T_DELAY_TIME % 2) == 0);
  REQUIRE(tValue() == Approx(0.0F));

  constexpr float STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(tValue.GetStepSize() == Approx(STEP_SIZE));
  REQUIRE(tValue.GetCurrentStep() > 0.0F);

  // Should be delayed here
  tValue.Increment();
  for (size_t i = 1; i < T_DELAY_TIME; ++i)
  {
    if (i == (T_DELAY_TIME - 1))
    {
      REQUIRE(tValue.DelayJustFinishing());
    }

    REQUIRE(tValue.IsDelayed());
    tValue.Increment();
    REQUIRE(tValue() == Approx(0.0F));
  }
  REQUIRE(!tValue.IsDelayed());

  // Go to 0.5
  while (tValue() < 0.5F)
  {
    tValue.Increment();
  }
  REQUIRE(tValue() == Approx(0.5F));

  // Should be delayed here
  tValue.Increment();
  for (size_t i = 1; i < T_DELAY_TIME; ++i)
  {
    if (i == (T_DELAY_TIME - 1))
    {
      REQUIRE(tValue.DelayJustFinishing());
    }

    REQUIRE(tValue.IsDelayed());
    tValue.Increment();
    REQUIRE(tValue() == Approx(0.5F));
  }
  REQUIRE(!tValue.IsDelayed());

  // Normal incrementing
  float val = 0.5F;
  for (size_t i = 0; i < NUM_STEPS / 2; ++i)
  {
    tValue.Increment();
    val += STEP_SIZE;
    REQUIRE(tValue() == Approx(val));
  }

  // Should be delayed here
  tValue.Increment();
  for (size_t i = 1; i < T_DELAY_TIME; ++i)
  {
    if (i == (T_DELAY_TIME - 1))
    {
      REQUIRE(tValue.DelayJustFinishing());
    }

    REQUIRE(tValue.IsDelayed());
    tValue.Increment();
    REQUIRE(tValue() == Approx(1.0F));
  }
  REQUIRE(!tValue.IsDelayed());

  // Back down
  tValue.Increment();
  REQUIRE(tValue.GetCurrentStep() < 0.0F);
  REQUIRE(tValue() == Approx(1.0F - STEP_SIZE));

  // Go down to 0.5
  while (tValue() > 0.5F)
  {
    tValue.Increment();
  }
  REQUIRE(tValue() == Approx(0.5F));

  // Should be delayed here
  tValue.Increment();
  for (size_t i = 1; i < T_DELAY_TIME; ++i)
  {
    if (i == (T_DELAY_TIME - 1))
    {
      REQUIRE(tValue.DelayJustFinishing());
    }

    REQUIRE(tValue.IsDelayed());
    tValue.Increment();
    REQUIRE(tValue() == Approx(0.5F));
  }
  REQUIRE(!tValue.IsDelayed());

  // Normal incrementing - going down
  val = 0.5F;
  for (size_t i = 0; i < NUM_STEPS / 2; ++i)
  {
    tValue.Increment();
    val -= STEP_SIZE;
    REQUIRE(tValue() == Approx(val).margin(SMALL_FLOAT));
  }
  REQUIRE(tValue() == Approx(0.0F).margin(SMALL_FLOAT));
  REQUIRE(tValue.GetCurrentStep() < 0.0F);

  // Should be delayed here
  tValue.Increment();
  REQUIRE(tValue() == Approx(0.0F).margin(SMALL_FLOAT));
  REQUIRE(tValue.GetCurrentStep() < 0.0F);

  for (size_t i = 1; i < T_DELAY_TIME; ++i)
  {
    if (i == (T_DELAY_TIME - 1))
    {
      REQUIRE(tValue.DelayJustFinishing());
    }

    REQUIRE(tValue.IsDelayed());
    tValue.Increment();
    REQUIRE(tValue() == Approx(0.0F).margin(SMALL_FLOAT));
  }
  REQUIRE(!tValue.IsDelayed());

  // Normal incrementing - going back up
  tValue.Increment();
  REQUIRE(tValue.GetCurrentStep() > 0.0F);
  REQUIRE(tValue() == Approx(STEP_SIZE));
}
