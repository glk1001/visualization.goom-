//#undef NO_LOGGING

#include "t_values.h"

#include "goom_config.h"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <vector>

namespace GOOM::UTILS
{

TValue::TValue(const StepSizeProperties& stepSizeProperties) noexcept
  : m_stepType{stepSizeProperties.stepType},
    m_stepSize{stepSizeProperties.stepSize},
    m_t{stepSizeProperties.startingT}
{
  Expects(stepSizeProperties.stepSize > 0.0F);
  Expects(stepSizeProperties.startingT >= MIN_T_VALUE);
  Expects(stepSizeProperties.startingT <= MAX_T_VALUE);
}

TValue::TValue(const StepSizeProperties& stepSizeProperties,
               const std::vector<DelayPoint>& delayPoints) noexcept
  : m_stepType{stepSizeProperties.stepType},
    m_stepSize{stepSizeProperties.stepSize},
    m_t{stepSizeProperties.startingT},
    m_delayPoints{delayPoints}
{
  Expects(stepSizeProperties.stepSize > 0.0F);
  Expects(stepSizeProperties.startingT >= MIN_T_VALUE);
  Expects(stepSizeProperties.startingT <= MAX_T_VALUE);
  ValidateDelayPoints();
}

TValue::TValue(const NumStepsProperties& numStepsProperties) noexcept
  : m_stepType{numStepsProperties.stepType},
    m_stepSize{1.0F / static_cast<float>(numStepsProperties.numSteps)},
    m_t{numStepsProperties.startingT}
{
  Expects(numStepsProperties.numSteps > 0U);
  Expects(numStepsProperties.startingT >= MIN_T_VALUE);
  Expects(numStepsProperties.startingT <= MAX_T_VALUE);
}

TValue::TValue(const NumStepsProperties& numStepsProperties,
               const std::vector<DelayPoint>& delayPoints) noexcept
  : m_stepType{numStepsProperties.stepType},
    m_stepSize{1.0F / static_cast<float>(numStepsProperties.numSteps)},
    m_t{numStepsProperties.startingT},
    m_delayPoints{delayPoints}
{
  Expects(numStepsProperties.numSteps > 0U);
  Expects(numStepsProperties.startingT >= MIN_T_VALUE);
  Expects(numStepsProperties.startingT <= MAX_T_VALUE);
  ValidateDelayPoints();
}

auto TValue::ValidateDelayPoints() const noexcept -> void
{
#ifndef NDEBUG
  auto prevT0 = -1.0F;

  for (const auto& delayPoint : m_delayPoints)
  {
    assert(prevT0 < delayPoint.t0);
    assert(0.0F <= delayPoint.t0);
    assert(delayPoint.t0 <= 1.0F);

    prevT0 = delayPoint.t0;
  }
#endif
}

auto TValue::Increment() noexcept -> void
{
  switch (m_stepType)
  {
    case StepType::SINGLE_CYCLE:
      SingleCycleIncrement();
      break;
    case StepType::CONTINUOUS_REPEATABLE:
      ContinuousRepeatableIncrement();
      break;
    case StepType::CONTINUOUS_REVERSIBLE:
      ContinuousReversibleIncrement();
      break;
  }
}

inline auto TValue::SingleCycleIncrement() noexcept -> void
{
  if (m_t >= MAX_T_VALUE)
  {
    m_t = MAX_T_VALUE;
    return;
  }
  m_t += m_currentStep;

  Ensures(not std::isnan(m_t));
  Ensures(m_t >= MIN_T_VALUE);
}

inline auto TValue::ContinuousRepeatableIncrement() noexcept -> void
{
  if (IsInDelayZone())
  {
    return;
  }

  if (Boundaries::END == m_currentPosition)
  {
    HandleBoundary(MIN_T_VALUE, FloatSign::POSITIVE);
    m_currentPosition = Boundaries::START;
  }
  else
  {
    m_t += m_currentStep;

    if (m_t >= MAX_T_VALUE)
    {
      m_currentPosition = Boundaries::END;
      m_t               = MAX_T_VALUE;
    }
    else
    {
      m_currentPosition = Boundaries::INSIDE;
    }
  }

  Ensures(not std::isnan(m_t));
  Ensures(m_t >= MIN_T_VALUE);
  Ensures(m_t <= MAX_T_VALUE);
}

inline auto TValue::ContinuousReversibleIncrement() noexcept -> void
{
  if (IsInDelayZone())
  {
    return;
  }

  m_t += m_currentStep;

  if ((m_t <= MIN_T_VALUE) or (m_t >= MAX_T_VALUE))
  {
    m_currentDelayPoints = m_delayPoints;
  }

  CheckContinuousReversibleBoundary();

  Ensures(not std::isnan(m_t));
  Ensures(m_t >= MIN_T_VALUE);
  Ensures(m_t <= MAX_T_VALUE);
}

inline auto TValue::CheckContinuousReversibleBoundary() noexcept -> void
{
  if (Boundaries::END == m_currentPosition)
  {
    HandleBoundary(MAX_T_VALUE - m_stepSize, FloatSign::NEGATIVE);
    m_currentPosition = Boundaries::INSIDE;
  }
  else if (Boundaries::START == m_currentPosition)
  {
    HandleBoundary(MIN_T_VALUE + m_stepSize, FloatSign::POSITIVE);
    m_currentPosition = Boundaries::INSIDE;
  }
  else if (m_t >= MAX_T_VALUE)
  {
    m_t               = MAX_T_VALUE;
    m_currentPosition = Boundaries::END;
  }
  else if (m_t <= MIN_T_VALUE)
  {
    m_t               = MIN_T_VALUE;
    m_currentPosition = Boundaries::START;
  }
}

inline auto TValue::IsInDelayZone() noexcept -> bool
{
  if (m_justFinishedDelay)
  {
    m_justFinishedDelay = false;
    return false;
  }

  if ((not m_startedDelay) and WeAreStartingDelayPoint())
  {
    m_startedDelay = true;
  }

  if (m_delayPointCount > 0)
  {
    --m_delayPointCount;
    if (0 == m_delayPointCount)
    {
      m_startedDelay      = false;
      m_justFinishedDelay = true;
    }
    return true;
  }

  return false;
}

inline auto TValue::WeAreStartingDelayPoint() noexcept -> bool
{
  // NOLINTBEGIN(readability-use-anyofallof)
  for (const auto& delayZone : m_currentDelayPoints)
  {
    if (IsInThisDelayZone(delayZone))
    {
      m_delayPointCount = delayZone.delayTime;
      m_currentDelayPoints.erase(begin(m_currentDelayPoints));
      return true;
    }
  }
  return false;
  // NOLINTEND(readability-use-anyofallof)
}

auto TValue::SetStepSize(const float val) noexcept -> void
{
  Expects(val > 0.0F);
  Expects((m_stepType != StepType::SINGLE_CYCLE) or (m_currentStep >= 0.0F));

  const auto oldCurrentStep = m_currentStep;

  m_stepSize    = val;
  m_currentStep = m_currentStep < 0.0F ? -m_stepSize : +m_stepSize;

  if (((oldCurrentStep < 0.0F) and (m_currentStep > 0.0F)) or
      ((oldCurrentStep > 0.0F) and (m_currentStep < 0.0F)))
  {
    m_currentDelayPoints = m_delayPoints;
  }
}

auto TValue::GetNumSteps() const noexcept -> uint32_t
{
  return static_cast<uint32_t>(1.0F / m_stepSize);
}

auto TValue::SetNumSteps(const uint32_t val) noexcept -> void
{
  Expects(val > 0U);

  SetStepSize(1.0F / static_cast<float>(val));
}


inline auto TValue::HandleBoundary(const float continueValue, const FloatSign floatSign) noexcept
    -> void
{
  m_t = continueValue;

  if (FloatSign::NEGATIVE == floatSign)
  {
    m_currentStep = -m_stepSize;
    Ensures(m_currentStep < 0.0F);
  }
  else if (FloatSign::POSITIVE == floatSign)
  {
    m_currentStep = +m_stepSize;
    Ensures(m_currentStep > 0.0F);
  }
  else
  {
    FailFast();
  }

  m_currentDelayPoints = m_delayPoints;
}

} // namespace GOOM::UTILS
