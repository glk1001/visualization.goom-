module;

#include <cmath>
#include <cstdint>

export module Goom.Utils.Math.Rand.RandUtils;

import Goom.Utils.Math.Rand.RandGen;
import Goom.Lib.AssertUtils;

export namespace GOOM::UTILS::MATH::RAND
{

inline constexpr auto GOOM_RAND_MAX = GEN::GOOM_RAND_MAX;

[[nodiscard]] auto GetRandSeed() noexcept -> uint64_t;
auto SetRandSeed(uint64_t seed) noexcept -> void;

// Return random positive integer in the range n0 <= n < n1.
[[nodiscard]] auto GetRandInRange(uint32_t n0, uint32_t n1) noexcept -> uint32_t;

// Return random integer in the range n0 <= n < n1.
[[nodiscard]] auto GetRandInRange(int32_t n0, int32_t n1) noexcept -> int32_t;

// Return random float in the range x0 <= n <= x1.
[[nodiscard]] auto GetRandInRange(float x0, float x1) noexcept -> float;
[[nodiscard]] auto GetRandInRange(double x0, double x1) noexcept -> double;

[[nodiscard]] auto ProbabilityOf(float prob) noexcept -> bool;

// Return random integer in the range 0 <= n < n1.
[[nodiscard]] auto GetNRand(uint32_t n1) noexcept -> uint32_t;

// Return random integer in the range 0 <= n < GOOM_RAND_MAX.
[[nodiscard]] auto GetRand() noexcept -> uint32_t;

} // namespace GOOM::UTILS::MATH::RAND

namespace GOOM::UTILS::MATH::RAND
{

inline auto GetRandSeed() noexcept -> uint64_t
{
  return GEN::GetRandSeed();
}

inline auto SetRandSeed(uint64_t seed) noexcept -> void
{
  GEN::SetRandSeed(seed);
}

inline auto ProbabilityOf(const float prob) noexcept -> bool
{
  return GetRandInRange(0.0F, 1.0F) <= prob;
}

inline auto GetNRand(const uint32_t n) noexcept -> uint32_t
{
  return GetRandInRange(0U, n);
}

inline auto GetRand() noexcept -> uint32_t
{
  return GetRandInRange(0U, GOOM_RAND_MAX);
}

} // namespace GOOM::UTILS::MATH::RAND

module :private;

namespace GOOM::UTILS::MATH::RAND
{

// IMPORTANT: DO NOT MAKE ANY OF thESE INLINE - SCREWS UP thread_local.
// See TEST_CASE("repeatable random sequence")

auto GetRandInRange(const uint32_t n0, const uint32_t n1) noexcept -> uint32_t
{
  Expects(n0 < n1);

  return n0 + GEN::Generate(n1 - n0);
}

auto GetRandInRange(const int32_t n0, const int32_t n1) noexcept -> int32_t
{
  Expects(n0 < n1);

  return n0 + static_cast<int32_t>(GEN::Generate(static_cast<uint32_t>(n1 - n0)));
}

auto GetRandInRange(const float x0, const float x1) noexcept -> float
{
  Expects(x0 < x1);

  static constexpr auto ENG_MAX = static_cast<float>(GOOM_RAND_MAX - 1);
  const auto t                  = static_cast<float>(GEN::Generate(GOOM_RAND_MAX)) / ENG_MAX;
  return std::lerp(x0, x1, t);
  //  thread_local std::uniform_real_distribution<> dis(0, 1);
  //  return std::lerp(x0, x1, static_cast<float>(dis(eng)));
}

auto GetRandInRange(const double x0, const double x1) noexcept -> double
{
  Expects(x0 < x1);

  static constexpr auto ENG_MAX = static_cast<double>(GOOM_RAND_MAX - 1);
  const auto t                  = static_cast<double>(GEN::Generate(GOOM_RAND_MAX)) / ENG_MAX;
  return std::lerp(x0, x1, t);
  //  thread_local std::uniform_real_distribution<> dis(0, 1);
  //  return std::lerp(x0, x1, static_cast<float>(dis(eng)));
}

} // namespace GOOM::UTILS::MATH::RAND
