#include "catch2/catch.hpp"
#include "utils/math/misc.h"
#include "utils/t_values.h"

#include <vector>

namespace GOOM::UNIT_TESTS
{

using UTILS::IncrementedValue;
using UTILS::TValue;
using UTILS::MATH::SMALL_FLOAT;

TEST_CASE("TValue SINGLE_CYCLE")
{
  static constexpr auto NUM_STEPS = 10U;
  auto tValue                     = TValue{TValue::StepType::SINGLE_CYCLE, NUM_STEPS};

  static constexpr auto STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
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
  static constexpr auto NUM_STEPS = 10U;
  auto tValue                     = TValue{TValue::StepType::CONTINUOUS_REPEATABLE, NUM_STEPS};

  static constexpr auto STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(tValue.GetStepSize() == Approx(STEP_SIZE));

  // Zero steps
  REQUIRE(tValue() == Approx(0.0F));

  // One step
  tValue.Increment();
  REQUIRE(tValue() == Approx(STEP_SIZE));

  // Step till 1.0
  for (auto i = 1U; i < NUM_STEPS; ++i)
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
  static constexpr auto NUM_STEPS = 10U;
  auto tValue                     = TValue{TValue::StepType::CONTINUOUS_REVERSIBLE, NUM_STEPS};

  static constexpr auto STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(tValue.GetStepSize() == Approx(STEP_SIZE));

  // Zero steps
  REQUIRE(tValue() == Approx(0.0F));

  // One step
  tValue.Increment();
  REQUIRE(tValue() == Approx(STEP_SIZE));

  // Step till 1.0
  for (auto i = 1U; i < NUM_STEPS; ++i)
  {
    tValue.Increment();
  }
  REQUIRE(tValue() == Approx(1.0F));

  // Back down
  tValue.Increment();
  REQUIRE(tValue() == Approx(1.0F - STEP_SIZE));

  // Keep going down
  for (auto i = 1U; i < NUM_STEPS; ++i)
  {
    tValue.Increment();
  }
  REQUIRE(tValue() == Approx(0.0F).margin(SMALL_FLOAT));

  // Back up
  tValue.Increment();
  REQUIRE(tValue() == Approx(STEP_SIZE));
}

void GotoUpToValue(const float t, TValue& tValue)
{
  while (tValue() < t)
  {
    tValue.Increment();
  }
}

void GotoDownToValue(const float t, TValue& tValue)
{
  while (tValue() > t)
  {
    tValue.Increment();
  }
}

void CheckIsDelayed(TValue& tValue, const uint32_t delayTime, const float delayT)
{
  for (auto i = 0U; i < delayTime; ++i)
  {
    INFO("i: " << i << ", delayTime = " << delayTime << ", delayT = " << delayT);

    REQUIRE(tValue.IsDelayed());

    if (i == (delayTime - 1))
    {
      REQUIRE(tValue.DelayJustFinishing());
    }

    tValue.Increment();
    REQUIRE(tValue() == Approx(delayT).margin(SMALL_FLOAT));
  }
}

TEST_CASE("TValue CONTINUOUS_REPEATABLE with delay")
{
  static constexpr auto NUM_STEPS    = 10U;
  static constexpr auto T_DELAY_TIME = 6U;
  static constexpr auto MID_DELAY_T  = 0.5F;
  auto tValue                        = TValue{
      TValue::StepType::CONTINUOUS_REPEATABLE,
      NUM_STEPS,
                             {{0.0F, T_DELAY_TIME}, {MID_DELAY_T, T_DELAY_TIME}, {1.0F, T_DELAY_TIME}}
  };
  REQUIRE((NUM_STEPS % 2) == 0);
  REQUIRE((T_DELAY_TIME % 2) == 0);

  static constexpr auto STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(tValue.GetStepSize() == Approx(STEP_SIZE));

  // Zero steps
  REQUIRE(tValue() == Approx(0.0F));

  // Should be delayed here
  tValue.Increment();
  CheckIsDelayed(tValue, T_DELAY_TIME - 1, 0.0F);
  REQUIRE(!tValue.IsDelayed());

  GotoUpToValue(MID_DELAY_T, tValue);
  REQUIRE(tValue() == Approx(MID_DELAY_T));

  // Should be delayed here
  tValue.Increment();
  CheckIsDelayed(tValue, T_DELAY_TIME - 1, MID_DELAY_T);
  REQUIRE(!tValue.IsDelayed());

  // Normal incrementing up
  auto val = MID_DELAY_T;
  for (auto i = 0U; i < NUM_STEPS / 2; ++i)
  {
    tValue.Increment();
    val += STEP_SIZE;
    INFO("Step = " << i << " val = " << val);
    REQUIRE(tValue() == Approx(val));
  }

  // Should be delayed here
  tValue.Increment();
  CheckIsDelayed(tValue, T_DELAY_TIME - 1, 1.0F);
  REQUIRE(!tValue.IsDelayed());

  // Back to the start
  tValue.Increment();
  REQUIRE(tValue() == Approx(0.0F));
}

TEST_CASE("TValue CONTINUOUS_REVERSIBLE with delay")
{
  static constexpr auto NUM_STEPS    = 10U;
  static constexpr auto T_DELAY_TIME = 6U;
  static constexpr auto MID_DELAY_T  = 0.5F;
  auto tValue                        = TValue{
      TValue::StepType::CONTINUOUS_REVERSIBLE,
      NUM_STEPS,
                             {{0.0F, T_DELAY_TIME}, {MID_DELAY_T, T_DELAY_TIME}, {1.0F, T_DELAY_TIME}}
  };
  REQUIRE((NUM_STEPS % 2) == 0);
  REQUIRE((T_DELAY_TIME % 2) == 0);
  REQUIRE(tValue() == Approx(0.0F));

  static constexpr auto STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(tValue.GetStepSize() == Approx(STEP_SIZE));
  REQUIRE(tValue.GetCurrentStep() > 0.0F);

  // Should be delayed here
  tValue.Increment();
  CheckIsDelayed(tValue, T_DELAY_TIME - 1, 0.0F);
  REQUIRE(!tValue.IsDelayed());

  GotoUpToValue(MID_DELAY_T, tValue);
  REQUIRE(tValue() == Approx(MID_DELAY_T));

  // Should be delayed here
  tValue.Increment();
  CheckIsDelayed(tValue, T_DELAY_TIME - 1, MID_DELAY_T);
  REQUIRE(!tValue.IsDelayed());

  // Normal incrementing
  auto val = MID_DELAY_T;
  for (auto i = 0U; i < NUM_STEPS / 2; ++i)
  {
    tValue.Increment();
    val += STEP_SIZE;
    REQUIRE(tValue() == Approx(val));
  }

  // Should be delayed here
  tValue.Increment();
  CheckIsDelayed(tValue, T_DELAY_TIME - 1, 1.0F);
  REQUIRE(!tValue.IsDelayed());

  // Back down
  tValue.Increment();
  REQUIRE(tValue.GetCurrentStep() < 0.0F);
  REQUIRE(tValue() == Approx(1.0F - STEP_SIZE));

  GotoDownToValue(MID_DELAY_T, tValue);
  REQUIRE(tValue() == Approx(MID_DELAY_T));

  // Should be delayed here
  tValue.Increment();
  CheckIsDelayed(tValue, T_DELAY_TIME - 1, MID_DELAY_T);
  REQUIRE(!tValue.IsDelayed());

  // Normal incrementing - going down
  val = MID_DELAY_T;
  for (auto i = 0U; i < NUM_STEPS / 2; ++i)
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
  CheckIsDelayed(tValue, T_DELAY_TIME - 1, 0.0F);
  REQUIRE(!tValue.IsDelayed());

  // Normal incrementing - going back up
  tValue.Increment();
  REQUIRE(tValue.GetCurrentStep() > 0.0F);
  REQUIRE(tValue() == Approx(STEP_SIZE));
}

TEST_CASE("IncrementedValue SINGLE_CYCLE")
{
  static constexpr auto MIN_VALUE = -5.0F;
  static constexpr auto MAX_VALUE = +15.0F;
  static constexpr auto NUM_STEPS = 10U;
  auto value = IncrementedValue{MIN_VALUE, MAX_VALUE, TValue::StepType::SINGLE_CYCLE, NUM_STEPS};

  static constexpr auto STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(value.GetT().GetStepSize() == Approx(STEP_SIZE));

  // Zero steps
  REQUIRE(value() == Approx(MIN_VALUE));

  // One step
  value.Increment();
  REQUIRE(value() == Approx(MIN_VALUE + (STEP_SIZE * (MAX_VALUE - MIN_VALUE))));

  // Peek
  const auto nextValue = value.PeekNext();
  REQUIRE(nextValue == Approx(MIN_VALUE + ((2.0F * STEP_SIZE) * (MAX_VALUE - MIN_VALUE))));
  REQUIRE(value() == Approx(MIN_VALUE + (STEP_SIZE * (MAX_VALUE - MIN_VALUE))));

  for (auto i = 1U; i < NUM_STEPS; ++i)
  {
    value.Increment();
  }
  REQUIRE(value() == Approx(MAX_VALUE).margin(SMALL_FLOAT));
}

TEST_CASE("IncrementedValue CONTINUOUS_REPEATABLE")
{
  static constexpr auto MIN_VALUE = -5.0F;
  static constexpr auto MAX_VALUE = +15.0F;
  static constexpr auto NUM_STEPS = 10U;
  auto value =
      IncrementedValue{MIN_VALUE, MAX_VALUE, TValue::StepType::CONTINUOUS_REPEATABLE, NUM_STEPS};

  static constexpr auto STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(value.GetT().GetStepSize() == Approx(STEP_SIZE));

  // Zero steps
  REQUIRE(value() == Approx(MIN_VALUE));

  // One step
  value.Increment();
  REQUIRE(value() == Approx(MIN_VALUE + (STEP_SIZE * (MAX_VALUE - MIN_VALUE))));

  // Peek
  const auto nextValue = value.PeekNext();
  REQUIRE(nextValue == Approx(MIN_VALUE + ((2.0F * STEP_SIZE) * (MAX_VALUE - MIN_VALUE))));
  REQUIRE(value() == Approx(MIN_VALUE + (STEP_SIZE * (MAX_VALUE - MIN_VALUE))));

  // Step till MAX_VALUE
  for (auto i = 1U; i < NUM_STEPS; ++i)
  {
    value.Increment();
  }
  REQUIRE(value() == Approx(MAX_VALUE));

  // Should go back to start
  value.Increment();
  REQUIRE(value() == Approx(MIN_VALUE));
}

TEST_CASE("IncrementedValue CONTINUOUS_REVERSIBLE")
{
  static constexpr auto MIN_VALUE = -5.0F;
  static constexpr auto MAX_VALUE = +15.0F;
  static constexpr auto NUM_STEPS = 10U;
  auto value =
      IncrementedValue{MIN_VALUE, MAX_VALUE, TValue::StepType::CONTINUOUS_REVERSIBLE, NUM_STEPS};

  static constexpr auto STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(value.GetT().GetStepSize() == Approx(STEP_SIZE));

  // Zero steps
  REQUIRE(value() == Approx(MIN_VALUE));

  // One step
  value.Increment();
  REQUIRE(value() == Approx(MIN_VALUE + (STEP_SIZE * (MAX_VALUE - MIN_VALUE))));

  // Peek
  const auto nextValue = value.PeekNext();
  REQUIRE(nextValue == Approx(MIN_VALUE + ((2.0F * STEP_SIZE) * (MAX_VALUE - MIN_VALUE))));
  REQUIRE(value() == Approx(MIN_VALUE + (STEP_SIZE * (MAX_VALUE - MIN_VALUE))));

  // Step till MAX_VALUE
  for (auto i = 1U; i < NUM_STEPS; ++i)
  {
    value.Increment();
  }
  REQUIRE(value() == Approx(MAX_VALUE));

  // Back down
  value.Increment();
  REQUIRE(value() == Approx(MIN_VALUE + ((1.0F - STEP_SIZE) * (MAX_VALUE - MIN_VALUE))));

  // Keep going down
  for (auto i = 1U; i < NUM_STEPS; ++i)
  {
    value.Increment();
  }
  REQUIRE(value() == Approx(MIN_VALUE).margin(SMALL_FLOAT));

  // Back up
  value.Increment();
  REQUIRE(value() == Approx(MIN_VALUE + (STEP_SIZE * (MAX_VALUE - MIN_VALUE))));
}

} // namespace GOOM::UNIT_TESTS
