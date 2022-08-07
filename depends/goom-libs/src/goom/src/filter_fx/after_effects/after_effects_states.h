#pragma once

#include "filter_fx/filter_settings.h"

#include <cstdint>
#include <memory>

namespace GOOM::UTILS::MATH
{
class IGoomRand;
}

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

struct AfterEffectsProbabilities
{
  float blockWavyProbability;
  float imageVelocityProbability;
  float noiseProbability;
  float planeProbability;
  float rotateProbability;
  float tanEffectProbability;
};

class AfterEffectsStates
{
public:
  explicit AfterEffectsStates(const GOOM::UTILS::MATH::IGoomRand& goomRand) noexcept;
  AfterEffectsStates(const AfterEffectsStates&) noexcept = delete;
  AfterEffectsStates(AfterEffectsStates&&) noexcept      = delete;
  ~AfterEffectsStates() noexcept;
  auto operator=(const AfterEffectsStates&) -> AfterEffectsStates& = delete;
  auto operator=(AfterEffectsStates&&) -> AfterEffectsStates&      = delete;

  auto TurnPlaneEffectOn() -> void;

  auto SetDefaults() -> void;
  auto UpdateTimers() -> void;
  auto ResetAllStates(HypercosOverlay value, const AfterEffectsProbabilities& effectsProbabilities)
      -> void;
  auto ResetStandardStates(const AfterEffectsProbabilities& effectsProbabilities) -> void;
  auto CheckForPendingOffTimers() -> void;
  auto UpdateFilterSettingsFromStates(ZoomFilterSettings& filterSettings) const -> void;

private:
  HypercosOverlay m_hypercosOverlayEffect;

  class EffectState;

  static constexpr float PROB_REPEAT_BLOCKY_WAVY_EFFECT = 0.9F;
  static constexpr uint32_t BLOCKY_WAVY_EFFECT_OFF_TIME = 100;
  std::unique_ptr<EffectState> m_blockyWavyEffect;

  static constexpr float PROB_REPEAT_IMAGE_VELOCITY_EFFECT = 0.9F;
  static constexpr uint32_t IMAGE_VELOCITY_EFFECT_OFF_TIME = 100;
  std::unique_ptr<EffectState> m_imageVelocityEffect;

  static constexpr float PROB_REPEAT_NOISE_EFFECT = 0.0F;
  static constexpr uint32_t NOISE_EFFECT_OFF_TIME = 100;
  std::unique_ptr<EffectState> m_noiseEffect;

  static constexpr float PROB_REPEAT_PLANE_EFFECT = 0.3F;
  static constexpr uint32_t PLANE_EFFECT_OFF_TIME = 100;
  std::unique_ptr<EffectState> m_planeEffect;

  static constexpr float PROB_REPEAT_ROTATION_EFFECT = 0.0F;
  static constexpr uint32_t ROTATION_EFFECT_OFF_TIME = 0;
  std::unique_ptr<EffectState> m_rotationEffect;

  static constexpr float PROB_REPEAT_TAN_EFFECT = 0.1F;
  static constexpr uint32_t TAN_EFFECT_OFF_TIME = 100;
  std::unique_ptr<EffectState> m_tanEffect;
};

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
