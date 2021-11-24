#include "adaptive_exposure.h"

#include "utils/mathutils.h"

#if __cplusplus <= 201402L
namespace GOOM
{
namespace UTILS
{
#else
namespace GOOM::UTILS
{
#endif

void AdaptiveExposure::Start()
{
  m_updateNum = 0;
  m_currentExposure = INITIAL_EXPOSURE;
}

void AdaptiveExposure::UpdateAverageLuminance(float averageLuminance)
{
  ++m_updateNum;

  if (averageLuminance < 1.0F / MAX_EXPOSURE)
  {
    averageLuminance = 1.0F / MAX_EXPOSURE;
  }

  const float targetExposure = stdnew::clamp(1.0F / averageLuminance, MIN_EXPOSURE, MAX_EXPOSURE);

  constexpr float EXPOSURE_LERP_FACTOR = 0.08F;
  m_currentExposure = stdnew::lerp(m_currentExposure, targetExposure, EXPOSURE_LERP_FACTOR);

  if (m_updateNum < RAMP_UP_PERIOD)
  {
    // Don't do anything too sudden until things have smoothed down.
    m_currentExposure = stdnew::clamp(m_currentExposure, 0.9F, 1.05F);
  }
}

#if __cplusplus <= 201402L
} // namespace UTILS
} // namespace GOOM
#else
} // namespace GOOM::UTILS
#endif
