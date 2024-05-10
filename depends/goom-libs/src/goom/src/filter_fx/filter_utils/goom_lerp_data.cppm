module;

#include "goom/goom_config.h"

#include <algorithm>
#include <cmath>

export module Goom.FilterFx.FilterUtils.GoomLerpData;

import Goom.Utils.Math.Misc;
import Goom.Utils.Math.TValues;

export namespace GOOM
{

class GoomLerpData
{
public:
  static constexpr auto DEFAULT_INCREMENT = 0.0000001F;

  GoomLerpData() noexcept = default;
  GoomLerpData(float increment, bool useSFunction) noexcept;

  auto Reset() noexcept -> void;

  auto SetLerpToEnd() noexcept -> void;

  [[nodiscard]] auto GetIncrement() const noexcept -> float;
  auto SetIncrement(float increment) noexcept -> void;

  [[nodiscard]] auto GetUseSFunction() const noexcept -> bool;
  auto SetUseSFunction(bool value) noexcept -> void;
  [[nodiscard]] static auto GetSFuncValue(float t) noexcept -> float;
  [[nodiscard]] auto GetSFunctionTIncrement() const noexcept -> float;

  auto Update() noexcept -> void;

  [[nodiscard]] auto GetLerpFactor() const noexcept -> float;

private:
  bool m_useSFunction                 = false;
  static constexpr auto K_VAL         = 5.0F;
  static constexpr auto A_VAL         = 1.5F;
  static constexpr auto DEFAULT_T_INC = 0.007F;
  UTILS::MATH::TValue m_sFuncTVal{
      UTILS::MATH::TValue::StepSizeProperties{
                                              DEFAULT_T_INC, UTILS::MATH::TValue::StepType::CONTINUOUS_REVERSIBLE}
  };
  [[nodiscard]] auto GetNextSFuncValue() const noexcept -> float;

  UTILS::MATH::TValue m_incLerpFactor{
      UTILS::MATH::TValue::StepSizeProperties{
                                              DEFAULT_INCREMENT, UTILS::MATH::TValue::StepType::CONTINUOUS_REVERSIBLE}
  };
  float m_funcLerpFactor = 0.0F;
  float m_lerpFactor     = 0.0F;
};

inline auto GoomLerpData::GetSFuncValue(const float t) noexcept -> float
{
  return 1.0F / (1.0F + std::exp(-K_VAL * ((A_VAL * t) - UTILS::MATH::HALF)));
}

inline auto GoomLerpData::GetSFunctionTIncrement() const noexcept -> float
{
  return m_sFuncTVal.GetStepSize();
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
inline GoomLerpData::GoomLerpData(const float increment, const bool useSFunction) noexcept
  : m_useSFunction{useSFunction},
    m_incLerpFactor{
        UTILS::MATH::TValue::StepSizeProperties{
            increment,
            UTILS::MATH::TValue::StepType::CONTINUOUS_REVERSIBLE}},
    m_funcLerpFactor{not useSFunction ? 0.0F : GetNextSFuncValue()},
    m_lerpFactor{m_funcLerpFactor}
{
  Expects(increment > -1.0F);
  Expects(increment < +1.0F);
}

inline auto GoomLerpData::Reset() noexcept -> void
{
  m_sFuncTVal.Reset();
  m_incLerpFactor.Reset();
  m_incLerpFactor.SetStepSize(DEFAULT_INCREMENT);
  m_funcLerpFactor = 0.0F;
  m_lerpFactor     = 0.0F;
}

inline auto GoomLerpData::GetIncrement() const noexcept -> float
{
  return m_incLerpFactor.GetStepSize();
}

inline auto GoomLerpData::SetIncrement(const float increment) noexcept -> void
{
  Expects(increment > 0.0F);

  m_incLerpFactor.SetStepSize(increment);
}

inline auto GoomLerpData::GetUseSFunction() const noexcept -> bool
{
  return m_useSFunction;
}

inline auto GoomLerpData::SetUseSFunction(const bool value) noexcept -> void
{
  m_useSFunction = value;
}

inline auto GoomLerpData::SetLerpToEnd() noexcept -> void
{
  m_sFuncTVal.Reset(1.0F);
  m_incLerpFactor.Reset(1.0F);
}

inline auto GoomLerpData::Update() noexcept -> void
{
  if (m_useSFunction)
  {
    m_sFuncTVal.Increment();
    m_funcLerpFactor = std::lerp(0.0F, 1.0F, GetNextSFuncValue());
  }

  m_incLerpFactor.Increment();
  m_lerpFactor = std::clamp(m_funcLerpFactor + m_incLerpFactor(), 0.0F, 1.0F);
}

inline auto GoomLerpData::GetNextSFuncValue() const noexcept -> float
{
  return GetSFuncValue(m_sFuncTVal());
}

inline auto GoomLerpData::GetLerpFactor() const noexcept -> float
{
  return m_lerpFactor;
}

} // namespace GOOM
