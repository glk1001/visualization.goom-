#pragma once

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
  static constexpr float MIN_EXPOSURE = 0.10F;
  static constexpr float MAX_EXPOSURE = 50.0F;
  static constexpr float DEFAULT_EXPOSURE = 1.0F;

  AdaptiveExposure() noexcept = default;

  void Start();

  void UpdateExposure(float averageLuminance);
  [[nodiscard]] auto GetCurrentExposure() const -> float;

private:
  float m_currentExposure = DEFAULT_EXPOSURE;

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
} // namespace SHADERS
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::SHADERS
#endif
