#pragma once

#include "filter_settings.h"
#include "utils/timer.h"

#include <cstdint>

namespace GOOM
{

namespace UTILS::MATH
{
class IGoomRand;
}

namespace VISUAL_FX::FILTERS
{

class ExtraEffect
{
public:
  explicit ExtraEffect(const UTILS::MATH::IGoomRand& goomRand,
                       float probabilityOfEffect,
                       float probabilityOfRepeatEffect,
                       uint32_t effectOffTime) noexcept;

  void UpdateTimer();
  void UpdateEffect();
  void SetEffect(bool value);
  void EffectUpdateActivated();
  [[nodiscard]] auto IsTurnedOn() const -> bool;

private:
  const UTILS::MATH::IGoomRand& m_goomRand;

  const float m_probabilityOfEffect;
  const float m_probabilityOfRepeatEffect;
  bool m_effectTurnedOn = false;
  UTILS::Timer m_effectOffTimer;
  bool m_effectOffTimerResetPending = false;
};

class ExtraEffects
{
public:
  explicit ExtraEffects(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  void SetFilterSettingsDefaults(ZoomFilterSettings& filterSettings);
  void UpdateFilterSettings(ZoomFilterSettings& filterSettings) const;

  void UpdateTimers();
  void UpdateEffects();
  void EffectsUpdatesActivated();

  void SetHypercosOverlayEffect(HypercosOverlay value);
  void TurnPlaneEffectOn();

private:
  const UTILS::MATH::IGoomRand& m_goomRand;

  HypercosOverlay m_hypercosOverlayEffect;

  static constexpr float PROB_BLOCKY_WAVY_EFFECT = 0.3F;
  static constexpr float PROB_REPEAT_BLOCKY_WAVY_EFFECT = 0.9F;
  static constexpr uint32_t BLOCKY_WAVY_EFFECT_OFF_TIME = 100;
  ExtraEffect m_blockyWavyEffect;

  static constexpr float PROB_IMAGE_VELOCITY_EFFECT = 0.1F;
  static constexpr float PROB_REPEAT_IMAGE_VELOCITY_EFFECT = 0.9F;
  static constexpr uint32_t IMAGE_VELOCITY_EFFECT_OFF_TIME = 100;
  ExtraEffect m_imageVelocityEffect;

  static constexpr float PROB_NOISE_EFFECT = 0.1F;
  static constexpr float PROB_REPEAT_NOISE_EFFECT = 0.0F;
  static constexpr uint32_t NOISE_EFFECT_OFF_TIME = 100;
  ExtraEffect m_noiseEffect;

  static constexpr float PROB_PLANE_EFFECT = 0.8F;
  static constexpr float PROB_REPEAT_PLANE_EFFECT = 0.3F;
  static constexpr uint32_t PLANE_EFFECT_OFF_TIME = 100;
  ExtraEffect m_planeEffect;

  static constexpr float PROB_TAN_EFFECT = 0.2F;
  static constexpr float PROB_REPEAT_TAN_EFFECT = 0.1F;
  static constexpr uint32_t TAN_EFFECT_OFF_TIME = 100;
  ExtraEffect m_tanEffect;
};

} // namespace VISUAL_FX::FILTERS
} // namespace GOOM
