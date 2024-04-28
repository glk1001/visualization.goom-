module;

#include "goom/goom_config.h"
#include "goom/goom_types.h"
#include "math/misc.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <type_traits>
#include <vector>

export module Goom.Utils:TValues;

export namespace GOOM::UTILS
{

class TValue
{
public:
  static constexpr float MIN_T_VALUE = 0.0F;
  static constexpr float MAX_T_VALUE = 1.0F;
  struct DelayPoint
  {
    float t0;
    uint32_t delayTime;
  };
  enum class StepType : UnderlyingEnumType
  {
    CONTINUOUS_REPEATABLE,
    CONTINUOUS_REVERSIBLE,
    SINGLE_CYCLE,
  };

  struct StepSizeProperties
  {
    float stepSize = 0.0F;
    StepType stepType{};
    float startingT = 0.0F;
  };
  struct NumStepsProperties
  {
    StepType stepType{};
    uint32_t numSteps = 0U;
    float startingT   = 0.0F;
  };

  explicit TValue(const StepSizeProperties& stepSizeProperties) noexcept;
  TValue(const StepSizeProperties& stepSizeProperties,
         const std::vector<DelayPoint>& delayPoints) noexcept;
  explicit TValue(const NumStepsProperties& numStepsProperties) noexcept;
  TValue(const NumStepsProperties& numStepsProperties,
         const std::vector<DelayPoint>& delayPoints) noexcept;

  [[nodiscard]] auto GetStepType() const noexcept -> StepType;

  [[nodiscard]] auto GetStepSize() const noexcept -> float;
  auto SetStepSize(float stepSize) noexcept -> void;

  [[nodiscard]] auto GetNumSteps() const noexcept -> uint32_t;
  auto SetNumSteps(uint32_t numSteps) noexcept -> void;

  [[nodiscard]] auto IsInsideBoundary() const noexcept -> bool;
  [[nodiscard]] auto HasJustHitStartBoundary() const noexcept -> bool;
  [[nodiscard]] auto HasJustHitEndBoundary() const noexcept -> bool;

  [[nodiscard]] auto DelayJustFinishing() const noexcept -> bool;
  [[nodiscard]] auto IsDelayed() const noexcept -> bool;

  [[nodiscard]] auto GetCurrentStep() const noexcept -> float;
  [[nodiscard]] auto IsStopped() const noexcept -> bool;
  auto Reset(float t = 0.0) noexcept -> void;
  auto Increment() noexcept -> void;
  auto operator()() const noexcept -> float;

private:
  StepType m_stepType;
  float m_stepSize;
  float m_currentStep{m_stepSize};
  float m_t;

  enum class Boundaries : UnderlyingEnumType
  {
    START,
    INSIDE,
    END
  };
  Boundaries m_currentPosition = Boundaries::INSIDE;

  std::vector<DelayPoint> m_delayPoints{};
  std::vector<DelayPoint> m_currentDelayPoints{m_delayPoints};
  bool m_startedDelay              = false;
  bool m_justFinishedDelay         = false;
  uint32_t m_delayPointCount       = 0;
  static constexpr float T_EPSILON = 1.0e-07F;
  [[nodiscard]] auto IsInDelayZone() noexcept -> bool;
  [[nodiscard]] auto IsInThisDelayZone(const DelayPoint& delayPoint) const noexcept -> bool;
  [[nodiscard]] auto WeAreStartingDelayPoint() noexcept -> bool;
  auto ValidateDelayPoints() const noexcept -> void;

  auto SingleCycleIncrement() noexcept -> void;
  auto ContinuousRepeatableIncrement() noexcept -> void;
  auto ContinuousReversibleIncrement() noexcept -> void;
  auto CheckContinuousReversibleBoundary() noexcept -> void;
  enum class FloatSign : UnderlyingEnumType
  {
    POSITIVE,
    NEGATIVE
  };
  auto HandleBoundary(float continueValue, FloatSign floatSign) noexcept -> void;
  auto UpdateCurrentPositionAndStep() -> void;
};

template<typename T>
class IncrementedValue
{
public:
  IncrementedValue(TValue::StepType stepType, uint32_t numSteps) noexcept;
  IncrementedValue(const T& value1,
                   const T& value2,
                   TValue::StepType stepType,
                   uint32_t numSteps) noexcept;
  IncrementedValue(const T& value1,
                   const T& value2,
                   TValue::StepType stepType,
                   float stepSize) noexcept;

  [[nodiscard]] auto GetValue1() const noexcept -> const T&;
  [[nodiscard]] auto GetValue2() const noexcept -> const T&;

  auto SetValue1(const T& value1) noexcept -> void;
  auto SetValue2(const T& value2) noexcept -> void;
  auto SetValues(const T& value1, const T& value2) noexcept -> void;
  auto SetNumSteps(uint32_t val) noexcept -> void;
  auto ReverseValues() noexcept -> void;

  [[nodiscard]] auto operator()() const noexcept -> const T&;
  auto Increment() noexcept -> void;

