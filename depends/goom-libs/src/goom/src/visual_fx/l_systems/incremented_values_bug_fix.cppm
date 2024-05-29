module;

#include "goom/goom_config.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <type_traits>
#include <vector>

module Goom.VisualFx.LSystemFx:IncrementedValuesBugFix;

import LSys.Interpret;
import Goom.Utils.Math.TValues;
import Goom.Utils.Math.Misc;
import Goom.Lib.GoomTypes;

namespace GOOM::UTILS::MATH
{
template<typename T>
class IncrementedValueBugFix
{
public:
  IncrementedValueBugFix(TValue::StepType stepType, uint32_t numSteps) noexcept;
  IncrementedValueBugFix(const T& value1,
                         const T& value2,
                         TValue::StepType stepType,
                         uint32_t numSteps) noexcept;
  IncrementedValueBugFix(const T& value1,
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

} // namespace GOOM::UTILS::MATH

namespace GOOM::UTILS::MATH
{

using MATH::SMALL_FLOAT;
using MATH::UnorderedClamp;

using DefaultParams = ::LSYS::Interpreter::DefaultParams;

auto lerp(const DefaultParams& val1, const DefaultParams& val2, const float t) noexcept
    -> DefaultParams;

auto lerp(const DefaultParams& val1, const DefaultParams& val2, const float t) noexcept
    -> DefaultParams
{
  return {
      std::lerp(val1.turnAngleInDegrees, val2.turnAngleInDegrees, t),
      std::lerp(val1.width, val2.width, t),
      std::lerp(val1.distance, val2.distance, t),
  };
}

auto GetMatching(const DefaultParams& val,
                 const DefaultParams& val1,
                 const DefaultParams& val2) noexcept -> float;

auto GetMatching(const DefaultParams& val,
                 const DefaultParams& val1,
                 const DefaultParams& val2) noexcept -> float
{
  if (std::fabs(val2.turnAngleInDegrees - val1.turnAngleInDegrees) > SMALL_FLOAT)
  {
    return ((val.turnAngleInDegrees - val1.turnAngleInDegrees) /
            (val2.turnAngleInDegrees - val1.turnAngleInDegrees));
  }
  if (std::fabs(val2.width - val1.width) > SMALL_FLOAT)
  {
    return ((val.width - val1.width) / (val2.width - val1.width));
  }
  if (std::fabs(val2.distance - val1.distance) > SMALL_FLOAT)
  {
    return ((val.distance - val1.distance) / (val2.distance - val1.distance));
  }
  return 0.0F;
}

auto Clamped(const DefaultParams& val,
             const DefaultParams& val1,
             const DefaultParams& val2) noexcept -> DefaultParams;

// NOLINTNEXTLINE(readability-identifier-naming)
auto Clamped(const DefaultParams& val,
             const DefaultParams& val1,
             const DefaultParams& val2) noexcept -> DefaultParams
{
  return {UnorderedClamp(val.turnAngleInDegrees, val1.turnAngleInDegrees, val2.turnAngleInDegrees),
          UnorderedClamp(val.width, val1.width, val2.width),
          UnorderedClamp(val.distance, val1.distance, val2.distance)};
}

} // namespace GOOM::UTILS::MATH

namespace GOOM::UTILS::MATH
{

template<typename T>
IncrementedValueBugFix<T>::IncrementedValueBugFix(const TValue::StepType stepType,
                                             const uint32_t numSteps) noexcept
  : m_value1{}, m_value2{}, m_t{{stepType, numSteps}}
{
  Expects(numSteps > 0U);
}

template<typename T>
IncrementedValueBugFix<T>::IncrementedValueBugFix(const T& value1,
                                             const T& value2,
                                             const TValue::StepType stepType,
                                             const uint32_t numSteps) noexcept
  : m_value1{value1}, m_value2{value2}, m_t{{stepType, numSteps}}
{
  Expects(numSteps > 0U);
}

template<typename T>
IncrementedValueBugFix<T>::IncrementedValueBugFix(const T& value1,
                                             const T& value2,
                                             const TValue::StepType stepType,
                                             const float stepSize) noexcept
  : m_value1{value1}, m_value2{value2}, m_t{TValue::StepSizeProperties{stepSize, stepType}}
{
  Expects(stepSize > 0.0F);
}

template<typename T>
auto IncrementedValueBugFix<T>::GetValue1() const noexcept -> const T&
{
  return m_value1;
}

template<typename T>
auto IncrementedValueBugFix<T>::GetValue2() const noexcept -> const T&
{
  return m_value2;
}

template<typename T>
auto IncrementedValueBugFix<T>::SetValue1(const T& value1) noexcept -> void
{
  m_value1 = value1;
  ResetCurrentValue(m_currentValue);
}

template<typename T>
auto IncrementedValueBugFix<T>::SetValue2(const T& value2) noexcept -> void
{
  m_value2 = value2;
  ResetCurrentValue(m_currentValue);
}

template<typename T>
auto IncrementedValueBugFix<T>::SetValues(const T& value1, const T& value2) noexcept -> void
{
  m_value1 = value1;
  m_value2 = value2;
  ResetCurrentValue(m_currentValue);
}

template<typename T>
auto IncrementedValueBugFix<T>::ReverseValues() noexcept -> void
{
  std::swap(m_value1, m_value2);
  ResetCurrentValue(m_currentValue);
}

template<typename T>
auto IncrementedValueBugFix<T>::SetNumSteps(const uint32_t val) noexcept -> void
{
  m_t.SetNumSteps(val);
}

template<typename T>
auto IncrementedValueBugFix<T>::operator()() const noexcept -> const T&
{
  return m_currentValue;
}

template<typename T>
auto IncrementedValueBugFix<T>::Increment() noexcept -> void
{
  m_t.Increment();
  m_currentValue = GetValue(m_t());
}

template<typename T>
auto IncrementedValueBugFix<T>::PeekNext() const noexcept -> T
{
  auto tCopy = m_t;
  tCopy.Increment();
  return GetValue(tCopy());
}

template<typename T>
auto IncrementedValueBugFix<T>::GetValue(const float t) const noexcept -> T
{
  return LerpValues(m_value1, m_value2, t);
}

template<typename T>
auto IncrementedValueBugFix<T>::LerpValues(const T& val1, const T& val2, float t) noexcept -> T
{
  if constexpr (std::is_integral<T>::value)
  {
    return static_cast<T>(std::lerp(static_cast<float>(val1), static_cast<float>(val2), t));
  }

  return static_cast<T>(lerp(val1, val2, t));
}

template<typename T>
auto IncrementedValueBugFix<T>::Clamp(const T& val, const T& val1, const T& val2) noexcept -> T
{
  return Clamped(val, val1, val2);
}

template<typename T>
auto IncrementedValueBugFix<T>::GetMatchingT(const T& val, const T& val1, const T& val2) noexcept
    -> float
{
  return GetMatching(val, val1, val2);
}

template<typename T>
auto IncrementedValueBugFix<T>::GetT() const noexcept -> const TValue&
{
  return m_t;
}

template<typename T>
auto IncrementedValueBugFix<T>::ResetT(const float t) noexcept -> void
{
  m_t.Reset(t);
  m_currentValue = GetValue(m_t());
}

template<typename T>
auto IncrementedValueBugFix<T>::ResetCurrentValue(const T& newValue) noexcept -> void
{
  const auto newClampedValue = Clamp(newValue, m_value1, m_value2);
  ResetT(GetMatchingT(newClampedValue, m_value1, m_value2));
}

} // namespace GOOM::UTILS::MATH
