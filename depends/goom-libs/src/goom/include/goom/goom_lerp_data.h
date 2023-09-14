#pragma once

#include "goom_config.h"
#include "math20.h"

#include <algorithm>
#include <cmath>

namespace GOOM
{

class GoomLerpData
{
public:
  constexpr GoomLerpData() noexcept = default;
  constexpr GoomLerpData(float increment, bool useSFunction) noexcept;

  constexpr auto Reset() noexcept -> void;

  constexpr auto SetLerpToEnd() noexcept -> void;

  [[nodiscard]] constexpr auto GetIncrement() const noexcept -> float;
  constexpr auto SetIncrement(float increment) noexcept -> void;

  [[nodiscard]] constexpr auto GetUseSFunction() const noexcept -> bool;
  constexpr auto SetUseSFunction(bool value) noexcept -> void;
  [[nodiscard]] static constexpr auto GetSFuncValue(float t) noexcept -> float;
  [[nodiscard]] constexpr auto GetSFunctionTIncrement() const noexcept -> float;

  constexpr auto Update() noexcept -> void;

  constexpr auto GetLerpFactor() const noexcept -> float;

private:
  bool m_useSFunction                 = false;
  static constexpr auto K_VAL         = 5.0F;
  static constexpr auto A_VAL         = 1.5F;
  static constexpr auto DEFAULT_T_INC = 0.007F;
  float m_sFuncTIncrement             = DEFAULT_T_INC;
  float m_sFuncTVal                   = 0.0F;
  [[nodiscard]] constexpr auto GetNextSFuncValue() const noexcept -> float;

  float m_increment      = 0.0F;
  float m_incLerpFactor  = 0.0F;
  float m_funcLerpFactor = 0.0F;
  float m_lerpFactor     = 0.0F;
};

constexpr auto GoomLerpData::GetSFuncValue(const float t) noexcept -> float
{
  return 1.0F / (1.0F + std::exp(-K_VAL * ((A_VAL * t) - 0.5F)));
}

constexpr auto GoomLerpData::GetSFunctionTIncrement() const noexcept -> float
{
  return m_sFuncTIncrement;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
constexpr GoomLerpData::GoomLerpData(const float increment, const bool useSFunction) noexcept
  : m_useSFunction{useSFunction},
    m_increment{increment},
    m_funcLerpFactor{not useSFunction ? 0.0F : GetNextSFuncValue()},
    m_lerpFactor{m_funcLerpFactor}
{
  Expects(increment > -1.0F);
  Expects(increment < +1.0F);
}

constexpr auto GoomLerpData::Reset() noexcept -> void
{
  m_sFuncTVal      = 0.0F;
  m_funcLerpFactor = 0.0F;
  m_increment      = 0.0F;
  m_incLerpFactor  = 0.0F;
  m_lerpFactor     = 0.0F;
}

constexpr auto GoomLerpData::GetIncrement() const noexcept -> float
{
  return m_increment;
}

constexpr auto GoomLerpData::SetIncrement(const float increment) noexcept -> void
{
  Expects(increment > -1.0F);
  Expects(increment < +1.0F);

  m_increment = increment;
}

constexpr auto GoomLerpData::GetUseSFunction() const noexcept -> bool
{
  return m_useSFunction;
}

constexpr auto GoomLerpData::SetUseSFunction(const bool value) noexcept -> void
{
  m_useSFunction = value;
}

constexpr auto GoomLerpData::SetLerpToEnd() noexcept -> void
{
  m_sFuncTVal     = 1.0F;
  m_incLerpFactor = 1.0F;
}

constexpr auto GoomLerpData::Update() noexcept -> void
{
  if (m_useSFunction)
  {
    m_sFuncTVal += m_sFuncTIncrement;
    m_funcLerpFactor = STD20::lerp(0.0F, 1.0F, GetNextSFuncValue());
  }

  m_incLerpFactor = std::clamp(m_incLerpFactor + m_increment, 0.0F, 1.0F);
  m_lerpFactor    = std::clamp(m_funcLerpFactor + m_incLerpFactor, 0.0F, 1.0F);
}

constexpr auto GoomLerpData::GetNextSFuncValue() const noexcept -> float
{
  return GetSFuncValue(m_sFuncTVal);
}

constexpr auto GoomLerpData::GetLerpFactor() const noexcept -> float
{
  return m_lerpFactor;
}

} // namespace GOOM
