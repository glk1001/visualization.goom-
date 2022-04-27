#include "t_values.h"

//#undef NO_LOGGING

#include "goom/logging.h"
#include "utils/math/misc.h"

#undef NDEBUG
#include <cassert>

namespace GOOM::UTILS
{

TValue::TValue(const TValue::StepType stepType,
               const float stepSize,
               const float startingT) noexcept
  : m_stepType{stepType},
    m_stepSize{stepSize},
    m_currentStep{m_stepSize},
    m_t{startingT},
    m_currentPosition{(m_stepType == StepType::SINGLE_CYCLE) ? Boundaries::INSIDE
                                                             : Boundaries::START},
    m_delayPoints{},
    m_currentDelayPoints{m_delayPoints}
{
}

TValue::TValue(const TValue::StepType stepType,
               const float stepSize,
               const std::vector<DelayPoint>& delayPoints,
               const float startingT) noexcept
  : m_stepType{stepType},
    m_stepSize{stepSize},
    m_currentStep{m_stepSize},
    m_t{startingT},
    m_delayPoints{delayPoints},
    m_currentDelayPoints{m_delayPoints}
{
  ValidateDelayPoints();
}

TValue::TValue(const TValue::StepType stepType,
               const uint32_t numSteps,
               const float startingT) noexcept
  : m_stepType{stepType},
    m_stepSize{1.0F / static_cast<float>(numSteps)},
    m_currentStep{m_stepSize},
    m_t{startingT},
    m_delayPoints{},
    m_currentDelayPoints{m_delayPoints}
{
}

TValue::TValue(const TValue::StepType stepType,
               const uint32_t numSteps,
               const std::vector<DelayPoint>& delayPoints,
               const float startingT) noexcept
  : m_stepType{stepType},
    m_stepSize{1.0F / static_cast<float>(numSteps)},
    m_currentStep{m_stepSize},
    m_t{startingT},
    m_delayPoints{delayPoints},
    m_currentDelayPoints{m_delayPoints}
{
  ValidateDelayPoints();
}

auto TValue::ValidateDelayPoints() noexcept -> void
{
  float prevT0 = -1.0F;
  for (const auto& delayPoint : m_delayPoints)
  {
    assert(prevT0 < delayPoint.t0);
    assert(0.0F <= delayPoint.t0);
    assert(delayPoint.t0 <= 1.0F);

    prevT0 = delayPoint.t0;
  }
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
  if (m_t > (1.0F + T_EPSILON))
  {
    return;
  }
  m_t += m_currentStep;
}

inline auto TValue::ContinuousRepeatableIncrement() noexcept -> void
{
  if (IsInDelayZone())
  {
    return;
  }

  m_t += m_currentStep;

  if (m_t > (1.0F + T_EPSILON))
  {
    m_currentPosition = Boundaries::END;
    HandleBoundary(0.0F, +1.0F);
  }
  else
  {
    m_currentPosition = Boundaries::INSIDE;
  }
}

inline auto TValue::ContinuousReversibleIncrement() noexcept -> void
{
  if (IsInDelayZone())
  {
    return;
  }

  m_t += m_currentStep;

  if ((m_t <= 0.0F) || (m_t >= 1.0F))
  {
    m_currentDelayPoints = m_delayPoints;
  }

  CheckContinuousReversibleBoundary();
}

inline auto TValue::CheckContinuousReversibleBoundary() noexcept -> void
{
  if (m_t > (1.0F + T_EPSILON))
  {
    m_currentPosition = Boundaries::END;
    HandleBoundary(1.0F, -1.0F);
    m_t += m_currentStep;
  }
  else if (m_t < (0.0F - T_EPSILON))
  {
    m_currentPosition = Boundaries::START;
    HandleBoundary(0.0F, +1.0F);
    m_t += m_currentStep;
  }
  else
  {
    m_currentPosition = Boundaries::INSIDE;
  }
}

inline auto TValue::IsInDelayZone() noexcept -> bool
{
  if (m_justFinishedDelay)
  {
    m_justFinishedDelay = false;
    return false;
  }

  if ((!m_startedDelay) && WeAreStartingDelayPoint())
  {
    m_startedDelay = true;
  }

  if (m_delayPointCount > 0)
  {
    --m_delayPointCount;
    if (0 == m_delayPointCount)
    {
      m_startedDelay = false;
      m_justFinishedDelay = true;
    }
    return true;
  }

  return false;
}

inline auto TValue::WeAreStartingDelayPoint() noexcept -> bool
{
  for (const auto& delayZone : m_currentDelayPoints)
  {
    if (IsInThisDelayZone(delayZone))
    {
      m_delayPointCount = delayZone.delayTime;
      m_currentDelayPoints.erase(m_currentDelayPoints.begin());
      return true;
    }
  }
  return false;
}

auto TValue::SetStepSize(const float val) noexcept -> void
{
  assert(val >= 0.0F);

  const float oldCurrentStep = m_currentStep;

  m_stepSize = val;
  m_currentStep = m_currentStep < 0.0F ? -m_stepSize : +m_stepSize;

  if (((oldCurrentStep < 0.0F) && (m_currentStep > 0.0F)) ||
      ((oldCurrentStep > 0.0F) && (m_currentStep < 0.0F)))
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
  assert(val != 0U);

  SetStepSize(1.0F / static_cast<float>(val));
}


inline auto TValue::HandleBoundary(const float continueValue, const float stepSign) noexcept -> void
{
  m_t = continueValue;

  if (stepSign < 0.0F)
  {
    m_currentStep = -m_stepSize;
    assert(m_currentStep < 0.0F);
  }
  else
  {
    m_currentStep = +m_stepSize;
    assert(m_currentStep > 0.0F);
  }

  m_currentDelayPoints = m_delayPoints;
}

} // namespace GOOM::UTILS