  [[nodiscard]] auto PeekNext() const noexcept -> T;

  [[nodiscard]] auto GetT() const noexcept -> const TValue&;
  auto ResetT(float t = 0.0) noexcept -> void;
  auto ResetCurrentValue(const T& newValue) noexcept -> void;

private:
  T m_value1;
  T m_value2;
  TValue m_t;
  T m_currentValue = m_value1;
  [[nodiscard]] auto GetValue(float t) const noexcept -> T;
  [[nodiscard]] static auto LerpValues(const T& val1, const T& val2, float t) noexcept -> T;
  // NOLINTNEXTLINE(readability-identifier-naming)
  [[nodiscard]] static auto Clamp(const T& val, const T& val1, const T& val2) noexcept -> T;
  [[nodiscard]] static auto GetMatchingT(const T& val, const T& val1, const T& val2) noexcept
      -> float;
};

} // namespace GOOM::UTILS

namespace GOOM::UTILS
{

inline auto TValue::GetStepType() const noexcept -> StepType
{
  return m_stepType;
}

inline auto TValue::GetStepSize() const noexcept -> float
{
  return m_stepSize;
}

inline auto TValue::operator()() const noexcept -> float
{
  return m_t;
}

inline auto TValue::IsInsideBoundary() const noexcept -> bool
{
  return m_currentPosition == Boundaries::INSIDE;
}

inline auto TValue::HasJustHitStartBoundary() const noexcept -> bool
{
  return m_currentPosition == Boundaries::START;
}

inline auto TValue::HasJustHitEndBoundary() const noexcept -> bool
{
  return m_currentPosition == Boundaries::END;
}

inline auto TValue::DelayJustFinishing() const noexcept -> bool
{
  return 1 == m_delayPointCount;
}

inline auto TValue::IsDelayed() const noexcept -> bool
{
  return m_delayPointCount > 0;
}

inline auto TValue::GetCurrentStep() const noexcept -> float
{
  return m_currentStep;
}

inline auto TValue::IsStopped() const noexcept -> bool
{
  if (m_stepType != StepType::SINGLE_CYCLE)
  {
    return false;
  }
  return m_t >= MAX_T_VALUE;
}

inline auto TValue::Reset(const float t) noexcept -> void
{
  Expects(not std::isnan(t));

  m_t                 = t;
  m_startedDelay      = false;
  m_justFinishedDelay = false;

  UpdateCurrentPositionAndStep();
}

inline auto TValue::UpdateCurrentPositionAndStep() -> void
{
  if (StepType::SINGLE_CYCLE == m_stepType)
  {
    m_currentPosition = Boundaries::INSIDE;
  }
  else if (StepType::CONTINUOUS_REPEATABLE == m_stepType)
  {
    if (m_t >= MAX_T_VALUE)
    {
      m_currentPosition = Boundaries::END;
    }
    else
    {
      m_currentPosition = Boundaries::INSIDE;
    }
  }
  else
  {
    if (m_t >= MAX_T_VALUE)
    {
      m_currentPosition = Boundaries::END;
      m_currentStep     = -m_stepSize;
    }
    else if (m_t <= MIN_T_VALUE)
    {
      m_currentPosition = Boundaries::START;
      m_currentStep     = +m_stepSize;
    }
    else
    {
      m_currentPosition = Boundaries::INSIDE;
    }
  }
}

inline auto TValue::IsInThisDelayZone(const DelayPoint& delayPoint) const noexcept -> bool
{
  return (((delayPoint.t0 - m_stepSize) + T_EPSILON) < m_t) &&
         (m_t < ((delayPoint.t0 + m_stepSize) - T_EPSILON));
}

template<typename T>
inline IncrementedValue<T>::IncrementedValue(const TValue::StepType stepType,
                                             const uint32_t numSteps) noexcept
  : m_value1{}, m_value2{}, m_t{{stepType, numSteps}}
{
  Expects(numSteps > 0U);
}

template<typename T>
inline IncrementedValue<T>::IncrementedValue(const T& value1,
                                             const T& value2,
                                             const TValue::StepType stepType,
                                             const uint32_t numSteps) noexcept
  : m_value1{value1}, m_value2{value2}, m_t{{stepType, numSteps}}
{
  Expects(numSteps > 0U);
}

template<typename T>
inline IncrementedValue<T>::IncrementedValue(const T& value1,
                                             const T& value2,
                                             const TValue::StepType stepType,
                                             const float stepSize) noexcept
  : m_value1{value1}, m_value2{value2}, m_t{TValue::StepSizeProperties{stepSize, stepType}}
{
  Expects(stepSize > 0.0F);
}

template<typename T>
auto IncrementedValue<T>::GetValue1() const noexcept -> const T&
{
  return m_value1;
}

template<typename T>
auto IncrementedValue<T>::GetValue2() const noexcept -> const T&
{
  return m_value2;
}

template<typename T>
inline auto IncrementedValue<T>::SetValue1(const T& value1) noexcept -> void
{
  m_value1 = value1;
  ResetCurrentValue(m_currentValue);
}

template<typename T>
inline auto IncrementedValue<T>::SetValue2(const T& value2) noexcept -> void
{
  m_value2 = value2;
  ResetCurrentValue(m_currentValue);
}

template<typename T>
inline auto IncrementedValue<T>::SetValues(const T& value1, const T& value2) noexcept -> void
{
  m_value1 = value1;
  m_value2 = value2;
  ResetCurrentValue(m_currentValue);
}

template<typename T>
inline auto IncrementedValue<T>::ReverseValues() noexcept -> void
{
  std::swap(m_value1, m_value2);
  ResetCurrentValue(m_currentValue);
}

template<typename T>
inline auto IncrementedValue<T>::SetNumSteps(const uint32_t val) noexcept -> void
{
  m_t.SetNumSteps(val);
}

template<typename T>
inline auto IncrementedValue<T>::operator()() const noexcept -> const T&
{
  return m_currentValue;
}

template<typename T>
inline auto IncrementedValue<T>::Increment() noexcept -> void
{
  m_t.Increment();
  m_currentValue = GetValue(m_t());
}

template<typename T>
inline auto IncrementedValue<T>::PeekNext() const noexcept -> T
{
  auto tCopy = m_t;
  tCopy.Increment();
  return GetValue(tCopy());
}

template<typename T>
inline auto IncrementedValue<T>::GetValue(const float t) const noexcept -> T
{
  return LerpValues(m_value1, m_value2, t);
}

template<typename T>
inline auto IncrementedValue<T>::LerpValues(const T& val1, const T& val2, float t) noexcept -> T
{
  if constexpr (std::is_integral<T>::value)
  {
    return static_cast<T>(std::lerp(static_cast<float>(val1), static_cast<float>(val2), t));
  }

  using std::lerp;
  return static_cast<T>(lerp(val1, val2, t));
}

template<typename T>
inline auto IncrementedValue<T>::Clamp(const T& val, const T& val1, const T& val2) noexcept -> T
{
  return MATH::UnorderedClamp(val, val1, val2);
}

template<typename T>
inline auto IncrementedValue<T>::GetMatchingT(const T& val, const T& val1, const T& val2) noexcept
    -> float
{
  if (std::fabs(static_cast<float>(val2) - static_cast<float>(val1)) < MATH::SMALL_FLOAT)
  {
    return 0.0F;
  }
  return ((static_cast<float>(val) - static_cast<float>(val1)) /
          (static_cast<float>(val2) - static_cast<float>(val1)));
}

template<typename T>
inline auto IncrementedValue<T>::GetT() const noexcept -> const TValue&
{
  return m_t;
}

template<typename T>
inline auto IncrementedValue<T>::ResetT(const float t) noexcept -> void
{
  m_t.Reset(t);
  m_currentValue = GetValue(m_t());
}

template<typename T>
auto IncrementedValue<T>::ResetCurrentValue(const T& newValue) noexcept -> void
{
  const auto newClampedValue = Clamp(newValue, m_value1, m_value2);
  ResetT(GetMatchingT(newClampedValue, m_value1, m_value2));
}

} // namespace GOOM::UTILS

