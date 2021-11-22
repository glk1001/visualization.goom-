#pragma once

#include <cstdint>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
namespace FX_UTILS
{
#else
namespace GOOM::VISUAL_FX::FX_UTILS
{
#endif

class AdaptiveExposure
{
public:
  static constexpr float MIN_EXPOSURE = 0.01F;
  static constexpr float MAX_EXPOSURE = 100.0F;
  static constexpr float INITIAL_EXPOSURE = MIN_EXPOSURE;

  AdaptiveExposure() noexcept = default;

  void Start();

  void UpdateAverageLuminance(float averageLuminance);
  [[nodiscard]] auto GetCurrentExposure() const -> float;

private:
  float m_currentExposure = INITIAL_EXPOSURE;

  uint32_t m_tooHighLuminanceInARow = 0;
  uint32_t m_tooLowLuminanceInARow = 0;
  void UpdateLuminanceCounters(float avLuminance);

  [[nodiscard]] auto GetTargetExposureLerpFactor() const -> float;
};

inline auto AdaptiveExposure::GetCurrentExposure() const -> float
{
  return m_currentExposure;
}

#if __cplusplus <= 201402L
} // namespace FX_UTILS
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::FX_UTILS
#endif
