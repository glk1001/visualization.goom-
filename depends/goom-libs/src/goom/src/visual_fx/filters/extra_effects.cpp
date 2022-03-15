#include "extra_effects.h"

#include "filter_settings.h"
#include "rotation.h"
#include "utils/math/goom_rand_base.h"
#include "utils/timer.h"

#include <memory>

namespace GOOM::VISUAL_FX::FILTERS
{

using UTILS::Timer;
using UTILS::MATH::IGoomRand;

static constexpr bool NO_EXTRA_EFFECTS = false;

static constexpr HypercosOverlay DEFAULT_HYPERCOS_OVERLAY = HypercosOverlay::NONE;
static constexpr bool DEFAULT_BLOCKY_WAVY_NOISE_EFFECT = false;
static constexpr bool DEFAULT_IMAGE_VELOCITY_EFFECT = false;
static constexpr bool DEFAULT_NOISE_EFFECT = false;
static constexpr bool DEFAULT_PLANE_EFFECT = false;
static constexpr bool DEFAULT_TAN_EFFECT = false;

class ExtraEffects::ExtraEffect
{
public:
  ExtraEffect(const IGoomRand& goomRand,
              bool defaultValue,
              float probabilityOfEffect,
              float probabilityOfRepeatEffect,
              uint32_t effectOffTime) noexcept;

  void UpdateTimer();
  void UpdateEffect();
  void SetEffect(bool value);
  void EffectUpdateActivated();

