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
  m_tooHighLuminanceInARow = 0;
  m_tooLowLuminanceInARow = 0;
  m_currentExposure = INITIAL_EXPOSURE;
}

void AdaptiveExposure::UpdateAverageLuminance(float averageLuminance)
{
  if (averageLuminance < 1.0F / MAX_EXPOSURE)
  {
    averageLuminance = 1.0F / MAX_EXPOSURE;
  }

  UpdateLuminanceCounters(averageLuminance);

  const float targetExposure = stdnew::clamp(1.0F / averageLuminance, MIN_EXPOSURE, MAX_EXPOSURE);

  m_currentExposure =
      stdnew::lerp(m_currentExposure, targetExposure, GetTargetExposureLerpFactor());
}

void AdaptiveExposure::UpdateLuminanceCounters(const float avLuminance)
{
  constexpr float TOO_LOW_LUMINANCE = 0.05F;
  constexpr float TOO_HIGH_LUMINANCE = 5.0F;

  if (avLuminance > TOO_HIGH_LUMINANCE)
  {
    ++m_tooHighLuminanceInARow;
  }
  else
  {
    m_tooLowLuminanceInARow = 0;
  }
  if (avLuminance < TOO_LOW_LUMINANCE)
  {
    ++m_tooLowLuminanceInARow;
  }
  else
  {
    m_tooHighLuminanceInARow = 0;
  }
}

auto AdaptiveExposure::GetTargetExposureLerpFactor() const -> float
{
  constexpr float NORMAL_EXPOSURE_LERP_FACTOR = 0.08F;
  constexpr float TOO_HIGH_EXPOSURE_LERP_FACTOR = 0.6F;
  constexpr float TOO_LOW_EXPOSURE_LERP_FACTOR = 0.3F;

  constexpr uint32_t MAX_TOO_LOW_EXPOSURE_IN_A_ROW = 100000;
  constexpr uint32_t MAX_TOO_HIGH_EXPOSURE_IN_A_ROW = 50000;

  if (m_tooLowLuminanceInARow > MAX_TOO_LOW_EXPOSURE_IN_A_ROW)
  {
    return TOO_LOW_EXPOSURE_LERP_FACTOR;
  }
  if (m_tooHighLuminanceInARow > MAX_TOO_HIGH_EXPOSURE_IN_A_ROW)
  {
    return TOO_HIGH_EXPOSURE_LERP_FACTOR;
  }
  return NORMAL_EXPOSURE_LERP_FACTOR;
}

#if __cplusplus <= 201402L
} // namespace UTILS
} // namespace GOOM
#else
} // namespace GOOM::UTILS
#endif
