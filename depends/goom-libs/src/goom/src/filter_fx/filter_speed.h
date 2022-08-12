#pragma once

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
  static constexpr int32_t MAX_VITESSE = 128;

public:
  static constexpr int32_t STOP_SPEED      = MAX_VITESSE;
  static constexpr int32_t FASTEST_SPEED   = 0;
  static constexpr int32_t DEFAULT_VITESSE = 127;

  [[nodiscard]] constexpr auto GetVitesse() const noexcept -> int32_t;
  constexpr auto SetVitesse(int32_t val) noexcept -> void;
  constexpr auto SetDefault() noexcept -> void;
  constexpr auto GoSlowerBy(int32_t val) noexcept -> void;

  [[nodiscard]] constexpr auto GetReverseVitesse() const noexcept -> bool;
  constexpr auto SetReverseVitesse(bool val) noexcept -> void;
  constexpr auto ToggleReverseVitesse() noexcept -> void;

  [[nodiscard]] constexpr auto GetRelativeSpeed() const noexcept -> float;

private:
  int32_t m_vitesse     = DEFAULT_VITESSE;
  bool m_reverseVitesse = true;
};

constexpr auto Vitesse::SetDefault() noexcept -> void
{
  m_vitesse        = DEFAULT_VITESSE;
  m_reverseVitesse = true;
}

constexpr auto Vitesse::GetVitesse() const noexcept -> int32_t
{
  return m_vitesse;
}

constexpr auto Vitesse::SetVitesse(const int32_t val) noexcept -> void
{
  m_vitesse = std::clamp(val, FASTEST_SPEED, STOP_SPEED);
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
  m_reverseVitesse = !m_reverseVitesse;
}

constexpr auto Vitesse::GoSlowerBy(const int32_t val) noexcept -> void
{
  SetVitesse(m_vitesse + val);
}

constexpr auto Vitesse::GetRelativeSpeed() const noexcept -> float
{
  const auto speed = static_cast<float>(m_vitesse - MAX_VITESSE) / static_cast<float>(MAX_VITESSE);
  return m_reverseVitesse ? -speed : +speed;
}

} // namespace GOOM::FILTER_FX
