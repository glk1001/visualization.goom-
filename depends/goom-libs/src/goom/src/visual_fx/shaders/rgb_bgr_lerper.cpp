#include "rgb_bgr_lerper.h"

//#undef NO_LOGGING

#include "logging.h"

namespace GOOM::VISUAL_FX::SHADERS
{

using UTILS::Logging; // NOLINT(misc-unused-using-decls)

RgbBgrLerper::RgbBgrLerper(const PluginInfo& goomInfo,
                           const UTILS::MATH::IGoomRand& goomRand) noexcept
  : m_goomInfo{goomInfo}, m_goomRand{goomRand}, m_colorIndexes{0, 1, 2}
{
}

auto RgbBgrLerper::Update() noexcept -> void
{
  m_lerpOffTimer.Increment();
  if (not m_lerpOffTimer.Finished())
  {
    if (not RestartLerp())
    {
      return;
    }
    m_lerpOffTimer.SetToFinished();
  }

  Expects(m_lerpOffTimer.Finished());
  if (m_lerpOffTimer.JustFinished())
  {
    m_lerpT.SetNumSteps(
        m_goomRand.GetRandInRange(MIN_NUM_LERP_ON_STEPS, MAX_NUM_LERP_ON_STEPS + 1));
    LogInfo("LerpT = {}. m_currentT = {}. Reset lerpT steps to {}",
            m_lerpT(),
            m_currentT,
            m_lerpT.GetNumSteps());

    m_goomRand.Shuffle(begin(m_colorIndexes), end(m_colorIndexes));
    LogInfo("Reset m_colorIndexes = [{},{},{}].",
            m_colorIndexes.at(0),
            m_colorIndexes.at(1),
            m_colorIndexes.at(2));
  }

  m_currentT = m_lerpT();
  m_lerpT.Increment();

  if (m_lerpT.HasJustHitStartBoundary() or m_lerpT.HasJustHitEndBoundary())
  {
    m_lerpOffTimer.SetTimeLimit(
        m_goomRand.GetRandInRange(MIN_LERP_OFF_TIME, MAX_LERP_OFF_TIME + 1));
    LogInfo("LerpT = {}. m_currentT = {}. Set off timer {}",
            m_lerpT(),
            m_currentT,
            m_lerpOffTimer.GetTimeLimit());

    m_currentT = m_lerpT.HasJustHitStartBoundary() ? 0.0F : 1.0F;
  }
}

inline auto RgbBgrLerper::RestartLerp() const noexcept -> bool
{
  if (static constexpr float PROB_RESTART_LERP_AFTER_BIG_GOOM = 0.5F;
      m_goomRand.ProbabilityOf(PROB_RESTART_LERP_AFTER_BIG_GOOM) and
      (0 == m_goomInfo.GetSoundEvents().GetTimeSinceLastBigGoom()))
  {
    LogInfo("Restarting lerp - GetTimeSinceLastBigGoom = {}",
            m_goomInfo.GetSoundEvents().GetTimeSinceLastBigGoom());
    return true;
  }

  if (static constexpr float PROB_RESTART_LERP_AFTER_GOOM = 0.01F;
      m_goomRand.ProbabilityOf(PROB_RESTART_LERP_AFTER_GOOM) and
      (0 == m_goomInfo.GetSoundEvents().GetTimeSinceLastGoom()))
  {
    LogInfo("Restarting lerp - GetTimeSinceLastGoom = {}",
            m_goomInfo.GetSoundEvents().GetTimeSinceLastGoom());
    return true;
  }

  LogInfo("Not restarting lerp - GetTimeSinceLastGoom = {}, GetTimeSinceLastBigGoom = {}",
          m_goomInfo.GetSoundEvents().GetTimeSinceLastGoom(),
          m_goomInfo.GetSoundEvents().GetTimeSinceLastBigGoom());

  return false;
}

} // namespace GOOM::VISUAL_FX::SHADERS
