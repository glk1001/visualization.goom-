#pragma once

#include "mathutils.h"

#include <cstdint>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace UTILS
{
#else
namespace GOOM::UTILS
{
#endif


class TValue
{
public:
  static constexpr float MAX_T_VALUE = 1.0F + SMALL_FLOAT;
  struct DelayPoint
  {
    float t0;
    uint32_t delayTime;
  };
  enum class StepType
  {
    CONTINUOUS_REPEATABLE,
    CONTINUOUS_REVERSIBLE,
    SINGLE_CYCLE,
  };

  TValue(StepType stepType, float stepSize) noexcept;
  TValue(StepType stepType, float stepSize, const std::vector<DelayPoint>& delayPoints) noexcept;
  TValue(StepType stepType, uint32_t numSteps) noexcept;
  TValue(StepType stepType, uint32_t numSteps, const std::vector<DelayPoint>& delayPoints) noexcept;

  [[nodiscard]] auto GetStepType() const -> StepType;
  [[nodiscard]] auto GetStepSize() const -> float;
  void SetStepSize(float val);

  auto operator()() const -> float;
  [[nodiscard]] auto GetCurrentStep() const -> float;
  void Increment();
  auto IsStopped() const -> bool;
  void Reset(float t = 0.0);

private:
  const StepType m_stepType;
  float m_stepSize;
  float m_currentStep;
  float m_t = 0.0F;
  const std::vector<DelayPoint> m_delayPoints;
  std::vector<DelayPoint> m_currentDelayPoints;
  bool m_startedDelay = false;
  uint32_t m_delayPointCount = 0;
  [[nodiscard]] auto IsTimeDelayed() -> bool;
  [[nodiscard]] auto WeAreStartingDelayPoint() -> bool;
  void ValidateDelayPoints();
  void SingleCycleIncrement();
  void ContinuousRepeatableIncrement();
  void ContinuousReversibleIncrement();
  void HandleBoundary(float continueValue, float stepSign);
};

inline auto TValue::GetStepType() const -> StepType
{
  return m_stepType;
}

inline auto TValue::GetStepSize() const -> float
{
  return m_stepSize;
}

inline auto TValue::operator()() const -> float
{
  return m_t;
}

inline auto TValue::GetCurrentStep() const -> float
{
  return m_currentStep;
}

inline auto TValue::IsStopped() const -> bool
{
  if (m_stepType != StepType::SINGLE_CYCLE)
  {
    return false;
  }
  return m_t >= MAX_T_VALUE;
}

inline void TValue::Reset(const float t)
{
  m_t = t;
  m_currentStep = m_stepSize;
}

#if __cplusplus <= 201402L
} // namespace UTILS
} // namespace GOOM
#else
} // namespace GOOM::UTILS
#endif
