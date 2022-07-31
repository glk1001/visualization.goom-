#pragma once

#include "filter_fx/filter_settings.h"
#include "utils/propagate_const.h"

#include <cstdint>
#include <memory>

namespace GOOM
{

namespace UTILS::MATH
{
class IGoomRand;
}

namespace FILTER_FX::FILTER_EFFECTS
{

struct ExtraEffectsProbabilities
{
  float blockWavyProbability;
  float imageVelocityProbability;
  float noiseProbability;
  float planeProbability;
  float rotateProbability;
  float tanEffectProbability;
};

class ExtraEffectsStates
{
public:
  explicit ExtraEffectsStates(const UTILS::MATH::IGoomRand& goomRand) noexcept;
  ExtraEffectsStates(const ExtraEffectsStates&) noexcept = delete;
  ExtraEffectsStates(ExtraEffectsStates&&) noexcept      = delete;
  ~ExtraEffectsStates() noexcept;
  auto operator=(const ExtraEffectsStates&) -> ExtraEffectsStates& = delete;
  auto operator=(ExtraEffectsStates&&) -> ExtraEffectsStates&      = delete;

  auto TurnPlaneEffectOn() -> void;

  auto SetDefaults() -> void;
  auto UpdateTimers() -> void;
  auto ResetAllStates(HypercosOverlay value, const ExtraEffectsProbabilities& effectsProbabilities)
      -> void;
  auto ResetStandardStates(const ExtraEffectsProbabilities& effectsProbabilities) -> void;
  auto CheckForPendingOffTimers() -> void;
  auto UpdateFilterSettingsFromStates(ZoomFilterSettings& filterSettings) const -> void;

private:
  HypercosOverlay m_hypercosOverlayEffect;

  class EffectState;

  static constexpr float PROB_REPEAT_BLOCKY_WAVY_EFFECT = 0.9F;
  static constexpr uint32_t BLOCKY_WAVY_EFFECT_OFF_TIME = 100;
  std::experimental::propagate_const<std::unique_ptr<EffectState>> m_blockyWavyEffect;

  static constexpr float PROB_REPEAT_IMAGE_VELOCITY_EFFECT = 0.9F;
  static constexpr uint32_t IMAGE_VELOCITY_EFFECT_OFF_TIME = 100;
  std::experimental::propagate_const<std::unique_ptr<EffectState>> m_imageVelocityEffect;

  static constexpr float PROB_REPEAT_NOISE_EFFECT = 0.0F;
  static constexpr uint32_t NOISE_EFFECT_OFF_TIME = 100;
  std::experimental::propagate_const<std::unique_ptr<EffectState>> m_noiseEffect;

  static constexpr float PROB_REPEAT_PLANE_EFFECT = 0.3F;
  static constexpr uint32_t PLANE_EFFECT_OFF_TIME = 100;
  std::experimental::propagate_const<std::unique_ptr<EffectState>> m_planeEffect;

  static constexpr float PROB_REPEAT_ROTATION_EFFECT = 0.0F;
  static constexpr uint32_t ROTATION_EFFECT_OFF_TIME = 0;
  std::experimental::propagate_const<std::unique_ptr<EffectState>> m_rotationEffect;

  static constexpr float PROB_REPEAT_TAN_EFFECT = 0.1F;
  static constexpr uint32_t TAN_EFFECT_OFF_TIME = 100;
  std::experimental::propagate_const<std::unique_ptr<EffectState>> m_tanEffect;
};

} // namespace FILTER_FX::FILTER_EFFECTS
} // namespace GOOM
