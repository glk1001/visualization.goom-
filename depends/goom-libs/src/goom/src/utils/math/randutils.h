#pragma once

#include <cstdint>
#include <iosfwd>
#include <type_traits>

namespace GOOM::UTILS::MATH::RAND
{

[[nodiscard]] auto GetRandSeed() noexcept -> uint64_t;
void SetRandSeed(uint64_t seed) noexcept;

extern const uint32_t G_RAND_MAX;

void SaveRandState(std::ostream& file);
void RestoreRandState(std::istream& file);

// Return random positive integer in the range n0 <= n < n1.
[[nodiscard]] auto GetRandInRange(uint32_t n0, uint32_t n1) noexcept -> uint32_t;

// Return random integer in the range 0 <= n < n1.
[[nodiscard]] auto GetNRand(uint32_t n1) noexcept -> uint32_t;

// Return random integer in the range 0 <= n < randMax.
[[nodiscard]] auto GetRand() noexcept -> uint32_t;

// Return random integer in the range n0 <= n < n1.
[[nodiscard]] auto GetRandInRange(int32_t n0, int32_t n1) noexcept -> int32_t;

// Return random float in the range x0 <= n <= x1.
[[nodiscard]] auto GetRandInRange(float x0, float x1) noexcept -> float;
[[nodiscard]] auto GetRandInRange(double x0, double x1) noexcept -> double;

template<typename T>
struct NumberRange
{
  T min;
  T max;
};
template<typename T>
[[nodiscard]] auto GetRandInRange(const NumberRange<T>& numberRange) noexcept -> T;

// Return prob(m/n)
[[nodiscard]] inline auto ProbabilityOfMInN(uint32_t m, uint32_t n) noexcept -> bool;
[[nodiscard]] inline auto ProbabilityOf(float prob) noexcept -> bool;

inline auto GetNRand(const uint32_t n1) noexcept -> uint32_t
{
  return GetRandInRange(0U, n1);
}

inline auto GetRand() noexcept -> uint32_t
{
  return GetRandInRange(0U, G_RAND_MAX);
}

template<typename T>
inline auto GetRandInRange(const NumberRange<T>& numberRange) noexcept -> T
{
  if (std::is_integral<T>())
  {
    return GetRandInRange(numberRange.min, numberRange.max + 1);
  }
  return GetRandInRange(numberRange.min, numberRange.max);
}

inline auto ProbabilityOfMInN(const uint32_t m, const uint32_t n) noexcept -> bool
{
  if (1 == m)
  {
    return 0 == GetNRand(n);
  }
  if (m == (n - 1))
  {
    return GetNRand(n) > 0;
  }
  return GetRandInRange(0.0F, 1.0F) <= (static_cast<float>(m) / static_cast<float>(n));
}

inline auto ProbabilityOf(const float prob) noexcept -> bool
{
  return GetRandInRange(0.0F, 1.0F) <= prob;
}

} // namespace GOOM::UTILS::MATH::RAND
