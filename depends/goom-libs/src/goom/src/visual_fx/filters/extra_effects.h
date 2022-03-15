#pragma once

#include "filter_settings.h"
#include "rotation.h"

#include <cstdint>
#include <memory>

namespace GOOM
{

namespace UTILS::MATH
{
class IGoomRand;
}

namespace VISUAL_FX::FILTERS
{

class ExtraEffects
{
public:
  explicit ExtraEffects(const UTILS::MATH::IGoomRand& goomRand) noexcept;
  ExtraEffects(const ExtraEffects&) noexcept = delete;
  ExtraEffects(ExtraEffects&&) noexcept = delete;
  ~ExtraEffects() noexcept;
  auto operator=(const ExtraEffects&) -> ExtraEffects& = delete;
  auto operator=(ExtraEffects&&) -> ExtraEffects& = delete;

  void SetDefaults();
  void TurnPlaneEffectOn();
  void UpdateTimers();
  void UpdateAllEffects(HypercosOverlay value, float rotateProbability);
  void UpdateEffects();
  void EffectsUpdatesActivated();
  void UpdateFilterSettings(ZoomFilterSettings& filterSettings) const;

private:
  const UTILS::MATH::IGoomRand& m_goomRand;

  HypercosOverlay m_hypercosOverlayEffect;
  bool m_rotationEffect;

  class ExtraEffect;

  static constexpr float PROB_BLOCKY_WAVY_EFFECT = 0.3F;
  static constexpr float PROB_REPEAT_BLOCKY_WAVY_EFFECT = 0.9F;
  static constexpr uint32_t BLOCKY_WAVY_EFFECT_OFF_TIME = 100;
  std::unique_ptr<ExtraEffect> m_blockyWavyEffect;

  static constexpr float PROB_IMAGE_VELOCITY_EFFECT = 0.1F;
  static constexpr float PROB_REPEAT_IMAGE_VELOCITY_EFFECT = 0.9F;
  static constexpr uint32_t IMAGE_VELOCITY_EFFECT_OFF_TIME = 100;
  std::unique_ptr<ExtraEffect> m_imageVelocityEffect;

  static constexpr float PROB_NOISE_EFFECT = 0.1F;
  static constexpr float PROB_REPEAT_NOISE_EFFECT = 0.0F;
  static constexpr uint32_t NOISE_EFFECT_OFF_TIME = 100;
  std::unique_ptr<ExtraEffect> m_noiseEffect;

  static constexpr float PROB_PLANE_EFFECT = 0.8F;
  static constexpr float PROB_REPEAT_PLANE_EFFECT = 0.3F;
  static constexpr uint32_t PLANE_EFFECT_OFF_TIME = 100;
  std::unique_ptr<ExtraEffect> m_planeEffect;

  static constexpr float PROB_TAN_EFFECT = 0.2F;
  static constexpr float PROB_REPEAT_TAN_EFFECT = 0.1F;
  static constexpr uint32_t TAN_EFFECT_OFF_TIME = 100;
  std::unique_ptr<ExtraEffect> m_tanEffect;
};

} // namespace VISUAL_FX::FILTERS
} // namespace GOOM