namespace GOOM::UTILS
{

TValue::TValue(const StepSizeProperties& stepSizeProperties) noexcept
  : m_stepType{stepSizeProperties.stepType},
    m_stepSize{stepSizeProperties.stepSize},
    m_t{stepSizeProperties.startingT}
{
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
#ifdef GOOM_DEBUG
  auto prevT0 = -1.0F;

  for (const auto& delayPoint : m_delayPoints)
  {
    Expects(prevT0 < delayPoint.t0);
    Expects(0.0F <= delayPoint.t0);
    Expects(delayPoint.t0 <= 1.0F);

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

  USED_FOR_DEBUGGING(std::isnan(m_t));
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

auto TValue::SetStepSize(const float stepSize) noexcept -> void
{
  Expects(stepSize > 0.0F);
  Expects((m_stepType != StepType::SINGLE_CYCLE) or (m_currentStep >= 0.0F));

  const auto oldCurrentStep = m_currentStep;

  m_stepSize    = stepSize;
  m_currentStep = m_currentStep < 0.0F ? -m_stepSize : +m_stepSize;

  if (((oldCurrentStep < 0.0F) and (m_currentStep > 0.0F)) or
      ((oldCurrentStep > 0.0F) and (m_currentStep < 0.0F)))
  {
    m_currentDelayPoints = m_delayPoints;
  }
}

auto TValue::GetNumSteps() const noexcept -> uint32_t
{
  Expects(m_stepSize > 0.0F);

  return static_cast<uint32_t>(1.0F / m_stepSize);
}

auto TValue::SetNumSteps(const uint32_t numSteps) noexcept -> void
{
  Expects(numSteps > 0U);

  SetStepSize(1.0F / static_cast<float>(numSteps));
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
