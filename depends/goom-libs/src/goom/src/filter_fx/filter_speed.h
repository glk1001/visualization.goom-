#pragma once

#include "goom_config.h"

#include <algorithm>
#include <cstdint>

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
  static constexpr uint32_t MAX_VITESSE = 128;

public:
  static constexpr uint32_t STOP_SPEED      = MAX_VITESSE;
  static constexpr uint32_t FASTEST_SPEED   = 0;
  static constexpr uint32_t DEFAULT_VITESSE = 127;

  [[nodiscard]] constexpr auto GetVitesse() const noexcept -> uint32_t;
  constexpr auto SetVitesse(uint32_t val) noexcept -> void;
  constexpr auto SetDefault() noexcept -> void;
  constexpr auto GoSlowerBy(uint32_t val) noexcept -> void;

  [[nodiscard]] constexpr auto GetReverseVitesse() const noexcept -> bool;
  constexpr auto SetReverseVitesse(bool val) noexcept -> void;
  constexpr auto ToggleReverseVitesse() noexcept -> void;

  [[nodiscard]] constexpr auto GetRelativeSpeed() const noexcept -> float;

private:
  uint32_t m_vitesse    = DEFAULT_VITESSE;
  bool m_reverseVitesse = true;
};

constexpr auto Vitesse::SetDefault() noexcept -> void
{
  m_vitesse        = DEFAULT_VITESSE;
  m_reverseVitesse = true;
}

constexpr auto Vitesse::GetReverseVitesse() const noexcept -> bool
{
  return m_reverseVitesse;
}

constexpr auto Vitesse::SetReverseVitesse(const bool val) noexcept -> void
{
  m_reverseVitesse = val;
}

constexpr auto Vitesse::ToggleReverseVitesse() noexcept -> void
{
  m_reverseVitesse = not m_reverseVitesse;
}

constexpr auto Vitesse::GetVitesse() const noexcept -> uint32_t
{
  return m_vitesse;
}

constexpr auto Vitesse::SetVitesse(const uint32_t val) noexcept -> void
{
  Expects(FASTEST_SPEED <= val);
  Expects(val <= STOP_SPEED);
  m_vitesse = val;
}

constexpr auto Vitesse::GoSlowerBy(const uint32_t val) noexcept -> void
{
  SetVitesse(std::clamp(m_vitesse + val, FASTEST_SPEED, STOP_SPEED));
}

constexpr auto Vitesse::GetRelativeSpeed() const noexcept -> float
{
  const auto speed =
      static_cast<float>(static_cast<int32_t>(m_vitesse) - static_cast<int32_t>(MAX_VITESSE)) /
      static_cast<float>(MAX_VITESSE);

  return m_reverseVitesse ? -speed : +speed;
}

} // namespace GOOM::FILTER_FX
