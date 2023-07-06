#include "color_multiplier_lerper.h"

#include "goom_config.h"
#include "utils/math/misc.h"

namespace GOOM::VISUAL_FX::SHADERS
{

ColorMultiplierLerper::ColorMultiplierLerper(const PluginInfo& goomInfo,
                                             const UTILS::MATH::IGoomRand& goomRand,
                                             float minColorMultiplier,
                                             float maxColorMultiplier) noexcept
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
  m_lerpConstTimer.Increment();
  if (not m_lerpConstTimer.Finished())
  {
    return;
  }

  m_currentColorMultiplier = STD20::lerp(m_srceColorMultiplier, m_destColorMultiplier, m_lerpT());

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
