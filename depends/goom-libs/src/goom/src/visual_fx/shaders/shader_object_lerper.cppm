module;

#include <cmath>
#include <cstdint>

module Goom.VisualFx.ShaderFx:ShaderObjectLerper;

import Goom.Utils.Timer;
import Goom.Utils.Math.TValues;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.AssertUtils;
import Goom.PluginInfo;

using GOOM::UTILS::Timer;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::NumberRange;
using GOOM::UTILS::MATH::TValue;

namespace GOOM::VISUAL_FX::SHADERS
{

class ShaderObjectLerper
{
public:
  struct Params
  {
    NumberRange<float> valueRange{};
    float minValueRangeDist{};
    NumberRange<uint32_t> numLerpStepsRange{};
    NumberRange<uint32_t> lerpConstTimeRange{};
    uint32_t initialNumLerpSteps{};
    uint32_t initialLerpConstTime{};
  };

  ShaderObjectLerper(const PluginInfo& goomInfo,
                     const GoomRand& goomRand,
                     const Params& params) noexcept;

  auto Update() noexcept -> void;
  auto ChangeValueRange() noexcept -> void;

  [[nodiscard]] auto GetLerpedValue() const noexcept -> float;

private:
  [[maybe_unused]] const PluginInfo* m_goomInfo;
  const GoomRand* m_goomRand;

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
                                       const GoomRand& goomRand,
                                       const Params& params) noexcept
  : m_goomInfo{&goomInfo},
    m_goomRand{&goomRand},
    m_params{params},
    m_lerpT{{TValue::StepType::CONTINUOUS_REVERSIBLE, m_params.initialNumLerpSteps}},
    m_lerpConstTimer{m_goomInfo->GetTime(), m_params.initialLerpConstTime, false}
{
  Expects(m_params.minValueRangeDist > 0.0F);
  Expects(m_params.valueRange.range >= m_params.minValueRangeDist);

  Expects(0 < m_params.numLerpStepsRange.min);
  Expects(m_params.numLerpStepsRange.min <= m_params.initialNumLerpSteps);
  Expects(m_params.initialNumLerpSteps <= m_params.numLerpStepsRange.max);

  Expects(0 < m_params.lerpConstTimeRange.min);
  Expects(m_params.lerpConstTimeRange.min <= m_params.initialLerpConstTime);
  Expects(m_params.initialLerpConstTime <= m_params.lerpConstTimeRange.max);
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

  // NOTE: The new dest value can be < srce value.
  for (auto i = 0U; i < MAX_LOOPS; ++i)
  {
    const auto destValue = m_goomRand->GetRandInRange(m_params.valueRange);
    if (std::fabs(m_srceValue - destValue) >= m_params.minValueRangeDist)
    {
      return destValue;
    }
  }

  return m_goomRand->GetRandInRange(m_params.valueRange);
}

} // namespace GOOM::VISUAL_FX::SHADERS