  [[nodiscard]] auto IsTurnedOn() const -> bool;

private:
  const IGoomRand& m_goomRand;
  const float m_probabilityOfEffect;
  const float m_probabilityOfRepeatEffect;
  bool m_effectTurnedOn;
  Timer m_effectOffTimer;
  bool m_effectOffTimerResetPending = false;
};

ExtraEffects::ExtraEffects(const IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand},
    m_hypercosOverlayEffect{DEFAULT_HYPERCOS_OVERLAY},
    m_rotationEffect{false},
    m_blockyWavyEffect{std::make_unique<ExtraEffect>(goomRand,
                                                     DEFAULT_BLOCKY_WAVY_NOISE_EFFECT,
                                                     PROB_BLOCKY_WAVY_EFFECT,
                                                     PROB_REPEAT_BLOCKY_WAVY_EFFECT,
                                                     BLOCKY_WAVY_EFFECT_OFF_TIME)},
    m_imageVelocityEffect{std::make_unique<ExtraEffect>(goomRand,
                                                        DEFAULT_IMAGE_VELOCITY_EFFECT,
                                                        PROB_IMAGE_VELOCITY_EFFECT,
                                                        PROB_REPEAT_IMAGE_VELOCITY_EFFECT,
                                                        IMAGE_VELOCITY_EFFECT_OFF_TIME)},
    m_noiseEffect{std::make_unique<ExtraEffect>(goomRand,
                                                DEFAULT_NOISE_EFFECT,
                                                PROB_NOISE_EFFECT,
                                                PROB_REPEAT_NOISE_EFFECT,
                                                NOISE_EFFECT_OFF_TIME)},
    m_planeEffect{std::make_unique<ExtraEffect>(goomRand,
                                                DEFAULT_PLANE_EFFECT,
                                                PROB_PLANE_EFFECT,
                                                PROB_REPEAT_PLANE_EFFECT,
                                                PLANE_EFFECT_OFF_TIME)},
    m_tanEffect{std::make_unique<ExtraEffect>(
        goomRand, DEFAULT_TAN_EFFECT, PROB_TAN_EFFECT, PROB_REPEAT_TAN_EFFECT, TAN_EFFECT_OFF_TIME)}
{
}

ExtraEffects::~ExtraEffects() noexcept = default;

void ExtraEffects::UpdateFilterSettings(ZoomFilterSettings& filterSettings) const
{
  filterSettings.filterEffectsSettings.hypercosOverlay = m_hypercosOverlayEffect;
  filterSettings.filterEffectsSettings.rotationEffect = m_rotationEffect;

  filterSettings.filterColorSettings.blockyWavy = m_blockyWavyEffect->IsTurnedOn();
  filterSettings.filterEffectsSettings.imageVelocityEffect = m_imageVelocityEffect->IsTurnedOn();
  filterSettings.filterEffectsSettings.noiseEffect = m_noiseEffect->IsTurnedOn();
  filterSettings.filterEffectsSettings.planeEffect = m_planeEffect->IsTurnedOn();
  filterSettings.filterEffectsSettings.tanEffect = m_tanEffect->IsTurnedOn();
}

void ExtraEffects::TurnPlaneEffectOn()
{
  if constexpr (NO_EXTRA_EFFECTS)
  {
    return;
  }

  m_planeEffect->SetEffect(true);
}

void ExtraEffects::SetDefaults()
{
  m_hypercosOverlayEffect = DEFAULT_HYPERCOS_OVERLAY;
  m_rotationEffect = false;
}

void ExtraEffects::UpdateTimers()
{
  if constexpr (NO_EXTRA_EFFECTS)
  {
    return;
  }

  m_blockyWavyEffect->UpdateTimer();
  m_imageVelocityEffect->UpdateTimer();
  m_noiseEffect->UpdateTimer();
  m_planeEffect->UpdateTimer();
  m_tanEffect->UpdateTimer();
}

void ExtraEffects::UpdateAllEffects(const HypercosOverlay value, const float rotateProbability)
{
  if constexpr (NO_EXTRA_EFFECTS)
  {
    return;
  }

  m_hypercosOverlayEffect = value;
  m_rotationEffect = m_goomRand.ProbabilityOf(rotateProbability);

  UpdateEffects();
}

void ExtraEffects::UpdateEffects()
{
  if constexpr (NO_EXTRA_EFFECTS)
  {
    return;
  }

  m_blockyWavyEffect->UpdateEffect();
  m_imageVelocityEffect->UpdateEffect();
  m_noiseEffect->UpdateEffect();
  m_planeEffect->UpdateEffect();
  m_tanEffect->UpdateEffect();
}

void ExtraEffects::EffectsUpdatesActivated()
{
  if constexpr (NO_EXTRA_EFFECTS)
  {
    return;
  }

  m_blockyWavyEffect->EffectUpdateActivated();
  m_imageVelocityEffect->EffectUpdateActivated();
  m_noiseEffect->EffectUpdateActivated();
  m_planeEffect->EffectUpdateActivated();
  m_tanEffect->EffectUpdateActivated();
}

inline ExtraEffects::ExtraEffect::ExtraEffect(const UTILS::MATH::IGoomRand& goomRand,
                                              const bool defaultValue,
                                              const float probabilityOfEffect,
                                              const float probabilityOfRepeatEffect,
                                              const uint32_t effectOffTime) noexcept
  : m_goomRand{goomRand},
    m_probabilityOfEffect{probabilityOfEffect},
    m_probabilityOfRepeatEffect{probabilityOfRepeatEffect},
    m_effectTurnedOn{defaultValue},
    m_effectOffTimer{effectOffTime, true}
{
}

inline void ExtraEffects::ExtraEffect::UpdateTimer()
{
  m_effectOffTimer.Increment();
}

inline void ExtraEffects::ExtraEffect::UpdateEffect()
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

inline void ExtraEffects::ExtraEffect::SetEffect(const bool value)
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

inline void ExtraEffects::ExtraEffect::EffectUpdateActivated()
{
  if (!m_effectOffTimerResetPending)
  {
    return;
  }

  // Wait a while before allowing effect back on.
  m_effectOffTimer.ResetToZero();
  m_effectOffTimerResetPending = false;
}

inline auto ExtraEffects::ExtraEffect::IsTurnedOn() const -> bool
{
  return m_effectTurnedOn;
}

} // namespace GOOM::VISUAL_FX::FILTERS
