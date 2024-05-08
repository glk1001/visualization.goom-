module;

#include "goom/goom_config.h"

#include <cmath>
#include <cstdint>

module Goom.VisualFx.ShaderFx:ColorMultiplierLerper;

import Goom.Utils.Timer;
import Goom.Utils.Math.TValues;
import Goom.Utils.Math.GoomRandBase;
import Goom.PluginInfo;

namespace GOOM::VISUAL_FX::SHADERS
{

class ColorMultiplierLerper
{
public:
  ColorMultiplierLerper(const PluginInfo& goomInfo,
                        const UTILS::MATH::IGoomRand& goomRand,
                        float minColorMultiplier,
                        float maxColorMultiplier) noexcept;

  auto Update() noexcept -> void;
  auto ChangeMultiplierRange() noexcept -> void;

  [[nodiscard]] auto GetColorMultiplier() const noexcept -> float;

private:
  const PluginInfo* m_goomInfo;
  const UTILS::MATH::IGoomRand* m_goomRand;

  static constexpr auto MIN_RANGE = 0.025F;
  float m_minColorMultiplier;
  float m_maxColorMultiplier;
  float m_srceColorMultiplier =
      m_goomRand->GetRandInRange(m_minColorMultiplier, m_maxColorMultiplier);
  float m_destColorMultiplier = GetDestColorMultiplier();
  [[nodiscard]] auto GetDestColorMultiplier() const noexcept -> float;
  float m_currentColorMultiplier = m_srceColorMultiplier;

  static constexpr uint32_t MIN_NUM_LERP_ON_STEPS     = 50U;
  static constexpr uint32_t MAX_NUM_LERP_ON_STEPS     = 500U;
  static constexpr uint32_t DEFAULT_NUM_LERP_ON_STEPS = MIN_NUM_LERP_ON_STEPS;
  UTILS::MATH::TValue m_lerpT{
      {UTILS::MATH::TValue::StepType::CONTINUOUS_REVERSIBLE, DEFAULT_NUM_LERP_ON_STEPS}
  };

  static constexpr uint32_t MIN_LERP_CONST_TIME     = 10U;
  static constexpr uint32_t MAX_LERP_CONST_TIME     = 50U;
  static constexpr uint32_t DEFAULT_LERP_CONST_TIME = MIN_LERP_CONST_TIME;
  UTILS::Timer m_lerpConstTimer{m_goomInfo->GetTime(), DEFAULT_LERP_CONST_TIME, false};
};

inline auto ColorMultiplierLerper::GetColorMultiplier() const noexcept -> float
{
  return m_currentColorMultiplier;
}

} // namespace GOOM::VISUAL_FX::SHADERS

namespace GOOM::VISUAL_FX::SHADERS
{

ColorMultiplierLerper::ColorMultiplierLerper(const PluginInfo& goomInfo,
                                             const UTILS::MATH::IGoomRand& goomRand,
                                             // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
                                             const float minColorMultiplier,
                                             const float maxColorMultiplier) noexcept
  : m_goomInfo{&goomInfo},
    m_goomRand{&goomRand},
    m_minColorMultiplier{minColorMultiplier},
    m_maxColorMultiplier{maxColorMultiplier}
{
  Expects(minColorMultiplier < maxColorMultiplier);
  Expects(std::fabs(minColorMultiplier - maxColorMultiplier) >= MIN_RANGE);
}

auto ColorMultiplierLerper::Update() noexcept -> void
{
  if (not m_lerpConstTimer.Finished())
  {
    return;
  }

  m_currentColorMultiplier = std::lerp(m_srceColorMultiplier, m_destColorMultiplier, m_lerpT());

  m_lerpConstTimer.ResetToZero();
  m_lerpT.Increment();
}

auto ColorMultiplierLerper::ChangeMultiplierRange() noexcept -> void
{
  m_srceColorMultiplier = m_currentColorMultiplier;
  m_destColorMultiplier = GetDestColorMultiplier();
  m_lerpConstTimer.ResetToZero();

  m_lerpT.SetNumSteps(m_goomRand->GetRandInRange(MIN_NUM_LERP_ON_STEPS, MAX_NUM_LERP_ON_STEPS));
  m_lerpConstTimer.SetTimeLimit(
      m_goomRand->GetRandInRange(MIN_LERP_CONST_TIME, MAX_LERP_CONST_TIME));
}

auto ColorMultiplierLerper::GetDestColorMultiplier() const noexcept -> float
{
  static constexpr auto MAX_LOOPS = 10U;

  for (auto i = 0U; i < MAX_LOOPS; ++i)
  {
    const auto destColorMultiplier =
        m_goomRand->GetRandInRange(m_minColorMultiplier, m_maxColorMultiplier);
    if (std::fabs(m_srceColorMultiplier - m_destColorMultiplier) >= MIN_RANGE)
    {
      return destColorMultiplier;
    }
  }

  return m_goomRand->GetRandInRange(m_minColorMultiplier, m_maxColorMultiplier);
}

} // namespace GOOM::VISUAL_FX::SHADERS
