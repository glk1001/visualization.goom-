#include "after_effects_states.h"

#include "filter_fx/filter_consts.h"
#include "utils/math/goom_rand_base.h"
#include "utils/timer.h"

#include <memory>

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

using UTILS::Timer;
using UTILS::MATH::IGoomRand;

static constexpr auto AFTER_EFFECTS_TURNED_OFF = ALL_FILTER_EFFECTS_TURNED_OFF;

static constexpr auto DEFAULT_HYPERCOS_OVERLAY      = HypercosOverlay::NONE;
static constexpr auto DEFAULT_BLOCKY_WAVY_EFFECT    = false;
static constexpr auto DEFAULT_IMAGE_VELOCITY_EFFECT = false;
static constexpr auto DEFAULT_NOISE_EFFECT          = false;
static constexpr auto DEFAULT_PLANE_EFFECT          = false;
static constexpr auto DEFAULT_ROTATION_EFFECT       = false;
static constexpr auto DEFAULT_TAN_EFFECT            = false;

class AfterEffectsStates::EffectState
{
public:
  EffectState(const IGoomRand& goomRand,
              bool defaultValue,
              float probabilityOfEffectRepeated,
              uint32_t effectOffTime) noexcept;

  void UpdateTimer();
  void UpdateState(float probabilityOfEffect);
  void SetState(bool value);
  void CheckPendingOffTimerReset();

  [[nodiscard]] auto IsTurnedOn() const -> bool;

private:
  const IGoomRand& m_goomRand;
  const float m_probabilityOfEffectRepeated;
  bool m_turnedOn;
  Timer m_offTimer;
  bool m_pendingOffTimerReset = false;
};

AfterEffectsStates::AfterEffectsStates(const IGoomRand& goomRand) noexcept
  : m_hypercosOverlayEffect{DEFAULT_HYPERCOS_OVERLAY},
    m_blockyWavyEffect{std::make_unique<EffectState>(goomRand,
                                                     DEFAULT_BLOCKY_WAVY_EFFECT,
                                                     PROB_REPEAT_BLOCKY_WAVY_EFFECT,
                                                     BLOCKY_WAVY_EFFECT_OFF_TIME)},
    m_imageVelocityEffect{std::make_unique<EffectState>(goomRand,
                                                        DEFAULT_IMAGE_VELOCITY_EFFECT,
                                                        PROB_REPEAT_IMAGE_VELOCITY_EFFECT,
                                                        IMAGE_VELOCITY_EFFECT_OFF_TIME)},
    m_noiseEffect{std::make_unique<EffectState>(
        goomRand, DEFAULT_NOISE_EFFECT, PROB_REPEAT_NOISE_EFFECT, NOISE_EFFECT_OFF_TIME)},
    m_planeEffect{std::make_unique<EffectState>(
        goomRand, DEFAULT_PLANE_EFFECT, PROB_REPEAT_PLANE_EFFECT, PLANE_EFFECT_OFF_TIME)},
    m_rotationEffect{std::make_unique<EffectState>(
        goomRand, DEFAULT_ROTATION_EFFECT, PROB_REPEAT_ROTATION_EFFECT, ROTATION_EFFECT_OFF_TIME)},
    m_tanEffect{std::make_unique<EffectState>(
        goomRand, DEFAULT_TAN_EFFECT, PROB_REPEAT_TAN_EFFECT, TAN_EFFECT_OFF_TIME)}
{
}

AfterEffectsStates::~AfterEffectsStates() noexcept = default;

auto AfterEffectsStates::UpdateFilterSettingsFromStates(
    AfterEffectsSettings& afterEffectsSettings) const -> void
{
  afterEffectsSettings.blockyWavy          = m_blockyWavyEffect->IsTurnedOn();
  afterEffectsSettings.hypercosOverlay     = m_hypercosOverlayEffect;
  afterEffectsSettings.imageVelocityEffect = m_imageVelocityEffect->IsTurnedOn();
  afterEffectsSettings.noiseEffect         = m_noiseEffect->IsTurnedOn();
  afterEffectsSettings.planeEffect         = m_planeEffect->IsTurnedOn();
  afterEffectsSettings.rotationEffect      = m_rotationEffect->IsTurnedOn();
  afterEffectsSettings.tanEffect           = m_tanEffect->IsTurnedOn();
}

