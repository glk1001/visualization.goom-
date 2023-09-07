#pragma once

#include "goom/goom_config.h"
#include "goom/goom_types.h"
#include "goom/math20.h"

#include <cstdint>

namespace GOOM::UTILS
{

class StepSpeed
{
public:
  StepSpeed(const MinMaxValues<uint32_t>& minMaxNumSteps, float initialSpeed);

  auto SetMinMaxNumSteps(const MinMaxValues<uint32_t>& minMaxNumSteps) -> void;
  auto SetSpeed(float val) -> void;

  [[nodiscard]] auto GetCurrentNumSteps() const -> uint32_t;

  template<typename T>
  auto ApplySpeed(T& obj) -> void;

private:
  uint32_t m_minNumSteps;
  uint32_t m_maxNumSteps;
  float m_tMinMaxLerp;
  uint32_t m_currentNumSteps = 0U;
  auto SetCurrentNumSteps() -> void;
};

inline StepSpeed::StepSpeed(const MinMaxValues<uint32_t>& minMaxNumSteps, const float initialSpeed)
  : m_minNumSteps{minMaxNumSteps.minValue},
    m_maxNumSteps{minMaxNumSteps.maxValue},
    m_tMinMaxLerp{initialSpeed}
{
  Expects(0 < minMaxNumSteps.minValue);
  Expects(minMaxNumSteps.minValue < minMaxNumSteps.maxValue);
  Expects(0.0F <= initialSpeed);

  SetCurrentNumSteps();
}

inline auto StepSpeed::SetMinMaxNumSteps(const MinMaxValues<uint32_t>& minMaxNumSteps) -> void
{
  Expects(0 < minMaxNumSteps.minValue);
  Expects(minMaxNumSteps.minValue < minMaxNumSteps.maxValue);
  m_minNumSteps = minMaxNumSteps.minValue;
  m_maxNumSteps = minMaxNumSteps.maxValue;
  SetCurrentNumSteps();
}

inline auto StepSpeed::SetSpeed(const float val) -> void
{
  Expects(0.0F <= val);
  Expects(val <= 1.0F);
  m_tMinMaxLerp = val;
  SetCurrentNumSteps();
}

inline auto StepSpeed::SetCurrentNumSteps() -> void
{
  m_currentNumSteps = STD20::lerp(m_maxNumSteps, m_minNumSteps, m_tMinMaxLerp);
}

inline auto StepSpeed::GetCurrentNumSteps() const -> uint32_t
{
  return m_currentNumSteps;
}

template<typename T>
inline auto StepSpeed::ApplySpeed(T& obj) -> void
{
  obj.SetNumSteps(m_currentNumSteps);
}

} // namespace GOOM::UTILS
