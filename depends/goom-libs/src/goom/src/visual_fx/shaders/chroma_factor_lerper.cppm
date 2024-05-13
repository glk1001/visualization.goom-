module;

#include "goom/goom_config.h"
#include "goom_plugin_info.h"
#include "utils/math/goom_rand_base.h"
#include "utils/t_values.h"
#include "utils/timer.h"

#include <cmath>
#include <cstdint>

module Goom.VisualFx.ShaderFx:ChromaFactorLerper;

namespace GOOM::VISUAL_FX::SHADERS
{

class ChromaFactorLerper
{
public:
  ChromaFactorLerper(const PluginInfo& goomInfo,
                     const UTILS::MATH::IGoomRand& goomRand,
                     float minChromaFactor,
                     float maxChromaFactor) noexcept;

  auto Update() noexcept -> void;
  auto ChangeChromaFactorRange() noexcept -> void;

  [[nodiscard]] auto GetChromaFactor() const noexcept -> float;

private:
  [[maybe_unused]] const PluginInfo* m_goomInfo;
  const UTILS::MATH::IGoomRand* m_goomRand;

  static constexpr auto MIN_CHROMA_RANGE = 0.1F;
  float m_minChromaFactor;
  float m_maxChromaFactor;
  float m_srceChromaFactor    = m_goomRand->GetRandInRange(m_minChromaFactor, m_maxChromaFactor);
  float m_destChromaFactor    = GetRandomDestChromaFactor();
  float m_currentChromaFactor = m_srceChromaFactor;
  [[nodiscard]] auto GetRandomDestChromaFactor() const noexcept -> float;

  static constexpr uint32_t MIN_NUM_LERP_ON_STEPS     = 50U;
  static constexpr uint32_t MAX_NUM_LERP_ON_STEPS     = 500U;
  static constexpr uint32_t DEFAULT_NUM_LERP_ON_STEPS = MIN_NUM_LERP_ON_STEPS;
  UTILS::TValue m_lerpT{
      {UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE, DEFAULT_NUM_LERP_ON_STEPS}
  };

  static constexpr uint32_t MIN_LERP_CONST_TIME     = 50U;
  static constexpr uint32_t MAX_LERP_CONST_TIME     = 100U;
  static constexpr uint32_t DEFAULT_LERP_CONST_TIME = MIN_LERP_CONST_TIME;
  UTILS::Timer m_lerpConstTimer{m_goomInfo->GetTime(), DEFAULT_LERP_CONST_TIME, false};
};

inline auto ChromaFactorLerper::GetChromaFactor() const noexcept -> float
{
  return m_currentChromaFactor;
}

} // namespace GOOM::VISUAL_FX::SHADERS

namespace GOOM::VISUAL_FX::SHADERS
{

ChromaFactorLerper::ChromaFactorLerper(const PluginInfo& goomInfo,
                                       const UTILS::MATH::IGoomRand& goomRand,
                                       // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
                                       const float minChromaFactor,
                                       const float maxChromaFactor) noexcept
  : m_goomInfo{&goomInfo},
    m_goomRand{&goomRand},
    m_minChromaFactor{minChromaFactor},
    m_maxChromaFactor{maxChromaFactor}
{
  Expects(minChromaFactor < maxChromaFactor);
  Expects(std::fabs(minChromaFactor - maxChromaFactor) >= MIN_CHROMA_RANGE);
}

auto ChromaFactorLerper::Update() noexcept -> void
{
  if (not m_lerpConstTimer.Finished())
  {
    return;
  }

  m_currentChromaFactor = std::lerp(m_srceChromaFactor, m_destChromaFactor, m_lerpT());

  m_lerpConstTimer.ResetToZero();
  m_lerpT.Increment();
}

auto ChromaFactorLerper::ChangeChromaFactorRange() noexcept -> void
{
  m_srceChromaFactor = m_currentChromaFactor;
  m_destChromaFactor = GetRandomDestChromaFactor();
  m_lerpT.Reset(0.0F);

  m_lerpT.SetNumSteps(m_goomRand->GetRandInRange(MIN_NUM_LERP_ON_STEPS, MAX_NUM_LERP_ON_STEPS));
  m_lerpConstTimer.SetTimeLimit(
      m_goomRand->GetRandInRange(MIN_LERP_CONST_TIME, MAX_LERP_CONST_TIME));
}

auto ChromaFactorLerper::GetRandomDestChromaFactor() const noexcept -> float
{
  static constexpr auto MAX_LOOPS = 10U;

  for (auto i = 0U; i < MAX_LOOPS; ++i)
  {
    const auto destChromaFactor = m_goomRand->GetRandInRange(m_minChromaFactor, m_maxChromaFactor);
    if (std::fabs(m_srceChromaFactor - destChromaFactor) >= MIN_CHROMA_RANGE)
    {
      return destChromaFactor;
    }
  }

  return m_goomRand->GetRandInRange(m_minChromaFactor, m_maxChromaFactor);
}

} // namespace GOOM::VISUAL_FX::SHADERS
