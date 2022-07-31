#pragma once

#include "filter_effects/rotation.h"
#include "goom/goom_graphic.h"
#include "point2d.h"

#include <cstdint>
#include <memory>

namespace GOOM::FILTER_FX
{

// 128 = vitesse nule...
// 256 = en arriere
//   hyper vite.. * * 0 = en avant hype vite.
// 128 = zero speed
// 256 = reverse
//   super fast ... 0 = forward quickly.
class Vitesse
{
  static constexpr int32_t MAX_VITESSE = 128;

public:
  static constexpr int32_t STOP_SPEED      = MAX_VITESSE;
  static constexpr int32_t FASTEST_SPEED   = 0;
  static constexpr int32_t DEFAULT_VITESSE = 127;

  [[nodiscard]] auto GetVitesse() const -> int32_t { return m_vitesse; };
  auto SetVitesse(int32_t val) -> void;
  auto SetDefault() -> void;
  auto GoSlowerBy(int32_t val) -> void;

  [[nodiscard]] auto GetReverseVitesse() const -> bool { return m_reverseVitesse; }
  auto SetReverseVitesse(const bool val) -> void { m_reverseVitesse = val; }
  auto ToggleReverseVitesse() -> void { m_reverseVitesse = !m_reverseVitesse; }

  [[nodiscard]] auto GetRelativeSpeed() const -> float;

private:
  int32_t m_vitesse     = DEFAULT_VITESSE;
  bool m_reverseVitesse = true;
};

enum class HypercosOverlay
{
  NONE,
  MODE0,
  MODE1,
  MODE2,
  MODE3,
  _num // unused, and marks the enum end
};

struct ZoomFilterBufferSettings
{
  int32_t tranLerpIncrement;
  float tranLerpToMaxSwitchMult;
};

struct ZoomFilterColorSettings
{
  bool blockyWavy;
};

class ISpeedCoefficientsEffect;

struct ZoomFilterEffectsSettings
{
  Vitesse vitesse;
  HypercosOverlay hypercosOverlay;

  float maxSpeedCoeff;
  std::shared_ptr<ISpeedCoefficientsEffect> speedCoefficientsEffect;
  FILTER_EFFECTS::RotationAdjustments rotationAdjustments;

  Point2dInt zoomMidpoint; // milieu de l'effet

  bool imageVelocityEffect;
  bool noiseEffect; // ajoute un bruit a la transformation
  bool planeEffect;
  bool rotationEffect;
  bool tanEffect;
};

struct ZoomFilterSettings
{
  ZoomFilterEffectsSettings filterEffectsSettings{};
  ZoomFilterBufferSettings filterBufferSettings{};
  ZoomFilterColorSettings filterColorSettings{};
};

inline auto Vitesse::SetVitesse(const int32_t val) -> void
{
  m_vitesse = std::clamp(val, FASTEST_SPEED, STOP_SPEED);
}

inline auto Vitesse::SetDefault() -> void
{
  m_vitesse        = DEFAULT_VITESSE;
  m_reverseVitesse = true;
}

inline auto Vitesse::GoSlowerBy(const int32_t val) -> void
{
  SetVitesse(m_vitesse + val);
}

inline auto Vitesse::GetRelativeSpeed() const -> float
{
  const auto speed = static_cast<float>(m_vitesse - MAX_VITESSE) / static_cast<float>(MAX_VITESSE);
  return m_reverseVitesse ? -speed : +speed;
}

} // namespace GOOM::FILTER_FX
