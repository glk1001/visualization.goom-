#pragma once

#include "draw/goom_draw.h"
#include "utils/goom_rand.h"

#include <cstdint>

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

class AdaptiveExposure
{
public:
  static constexpr float MIN_EXPOSURE = 0.01F;
  static constexpr float MAX_EXPOSURE = 100.0F;
  static constexpr float DEFAULT_EXPOSURE = 1.0F;

  explicit AdaptiveExposure(const DRAW::IGoomDraw& goomDraw, UTILS::IGoomRand& goomRand) noexcept;

  void Start();

  void UpdateExposure();
  [[nodiscard]] auto GetCurrentExposure() const -> float;

private:
  const DRAW::IGoomDraw& m_goomDraw;
  UTILS::IGoomRand& m_goomRand;
  float m_currentExposure = DEFAULT_EXPOSURE;

  uint32_t m_tooHighLuminanceInARow = 0;
  uint32_t m_tooLowLuminanceInARow = 0;
  void UpdateLuminanceCounters(float avLuminance);

  [[nodiscard]] auto GetTargetExposureLerpFactor() const -> float;

  const UTILS::IGoomRand::NumberRange<int32_t> m_exposureSampleWidthRange;
  const UTILS::IGoomRand::NumberRange<int32_t> m_exposureSampleHeightRange;
  [[nodiscard]] auto GetAverageLuminanceOfSpotSamples() const -> float;
};

inline auto AdaptiveExposure::GetCurrentExposure() const -> float
{
  return m_currentExposure;
}

#if __cplusplus <= 201402L
} // namespace SHADERS
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::SHADERS
#endif