auto AfterEffectsStates::TurnPlaneEffectOn() -> void
{
  if constexpr (AFTER_EFFECTS_TURNED_OFF)
  {
    return;
  }

  m_planeEffect->SetState(true);
}

auto AfterEffectsStates::SetDefaults() -> void
{
  m_hypercosOverlayEffect = DEFAULT_HYPERCOS_OVERLAY;
}

auto AfterEffectsStates::UpdateTimers() -> void
{
  if constexpr (AFTER_EFFECTS_TURNED_OFF)
  {
    return;
  }

  m_blockyWavyEffect->UpdateTimer();
  m_imageVelocityEffect->UpdateTimer();
  m_noiseEffect->UpdateTimer();
  m_planeEffect->UpdateTimer();
  m_rotationEffect->UpdateTimer();
  m_tanEffect->UpdateTimer();
}

auto AfterEffectsStates::ResetAllStates(const AfterEffectsProbabilities& effectsProbabilities)
    -> void
{
  if constexpr (AFTER_EFFECTS_TURNED_OFF)
  {
    return;
  }

  m_hypercosOverlayEffect = effectsProbabilities.hypercosWeights.GetRandomWeighted();

  ResetStandardStates(effectsProbabilities);
}

auto AfterEffectsStates::ResetStandardStates(const AfterEffectsProbabilities& effectsProbabilities)
    -> void
{
  if constexpr (AFTER_EFFECTS_TURNED_OFF)
  {
    return;
  }

  m_blockyWavyEffect->UpdateState(effectsProbabilities.probabilities.blockWavyProbability);
  m_imageVelocityEffect->UpdateState(effectsProbabilities.probabilities.imageVelocityProbability);
  m_noiseEffect->UpdateState(effectsProbabilities.probabilities.noiseProbability);
  m_planeEffect->UpdateState(effectsProbabilities.probabilities.planeProbability);
  m_rotationEffect->UpdateState(effectsProbabilities.probabilities.rotateProbability);
  m_tanEffect->UpdateState(effectsProbabilities.probabilities.tanEffectProbability);
}

auto AfterEffectsStates::CheckForPendingOffTimers() -> void
{
  if constexpr (AFTER_EFFECTS_TURNED_OFF)
  {
    return;
  }

  m_blockyWavyEffect->CheckPendingOffTimerReset();
  m_imageVelocityEffect->CheckPendingOffTimerReset();
  m_noiseEffect->CheckPendingOffTimerReset();
  m_planeEffect->CheckPendingOffTimerReset();
  m_rotationEffect->CheckPendingOffTimerReset();
  m_tanEffect->CheckPendingOffTimerReset();
}

inline AfterEffectsStates::EffectState::EffectState(const UTILS::MATH::IGoomRand& goomRand,
                                                    const bool defaultValue,
                                                    const float probabilityOfEffectRepeated,
                                                    const uint32_t effectOffTime) noexcept
  : m_goomRand{goomRand},
    m_probabilityOfEffectRepeated{probabilityOfEffectRepeated},
    m_turnedOn{defaultValue},
    m_offTimer{effectOffTime, true}
{
}

inline auto AfterEffectsStates::EffectState::UpdateTimer() -> void
{
  m_offTimer.Increment();
}

inline auto AfterEffectsStates::EffectState::UpdateState(const float probabilityOfEffect) -> void
{
  if (!m_offTimer.Finished())
  {
    return;
  }
  if (m_pendingOffTimerReset)
  {
    return;
  }

  SetState(m_goomRand.ProbabilityOf(probabilityOfEffect));
}

inline auto AfterEffectsStates::EffectState::SetState(const bool value) -> void
{
  const auto previouslyTurnedOn = m_turnedOn;
  m_turnedOn                    = value;

  if (previouslyTurnedOn && m_turnedOn)
  {
    m_turnedOn = m_goomRand.ProbabilityOf(m_probabilityOfEffectRepeated);
  }
  if (previouslyTurnedOn && (!m_turnedOn))
  {
    m_pendingOffTimerReset = true;
  }
}

inline auto AfterEffectsStates::EffectState::CheckPendingOffTimerReset() -> void
{
  if (!m_pendingOffTimerReset)
  {
    return;
  }

  // Wait a while before allowing effect back on.
  m_offTimer.ResetToZero();
  m_pendingOffTimerReset = false;
}

inline auto AfterEffectsStates::EffectState::IsTurnedOn() const -> bool
{
  return m_turnedOn;
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
