module;

#include <cstdint>

export module Goom.FilterFx.AfterEffects.AfterEffectsStates;

import Goom.FilterFx.AfterEffects.TheEffects.Rotation;
import Goom.FilterFx.AfterEffects.AfterEffectsTypes;
import Goom.Utils.EnumUtils;
import Goom.Utils.GoomTime;
import Goom.Utils.Timer;
import Goom.Utils.Math.GoomRandBase;
import Goom.Lib.GoomTypes;

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

class AfterEffectState
{
public:
  struct AfterEffectProperties
  {
    float probabilityOfEffectRepeated;
    uint32_t effectOffTime;
  };

  AfterEffectState(const UTILS::GoomTime& goomTime,
                   const UTILS::MATH::IGoomRand& goomRand,
                   bool turnedOn,
                   const AfterEffectProperties& effectProperties) noexcept;

  void UpdateState(float effectProbability);
  void SetState(bool value);
  void CheckPendingOffTimerReset();

  [[nodiscard]] auto IsTurnedOn() const -> bool;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  float m_probabilityOfEffectRepeated;
  bool m_turnedOn;
  UTILS::Timer m_offTimer;
  bool m_pendingOffTimerReset = false;
};

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS

export namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

enum class HypercosOverlayMode : UnderlyingEnumType
{
  NONE,
  MODE0,
  MODE1,
  MODE2,
  MODE3,
};

class AfterEffectsStates
{
public:
  using AfterEffectsOffTimeMap     = UTILS::EnumMap<AfterEffectsTypes, uint32_t>;
  using AfterEffectsProbabilityMap = UTILS::EnumMap<AfterEffectsTypes, float>;

  AfterEffectsStates(const UTILS::GoomTime& goomTime,
                     const UTILS::MATH::IGoomRand& goomRand,
                     const AfterEffectsProbabilityMap& repeatProbabilities,
                     const AfterEffectsOffTimeMap& offTimes) noexcept;
  AfterEffectsStates(const AfterEffectsStates&) noexcept = delete;
  AfterEffectsStates(AfterEffectsStates&&) noexcept      = delete;
  ~AfterEffectsStates() noexcept; // NOLINT(performance-trivially-destructible)
  auto operator=(const AfterEffectsStates&) -> AfterEffectsStates& = delete;
  auto operator=(AfterEffectsStates&&) -> AfterEffectsStates&      = delete;

  auto SetDefaults() -> void;
  auto CheckForPendingOffTimers() -> void;

  struct AfterEffectsProbabilities
  {
    UTILS::MATH::Weights<HypercosOverlayMode> hypercosModeWeights;
    AfterEffectsProbabilityMap probabilities{};
  };
  auto ResetAllStates(const AfterEffectsProbabilities& effectsProbabilities) -> void;
  auto ResetStandardStates(const AfterEffectsProbabilities& effectsProbabilities) -> void;

  using AfterEffectsActiveMap = UTILS::EnumMap<AfterEffectsTypes, bool>;
  struct AfterEffectsSettings
  {
    HypercosOverlayMode hypercosOverlayMode{};
    AfterEffectsActiveMap isActive{};
    RotationAdjustments rotationAdjustments;
  };
  auto UpdateAfterEffectsSettingsFromStates(AfterEffectsSettings& afterEffectsSettings) const
      -> void;

  auto TurnPlaneEffectOn() -> void;

private:
  HypercosOverlayMode m_hypercosOverlayMode = HypercosOverlayMode::NONE;
  AfterEffectState m_hypercosOverlayEffect;
  AfterEffectState m_imageVelocityEffect;
  AfterEffectState m_noiseEffect;
  AfterEffectState m_planeEffect;
  AfterEffectState m_rotationEffect;
  AfterEffectState m_tanEffect;
  AfterEffectState m_xyLerpEffect;
};

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
