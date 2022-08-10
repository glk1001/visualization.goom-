#pragma once

#include "rotation.h"
#include "utils/math/goom_rand_base.h"

#include <cstdint>
#include <memory>

namespace GOOM::UTILS::MATH
{
class IGoomRand;
}

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

enum class HypercosOverlay
{
  NONE,
  MODE0,
  MODE1,
  MODE2,
  MODE3,
  _num // unused, and marks the enum end
};

struct AfterEffectsSettings
{
  bool blockyWavy;

  HypercosOverlay hypercosOverlay;
  bool imageVelocityEffect;
  bool noiseEffect; // ajoute un bruit a la transformation
  bool planeEffect;
  bool rotationEffect;
  bool tanEffect;
  bool xyLerpEffect;

  RotationAdjustments rotationAdjustments;
};

struct AfterEffectsProbabilities
{
  GOOM::UTILS::MATH::Weights<HypercosOverlay> hypercosWeights;
  struct Probabilities
  {
    float blockWavyProbability;
    float imageVelocityProbability;
    float noiseProbability;
    float planeProbability;
    float rotateProbability;
    float tanEffectProbability;
    float xyLerpEffectProbability;
  };
  Probabilities probabilities;
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
  auto ResetAllStates(const AfterEffectsProbabilities& effectsProbabilities) -> void;
  auto ResetStandardStates(const AfterEffectsProbabilities& effectsProbabilities) -> void;
  auto CheckForPendingOffTimers() -> void;
  auto UpdateFilterSettingsFromStates(AfterEffectsSettings& afterEffectsSettings) const -> void;

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

  static constexpr float PROB_REPEAT_XY_LERP_EFFECT = 0.1F;
  static constexpr uint32_t XY_LERP_EFFECT_OFF_TIME = 100;
  std::unique_ptr<EffectState> m_xyLerpEffect;
};

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
