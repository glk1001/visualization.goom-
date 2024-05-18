module;

#include "goom/goom_config.h"

#include <algorithm>
#include <cstdint>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#endif
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4296) // expression is always true
#endif

export module Goom.FilterFx.FilterSpeed;

export namespace GOOM::FILTER_FX
{

class Vitesse
{
public:
  static constexpr uint32_t STOP_SPEED        = 0U;
  static constexpr uint32_t SLOWEST_SPEED     = STOP_SPEED + 1U;
  static constexpr uint32_t SLOW_SPEED        = STOP_SPEED + 2U;
  static constexpr uint32_t CALM_SPEED        = STOP_SPEED + 4U;
  static constexpr uint32_t FAST_SPEED        = STOP_SPEED + 5U;
  static constexpr uint32_t FASTER_SPEED      = STOP_SPEED + 6U;
  static constexpr uint32_t EVEN_FASTER_SPEED = STOP_SPEED + 7U;
  static constexpr uint32_t MAXIMUM_SPEED     = STOP_SPEED + 10U;
  static constexpr uint32_t DEFAULT_SPEED     = SLOWEST_SPEED;

  [[nodiscard]] constexpr auto GetVitesse() const noexcept -> uint32_t;
  constexpr auto SetVitesse(uint32_t val) noexcept -> void;
  constexpr auto SetDefault() noexcept -> void;
  constexpr auto GoSlowerBy(uint32_t amount) noexcept -> void;
  [[nodiscard]] constexpr static auto GetFasterBy(uint32_t speed, uint32_t amount) noexcept
      -> uint32_t;
  [[nodiscard]] constexpr static auto GetSlowerBy(uint32_t speed, uint32_t amount) noexcept
      -> uint32_t;

  [[nodiscard]] constexpr auto GetReverseVitesse() const noexcept -> bool;
  constexpr auto SetReverseVitesse(bool val) noexcept -> void;
  constexpr auto ToggleReverseVitesse() noexcept -> void;

  [[nodiscard]] constexpr auto IsFasterThan(uint32_t otherSpeed) const noexcept -> bool;

  [[nodiscard]] constexpr auto GetRelativeSpeed() const noexcept -> float;

private:
  uint32_t m_vitesse    = DEFAULT_SPEED;
  bool m_reverseVitesse = true;
};

} // namespace GOOM::FILTER_FX

namespace GOOM::FILTER_FX
{

constexpr auto Vitesse::SetDefault() noexcept -> void
{
  m_vitesse        = DEFAULT_SPEED;
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
  Expects(val <= MAXIMUM_SPEED);
  Expects(val >= STOP_SPEED);

  m_vitesse = val;
}

constexpr auto Vitesse::GoSlowerBy(const uint32_t amount) noexcept -> void
{
  if (amount > m_vitesse)
  {
    m_vitesse = STOP_SPEED;
    return;
  }
  m_vitesse -= amount;

  Ensures(m_vitesse <= MAXIMUM_SPEED);
  Ensures(m_vitesse >= STOP_SPEED);
}

constexpr auto Vitesse::GetFasterBy(const uint32_t speed, const uint32_t amount) noexcept
    -> uint32_t
{
  return std::min(speed + amount, MAXIMUM_SPEED);
}

constexpr auto Vitesse::GetSlowerBy(const uint32_t speed, const uint32_t amount) noexcept
    -> uint32_t
{
  if (amount >= speed)
  {
    return STOP_SPEED;
  }
  return speed - amount;
}

constexpr auto Vitesse::IsFasterThan(const uint32_t otherSpeed) const noexcept -> bool
{
  return m_vitesse > otherSpeed;
}

constexpr auto Vitesse::GetRelativeSpeed() const noexcept -> float
{
  const auto relativeSpeed = static_cast<float>(m_vitesse) / static_cast<float>(MAXIMUM_SPEED);

  return m_reverseVitesse ? -relativeSpeed : +relativeSpeed;
}

} // namespace GOOM::FILTER_FX

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#ifdef __clang__
#pragma clang diagnostic pop
#endif
