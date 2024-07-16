module;

#include "goom/goom_logger.h"
#include "xoshiro.hpp"

#include <cmath>
#include <cstdint>
#include <limits>
#include <random>

export module Goom.Utils.Math.RandUtils;

import Goom.Lib.AssertUtils;

export namespace GOOM::UTILS::MATH::RAND
{

[[nodiscard]] auto GetRandSeed() noexcept -> uint64_t;
auto SetRandSeed(uint64_t seed) noexcept -> void;

extern const uint32_t G_RAND_MAX;

// Return random positive integer in the range n0 <= n < n1.
[[nodiscard]] auto GetRandInRange(uint32_t n0, uint32_t n1) noexcept -> uint32_t;

// Return random integer in the range 0 <= n < n1.
[[nodiscard]] auto GetNRand(uint32_t n) noexcept -> uint32_t;

// Return random integer in the range 0 <= n < randMax.
[[nodiscard]] auto GetRand() noexcept -> uint32_t;

// Return random integer in the range n0 <= n < n1.
[[nodiscard]] auto GetRandInRange(int32_t n0, int32_t n1) noexcept -> int32_t;

// Return random float in the range x0 <= n <= x1.
[[nodiscard]] auto GetRandInRange(float x0, float x1) noexcept -> float;
[[nodiscard]] auto GetRandInRange(double x0, double x1) noexcept -> double;

[[nodiscard]] inline auto ProbabilityOf(float prob) noexcept -> bool;

} // namespace GOOM::UTILS::MATH::RAND

namespace GOOM::UTILS::MATH::RAND
{

inline auto GetNRand(const uint32_t n) noexcept -> uint32_t
{
  return GetRandInRange(0U, n);
}

inline auto GetRand() noexcept -> uint32_t
{
  return GetRandInRange(0U, G_RAND_MAX);
}

inline auto ProbabilityOf(const float prob) noexcept -> bool
{
  return GetRandInRange(0.0F, 1.0F) <= prob;
}

} // namespace GOOM::UTILS::MATH::RAND

namespace GOOM::UTILS::MATH::RAND
{

// NOTE: C++ std::uniform_int_distribution is too expensive (about double time) so we use
// Xoshiro and multiplication/shift technique. For timings, see tests/test_goomrand.cpp.

namespace
{

using RandType = xoshiro256plus64;
//using RandType = std::mt19937;

constexpr uint32_t G_RAND_MAX_CONST = (RandType::max() > std::numeric_limits<uint32_t>::max())
                                          ? std::numeric_limits<uint32_t>::max()
                                          : static_cast<uint32_t>(RandType::max());
//static_assert(RandType::max() == std::numeric_limits<uint32_t>::max());

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables): Hard to get around!
uint64_t sRandSeed = 1UL;
thread_local RandType sXoshiroEng{static_cast<uint_fast32_t>(GetRandSeed())}; // NOLINT(cert-err58-cpp)
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables): Hard to get around!

inline auto RandXoshiroFunc(const uint32_t n) noexcept -> uint32_t
{
  const auto x = static_cast<uint32_t>(sXoshiroEng());
  const auto m = (static_cast<uint64_t>(x) * static_cast<uint64_t>(n)) >> 32U;
  return static_cast<uint32_t>(m);

  // auto x = sXoshiroEng();
  // auto m       = static_cast<uint64_t>(x) * static_cast<uint64_t>(n);
  // auto l       = static_cast<uint32_t>(m);
  // const auto s = static_cast<uint32_t>(n);
  //
  // if (l < s)
  // {
  //   /* cppcheck-suppress oppositeExpression */
  //   const uint32_t t = -s % s;
  //   while (l < t)
  //   {
  //     x = sXoshiroEng();
  //     m = static_cast<uint64_t>(x) * static_cast<uint64_t>(n);
  //     l = static_cast<uint32_t>(m);
  //   }
  // }
  //
  // return m >> 32U;
}

} // namespace

const uint32_t G_RAND_MAX = G_RAND_MAX_CONST;

auto GetRandSeed() noexcept -> uint64_t
{
  return sRandSeed;
}

auto SetRandSeed(const uint64_t seed) noexcept -> void
{
  sRandSeed = seed;
//  sXoshiroEng.seed(static_cast<uint_fast32_t>(sRandSeed));
  sXoshiroEng = sRandSeed;
}

auto GetRandInRange(const uint32_t n0, const uint32_t n1) noexcept -> uint32_t
{
  Expects(n0 < n1);

  return n0 + RandXoshiroFunc(n1 - n0);
}

auto GetRandInRange(const int32_t n0, const int32_t n1) noexcept -> int32_t
{
  Expects(n0 < n1);

  return n0 + static_cast<int32_t>(RandXoshiroFunc(static_cast<uint32_t>(n1 - n0)));
}

auto GetRandInRange(const float x0, const float x1) noexcept -> float
{
  Expects(x0 < x1);

  static constexpr auto ENG_MAX = static_cast<float>(G_RAND_MAX_CONST - 1);
  const auto t                  = static_cast<float>(RandXoshiroFunc(G_RAND_MAX_CONST)) / ENG_MAX;
  return std::lerp(x0, x1, t);
  //  thread_local std::uniform_real_distribution<> dis(0, 1);
  //  return std::lerp(x0, x1, static_cast<float>(dis(eng)));
}

auto GetRandInRange(const double x0, const double x1) noexcept -> double
{
  Expects(x0 < x1);

  static constexpr auto ENG_MAX = static_cast<double>(G_RAND_MAX_CONST - 1);
  const auto t                  = static_cast<double>(RandXoshiroFunc(G_RAND_MAX_CONST)) / ENG_MAX;
  return std::lerp(x0, x1, t);
  //  thread_local std::uniform_real_distribution<> dis(0, 1);
  //  return std::lerp(x0, x1, static_cast<float>(dis(eng)));
}

} // namespace GOOM::UTILS::MATH::RAND
