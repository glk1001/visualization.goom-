#include "adaptive_exposure.h"

#include "color/colorutils.h"

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
namespace SHADERS
{
#else
namespace GOOM::VISUAL_FX::SHADERS
{
#endif

using COLOR::GetLuma;

AdaptiveExposure::AdaptiveExposure(const DRAW::IGoomDraw& goomDraw,
                                   UTILS::IGoomRand& goomRand) noexcept
  : m_goomDraw{goomDraw},
    m_goomRand{goomRand},
    m_exposureSampleWidthRange{static_cast<int32_t>(goomDraw.GetScreenWidth() / 3),
                               static_cast<int32_t>(1 + ((2 * goomDraw.GetScreenWidth()) / 3))},
    m_exposureSampleHeightRange{static_cast<int32_t>(goomDraw.GetScreenHeight() / 3),
                                static_cast<int32_t>(1 + ((2 * goomDraw.GetScreenHeight()) / 3))}
{
}

void AdaptiveExposure::Start()
{
  m_tooHighLuminanceInARow = 0;
  m_tooLowLuminanceInARow = 0;
  m_currentExposure = DEFAULT_EXPOSURE;
}

void AdaptiveExposure::UpdateExposure()
{
  const float avLuminance = GetAverageLuminanceOfSpotSamples();

  UpdateLuminanceCounters(avLuminance);

  const float targetExposure = stdnew::clamp(1.0F / avLuminance, MIN_EXPOSURE, MAX_EXPOSURE);

  m_currentExposure =
      stdnew::lerp(m_currentExposure, targetExposure, GetTargetExposureLerpFactor());
}

void AdaptiveExposure::UpdateLuminanceCounters(const float avLuminance)
{
  constexpr float TOO_LOW_LUMINANCE = 0.1F;
  constexpr float TOO_HIGH_LUMINANCE = 2.0F;

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
  constexpr float NORMAL_EXPOSURE_LERP_FACTOR = 0.1F;
  constexpr float TOO_HIGH_EXPOSURE_LERP_FACTOR = 0.9F;
  constexpr float TOO_LOW_EXPOSURE_LERP_FACTOR = 0.5F;

  if (m_tooLowLuminanceInARow > 10)
  {
    return TOO_LOW_EXPOSURE_LERP_FACTOR;
  }
  if (m_tooHighLuminanceInARow > 5)
  {
    return TOO_HIGH_EXPOSURE_LERP_FACTOR;
  }
  return NORMAL_EXPOSURE_LERP_FACTOR;
}

auto AdaptiveExposure::GetAverageLuminanceOfSpotSamples() const -> float
{
  float totalLogLuminance = 0.0F;
  constexpr size_t NUM_SAMPLES_TO_DO = 10;
  size_t numNonZeroSamples = 0;
  for (size_t i = 0; i < NUM_SAMPLES_TO_DO; ++i)
  {
    const int32_t x = m_goomRand.GetRandInRange(m_exposureSampleWidthRange);
    const int32_t y = m_goomRand.GetRandInRange(m_exposureSampleHeightRange);
    const Pixel pixel = m_goomDraw.GetPixel(x, y);
    const float luma = GetLuma(pixel);
    if (luma > 0.0F)
    {
      totalLogLuminance += std::log(luma);
      ++numNonZeroSamples;
    }
  }

  if (0 == numNonZeroSamples)
  {
    constexpr float SMALL_LUM = 0.01F;
    return SMALL_LUM;
  }

  return std::exp(totalLogLuminance / static_cast<float>(numNonZeroSamples));
}

#if __cplusplus <= 201402L
} // namespace SHADERS
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::SHADERS
#endif
