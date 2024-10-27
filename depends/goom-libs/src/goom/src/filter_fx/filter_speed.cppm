module;

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

import Goom.Lib.AssertUtils;

export namespace GOOM::FILTER_FX
{

class Speed
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

  [[nodiscard]] constexpr auto GetSpeed() const noexcept -> uint32_t;
  constexpr auto SetSpeed(uint32_t speed) noexcept -> void;
  constexpr auto SetDefaultSpeed() noexcept -> void;
  constexpr auto GoSlowerBy(uint32_t amount) noexcept -> void;
  [[nodiscard]] constexpr static auto GetFasterBy(uint32_t speed, uint32_t amount) noexcept
      -> uint32_t;
  [[nodiscard]] constexpr static auto GetSlowerBy(uint32_t speed, uint32_t amount) noexcept
      -> uint32_t;

  [[nodiscard]] constexpr auto IsSpeedReversed() const noexcept -> bool;
  constexpr auto SetSpeedReversed(bool flag) noexcept -> void;
  constexpr auto ToggleSpeedReversedFlag() noexcept -> void;

  [[nodiscard]] constexpr auto IsFasterThan(uint32_t otherSpeed) const noexcept -> bool;

  [[nodiscard]] constexpr auto GetRelativeSpeed() const noexcept -> float;

private:
  uint32_t m_speed     = DEFAULT_SPEED;
  bool m_speedReversed = true;
};

} // namespace GOOM::FILTER_FX

namespace GOOM::FILTER_FX
{

constexpr auto Speed::SetDefaultSpeed() noexcept -> void
{
  m_speed         = DEFAULT_SPEED;
  m_speedReversed = true;
}

constexpr auto Speed::IsSpeedReversed() const noexcept -> bool
{
  return m_speedReversed;
}

constexpr auto Speed::SetSpeedReversed(const bool flag) noexcept -> void
{
  m_speedReversed = flag;
}

constexpr auto Speed::ToggleSpeedReversedFlag() noexcept -> void
{
  m_speedReversed = not m_speedReversed;
}

constexpr auto Speed::GetSpeed() const noexcept -> uint32_t
{
  return m_speed;
}

constexpr auto Speed::SetSpeed(const uint32_t speed) noexcept -> void
{
  Expects(speed <= MAXIMUM_SPEED);
  Expects(speed >= STOP_SPEED);

  m_speed = speed;
}

constexpr auto Speed::GoSlowerBy(const uint32_t amount) noexcept -> void
{
  if (amount > m_speed)
  {
    m_speed = STOP_SPEED;
    return;
  }
  m_speed -= amount;

  Ensures(m_speed <= MAXIMUM_SPEED);
  Ensures(m_speed >= STOP_SPEED);
}

constexpr auto Speed::GetFasterBy(const uint32_t speed, const uint32_t amount) noexcept -> uint32_t
{
  return std::min(speed + amount, MAXIMUM_SPEED);
}

constexpr auto Speed::GetSlowerBy(const uint32_t speed, const uint32_t amount) noexcept -> uint32_t
{
  if (amount >= speed)
  {
    return STOP_SPEED;
  }
  return speed - amount;
}

constexpr auto Speed::IsFasterThan(const uint32_t otherSpeed) const noexcept -> bool
{
  return m_speed > otherSpeed;
}

constexpr auto Speed::GetRelativeSpeed() const noexcept -> float
{
  const auto relativeSpeed = static_cast<float>(m_speed) / static_cast<float>(MAXIMUM_SPEED);

  return m_speedReversed ? -relativeSpeed : +relativeSpeed;
}

} // namespace GOOM::FILTER_FX

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#ifdef __clang__
#pragma clang diagnostic pop
#endif
