module;

#include <cmath>
#include <cstdint>

module Goom.VisualFx.ShaderFx:ShaderObjectLerper;

import Goom.Utils.Timer;
import Goom.Utils.Math.TValues;
import Goom.Utils.Math.GoomRandBase;
import Goom.Lib.AssertUtils;
import Goom.PluginInfo;

namespace GOOM::VISUAL_FX::SHADERS
{

using UTILS::Timer;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::NumberRange;
using UTILS::MATH::TValue;

class ShaderObjectLerper
{
public:
  struct Params
  {
    NumberRange<float> valueRange{};
    NumberRange<uint32_t> numLerpStepsRange{};
    NumberRange<uint32_t> lerpConstTimeRange{};
    uint32_t initialNumLerpSteps{};
    uint32_t initialLerpConstTime{};
  };

  ShaderObjectLerper(const PluginInfo& goomInfo,
                     const IGoomRand& goomRand,
                     const Params& params) noexcept;

  auto Update() noexcept -> void;
  auto ChangeValueRange() noexcept -> void;

  [[nodiscard]] auto GetLerpedValue() const noexcept -> float;

private:
  [[maybe_unused]] const PluginInfo* m_goomInfo;
  const IGoomRand* m_goomRand;

  static constexpr auto MIN_RANGE = 0.1F;
  Params m_params;
  float m_srceValue = m_goomRand->GetRandInRange(m_params.valueRange);
  float m_destValue = GetNewDestValue();
  [[nodiscard]] auto GetNewDestValue() const noexcept -> float;
  float m_currentLerpedValue = m_srceValue;

  TValue m_lerpT;
  Timer m_lerpConstTimer;
};

} // namespace GOOM::VISUAL_FX::SHADERS

namespace GOOM::VISUAL_FX::SHADERS
{

inline auto ShaderObjectLerper::GetLerpedValue() const noexcept -> float
{
  return m_currentLerpedValue;
}

ShaderObjectLerper::ShaderObjectLerper(const PluginInfo& goomInfo,
                                       const IGoomRand& goomRand,
                                       const Params& params) noexcept
  : m_goomInfo{&goomInfo},
    m_goomRand{&goomRand},
    m_params{params},
    m_lerpT{{TValue::StepType::CONTINUOUS_REVERSIBLE, m_params.initialNumLerpSteps}},
    m_lerpConstTimer{m_goomInfo->GetTime(), m_params.initialLerpConstTime, false}
{
  Expects(m_params.valueRange.min < m_params.valueRange.max);
  Expects(std::fabs(m_params.valueRange.max - m_params.valueRange.min) >= MIN_RANGE);
}

auto ShaderObjectLerper::Update() noexcept -> void
{
  if (not m_lerpConstTimer.Finished())
  {
    return;
  }

  m_currentLerpedValue = std::lerp(m_srceValue, m_destValue, m_lerpT());

  m_lerpConstTimer.ResetToZero();
  m_lerpT.Increment();
}

auto ShaderObjectLerper::ChangeValueRange() noexcept -> void
{
  m_srceValue = m_currentLerpedValue;
  m_destValue = GetNewDestValue();
  m_lerpT.Reset(0.0F);
  m_lerpConstTimer.ResetToZero();

  m_lerpT.SetNumSteps(m_goomRand->GetRandInRange(m_params.numLerpStepsRange));
  m_lerpConstTimer.SetTimeLimit(m_goomRand->GetRandInRange(m_params.lerpConstTimeRange));
}

auto ShaderObjectLerper::GetNewDestValue() const noexcept -> float
{
  static constexpr auto MAX_LOOPS = 10U;

  for (auto i = 0U; i < MAX_LOOPS; ++i)
  {
    const auto destValue = m_goomRand->GetRandInRange(m_params.valueRange);
    if (std::fabs(m_srceValue - destValue) >= MIN_RANGE)
    {
      return destValue;
    }
  }

  return m_goomRand->GetRandInRange(m_params.valueRange);
}

} // namespace GOOM::VISUAL_FX::SHADERS
