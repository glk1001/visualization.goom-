#include "extra_effects.h"

#include "filter_settings.h"
#include "rotation.h"
#include "utils/math/goom_rand_base.h"

#include <memory>

namespace GOOM::VISUAL_FX::FILTERS
{

using UTILS::MATH::IGoomRand;

static constexpr bool DEFAULT_IMAGE_VELOCITY_EFFECT = false;
static constexpr bool DEFAULT_TAN_EFFECT = false;
static constexpr bool DEFAULT_PLANE_EFFECT = false;
static constexpr bool DEFAULT_NOISE_EFFECT = false;
static constexpr bool DEFAULT_BLOCKY_WAVY_NOISE_EFFECT = false;

ExtraEffects::ExtraEffects(const IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand},
    m_blockyWavyEffect{goomRand, PROB_BLOCKY_WAVY_EFFECT, PROB_REPEAT_BLOCKY_WAVY_EFFECT,
                       BLOCKY_WAVY_EFFECT_OFF_TIME},
    m_imageVelocityEffect{goomRand, PROB_IMAGE_VELOCITY_EFFECT, PROB_REPEAT_IMAGE_VELOCITY_EFFECT,
                          IMAGE_VELOCITY_EFFECT_OFF_TIME},
    m_noiseEffect{goomRand, PROB_NOISE_EFFECT, PROB_REPEAT_NOISE_EFFECT, NOISE_EFFECT_OFF_TIME},
    m_planeEffect{goomRand, PROB_PLANE_EFFECT, PROB_REPEAT_PLANE_EFFECT, PLANE_EFFECT_OFF_TIME},
    m_tanEffect{goomRand, PROB_TAN_EFFECT, PROB_REPEAT_TAN_EFFECT, TAN_EFFECT_OFF_TIME}
{
}

void ExtraEffects::TurnPlaneEffectOn()
{
  m_planeEffect.SetEffect(true);
}

void ExtraEffects::SetFilterSettingsDefaults(ZoomFilterSettings& filterSettings) const
{
  filterSettings.filterEffectsSettings.imageVelocityEffect = DEFAULT_IMAGE_VELOCITY_EFFECT;
  filterSettings.filterEffectsSettings.tanEffect = DEFAULT_TAN_EFFECT;
  filterSettings.filterEffectsSettings.planeEffect = DEFAULT_PLANE_EFFECT;
  filterSettings.filterEffectsSettings.noiseEffect = DEFAULT_NOISE_EFFECT;
  filterSettings.filterColorSettings.blockyWavy = DEFAULT_BLOCKY_WAVY_NOISE_EFFECT;

  filterSettings.filterEffectsSettings.rotation = std::make_shared<Rotation>(m_goomRand);
}

void ExtraEffects::UpdateFilterSettings(ZoomFilterSettings& filterSettings) const
{
  filterSettings.filterColorSettings.blockyWavy = m_blockyWavyEffect.IsTurnedOn();
  filterSettings.filterEffectsSettings.imageVelocityEffect = m_imageVelocityEffect.IsTurnedOn();
  filterSettings.filterEffectsSettings.noiseEffect = m_noiseEffect.IsTurnedOn();
  filterSettings.filterEffectsSettings.planeEffect = m_planeEffect.IsTurnedOn();
  filterSettings.filterEffectsSettings.tanEffect = m_tanEffect.IsTurnedOn();
}

void ExtraEffects::UpdateTimers()
{
  m_blockyWavyEffect.UpdateTimer();
  m_imageVelocityEffect.UpdateTimer();
  m_noiseEffect.UpdateTimer();
  m_planeEffect.UpdateTimer();
  m_tanEffect.UpdateTimer();
}

void ExtraEffects::UpdateEffects()
{
  m_blockyWavyEffect.UpdateEffect();
  m_imageVelocityEffect.UpdateEffect();
  m_noiseEffect.UpdateEffect();
  m_planeEffect.UpdateEffect();
  m_tanEffect.UpdateEffect();
}

void ExtraEffects::EffectsUpdatesActivated()
{
  m_blockyWavyEffect.EffectUpdateActivated();
  m_imageVelocityEffect.EffectUpdateActivated();
  m_noiseEffect.EffectUpdateActivated();
  m_planeEffect.EffectUpdateActivated();
  m_tanEffect.EffectUpdateActivated();
}

inline ExtraEffect::ExtraEffect(const UTILS::MATH::IGoomRand& goomRand,
                                const float probabilityOfEffect,
                                const float probabilityOfRepeatEffect,
                                const uint32_t effectOffTime) noexcept
  : m_goomRand{goomRand},
    m_probabilityOfEffect{probabilityOfEffect},
    m_probabilityOfRepeatEffect{probabilityOfRepeatEffect},
    m_effectOffTimer{effectOffTime, true}
{
}

inline void ExtraEffect::UpdateTimer()
{
  m_effectOffTimer.Increment();
}

inline void ExtraEffect::UpdateEffect()
{
  if (!m_effectOffTimer.Finished())
  {
    return;
  }
  if (m_effectOffTimerResetPending)
  {
    return;
  }

  SetEffect(m_goomRand.ProbabilityOf(m_probabilityOfEffect));
}

inline void ExtraEffect::SetEffect(const bool value)
{
  const bool previouslyTurnedOn = m_effectTurnedOn;
  m_effectTurnedOn = value;

  if (previouslyTurnedOn && m_effectTurnedOn)
  {
    m_effectTurnedOn = m_goomRand.ProbabilityOf(m_probabilityOfRepeatEffect);
  }
  if (previouslyTurnedOn && (!m_effectTurnedOn))
  {
    m_effectOffTimerResetPending = true;
  }
}

inline void ExtraEffect::EffectUpdateActivated()
{
  if (!m_effectOffTimerResetPending)
  {
    return;
  }

  // Wait a while before allowing effect back on.
  m_effectOffTimer.ResetToZero();
  m_effectOffTimerResetPending = false;
}

inline auto ExtraEffect::IsTurnedOn() const -> bool
{
  return m_effectTurnedOn;
}

} // namespace GOOM::VISUAL_FX::FILTERS
