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

// Return a random integer in the range 0 <= n < n1.
[[nodiscard]] auto GetNRand(uint32_t n1) noexcept -> uint32_t;

// Return a random integer in the range 0 <= n <= GOOM_RAND_MAX.
[[nodiscard]] auto GetRand() noexcept -> uint32_t;


// Return a random integer number in the range [n0, n0 + nRangePlus1).
[[nodiscard]] auto GetRandInRange(uint32_t n0, uint32_t nRangePlus1) noexcept -> uint32_t;
[[nodiscard]] auto GetRandInRange(int32_t n0, int32_t nRangePlus1) noexcept -> int32_t;

// Return a random real number in the range [x0, x0 + xRange].
[[nodiscard]] auto GetRandInRange(float x0, float xRange) noexcept -> float;
[[nodiscard]] auto GetRandInRange(double x0, double xRange) noexcept -> double;

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

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
inline auto GetRandInRange(const uint32_t n0, const uint32_t nRangePlus1) noexcept -> uint32_t
{
  return n0 + GetNRand(nRangePlus1);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
inline auto GetRandInRange(const int32_t n0, const int32_t nRangePlus1) noexcept -> int32_t
{
  Expects(nRangePlus1 > 0);
  return n0 + static_cast<int32_t>(GetNRand(static_cast<uint32_t>(nRangePlus1)));
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
inline auto GetRandInRange(const float x0, const float xRange) noexcept -> float
{
  static constexpr auto MAX_RAND_FLT = static_cast<float>(GOOM_RAND_MAX);
  const auto t = static_cast<float>(GetNRand(GOOM_RAND_MAX + 1)) / MAX_RAND_FLT;
  return x0 + (t * xRange);
  //  thread_local std::uniform_real_distribution<> dis(0, 1);
  //  return std::lerp(x0, x1, static_cast<float>(dis(eng)));
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
inline auto GetRandInRange(const double x0, const double xRange) noexcept -> double
{
  static constexpr auto MAX_RAND_DBL = static_cast<double>(GOOM_RAND_MAX);
  const auto t = static_cast<double>(GetNRand(GOOM_RAND_MAX + 1)) / MAX_RAND_DBL;
  return x0 + (t * xRange);
  //  thread_local std::uniform_real_distribution<> dis(0, 1);
  //  return std::lerp(x0, x1, static_cast<float>(dis(eng)));
}

} // namespace GOOM::UTILS::MATH::RAND

module :private;

namespace GOOM::UTILS::MATH::RAND
{

// IMPORTANT: DO NOT MAKE ANY OF thESE INLINE - SCREWS UP thread_local.
// See TEST_CASE("repeatable random sequence")

auto GetNRand(const uint32_t n) noexcept -> uint32_t
{
  return GEN::Generate(n);
}

auto GetRand() noexcept -> uint32_t
{
  return GEN::Generate(GOOM_RAND_MAX + 1);
}

} // namespace GOOM::UTILS::MATH::RAND
