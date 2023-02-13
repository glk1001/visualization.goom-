//#undef NO_LOGGING

#include "t_values.h"

#include "debugging_logger.h"
#include "goom_config.h"
#include "goom_logger.h"

namespace GOOM::UTILS
{

TValue::TValue(const TValue::StepType stepType,
               const float stepSize,
               const float startingT) noexcept
  : m_stepType{stepType}, m_stepSize{stepSize}, m_t{startingT}
{
  Expects(stepSize > 0.0F);
  Expects(startingT >= MIN_T_VALUE);
  Expects(startingT <= MAX_T_VALUE);
}

TValue::TValue(const TValue::StepType stepType,
               const float stepSize,
               const std::vector<DelayPoint>& delayPoints,
               const float startingT) noexcept
  : m_stepType{stepType}, m_stepSize{stepSize}, m_t{startingT}, m_delayPoints{delayPoints}
{
  Expects(stepSize > 0.0F);
  Expects(startingT >= MIN_T_VALUE);
  Expects(startingT <= MAX_T_VALUE);
  ValidateDelayPoints();
}

TValue::TValue(const TValue::StepType stepType,
               const uint32_t numSteps,
               const float startingT) noexcept
  : m_stepType{stepType}, m_stepSize{1.0F / static_cast<float>(numSteps)}, m_t{startingT}
{
  Expects(numSteps > 0U);
  Expects(startingT >= MIN_T_VALUE);
  Expects(startingT <= MAX_T_VALUE);
}

TValue::TValue(const TValue::StepType stepType,
               const uint32_t numSteps,
               const std::vector<DelayPoint>& delayPoints,
               const float startingT) noexcept
  : m_stepType{stepType},
    m_stepSize{1.0F / static_cast<float>(numSteps)},
    m_t{startingT},
    m_delayPoints{delayPoints}
{
  Expects(numSteps > 0U);
  Expects(startingT >= MIN_T_VALUE);
  Expects(startingT <= MAX_T_VALUE);
  ValidateDelayPoints();
}

TValue::~TValue() noexcept = default;

auto TValue::ValidateDelayPoints() const noexcept -> void
{
#ifndef NDEBUG
  auto prevT0 = -1.0F;
  USED_FOR_DEBUGGING(prevT0);

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
    default:
      FailFast();
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
    HandleBoundary(MIN_T_VALUE, POSITIVE_SIGN);
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
    HandleBoundary(MAX_T_VALUE - m_stepSize, NEGATIVE_SIGN);
    m_currentPosition = Boundaries::INSIDE;
  }
  else if (Boundaries::START == m_currentPosition)
  {
    HandleBoundary(MIN_T_VALUE + m_stepSize, POSITIVE_SIGN);
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


inline auto TValue::HandleBoundary(const float continueValue, const float stepSign) noexcept -> void
{
  m_t = continueValue;

  if (stepSign < 0.0F)
  {
    m_currentStep = -m_stepSize;
    Ensures(m_currentStep < 0.0F);
  }
  else
  {
    m_currentStep = +m_stepSize;
    Ensures(m_currentStep > 0.0F);
  }

  m_currentDelayPoints = m_delayPoints;
}

} // namespace GOOM::UTILS
