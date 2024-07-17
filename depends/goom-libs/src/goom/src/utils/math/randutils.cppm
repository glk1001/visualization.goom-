module;

#include "xoshiro.hpp"

#include <cmath>
#include <cstdint>
#include <limits>

export module Goom.Utils.Math.RandUtils;

import Goom.Lib.AssertUtils;

export namespace GOOM::UTILS::MATH::RAND
{

inline constexpr auto GOOM_RAND_MAX = std::numeric_limits<uint32_t>::max();

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

// NOTE: C++ std::uniform_int_distribution is too expensive (about double time) so we use
// Xoshiro and Lemire multiplication/shift technique. For timings, see tests/test_goomrand.cpp.

namespace
{

using RandType = XoshiroCpp::Xoshiro128Plus;
//using RandType = std::mt19937;

constexpr auto RAND_BITS = 32U;

static_assert(GOOM_RAND_MAX == RandType::max());
static_assert(GOOM_RAND_MAX == (static_cast<uint64_t>(1UL << RAND_BITS) - 1));
static_assert(((static_cast<uint64_t>(GOOM_RAND_MAX) + 1) >> RAND_BITS) == 1);

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables): Hard to get around!
uint64_t sRandSeed = 1UL;
thread_local RandType sXoshiroEng{GetRandSeed()}; // NOLINT(cert-err58-cpp)
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables): Hard to get around!

auto RandXoshiroFunc(const uint32_t n) noexcept -> uint32_t
{
  auto x       = sXoshiroEng();
  auto m       = static_cast<uint64_t>(x) * static_cast<uint64_t>(n);
  auto l       = static_cast<uint32_t>(m);
  const auto s = static_cast<uint32_t>(n);

  if (l < s)
  {
    /* cppcheck-suppress oppositeExpression */
    const uint32_t t = -s % s;
    while (l < t)
    {
      x = sXoshiroEng();
      m = static_cast<uint64_t>(x) * static_cast<uint64_t>(n);
      l = static_cast<uint32_t>(m);
    }
  }

  return m >> RAND_BITS;
}

} // namespace

auto GetRandSeed() noexcept -> uint64_t
{
  return sRandSeed;
}

auto SetRandSeed(const uint64_t seed) noexcept -> void
{
  sRandSeed = seed;
  //  sXoshiroEng.seed(static_cast<uint_fast32_t>(sRandSeed));
  sXoshiroEng = RandType{sRandSeed};
}

// IMPORTANT: DO NOT MAKE INLINE - SCREWS UP thread_local.
// See TEST_CASE("repeatable random sequence")
auto GetRandInRange(const uint32_t n0, const uint32_t n1) noexcept -> uint32_t
{
  Expects(n0 < n1);

  return n0 + RandXoshiroFunc(n1 - n0);
}

// IMPORTANT: DO NOT MAKE INLINE - SCREWS UP thread_local.
// See TEST_CASE("repeatable random sequence")
auto GetRandInRange(const int32_t n0, const int32_t n1) noexcept -> int32_t
{
  Expects(n0 < n1);

  return n0 + static_cast<int32_t>(RandXoshiroFunc(static_cast<uint32_t>(n1 - n0)));
}

// IMPORTANT: DO NOT MAKE INLINE - SCREWS UP thread_local.
// See TEST_CASE("repeatable random sequence")
auto GetRandInRange(const float x0, const float x1) noexcept -> float
{
  Expects(x0 < x1);

  static constexpr auto ENG_MAX = static_cast<float>(GOOM_RAND_MAX - 1);
  const auto t                  = static_cast<float>(RandXoshiroFunc(GOOM_RAND_MAX)) / ENG_MAX;
  return std::lerp(x0, x1, t);
  //  thread_local std::uniform_real_distribution<> dis(0, 1);
  //  return std::lerp(x0, x1, static_cast<float>(dis(eng)));
}

// IMPORTANT: DO NOT MAKE INLINE - SCREWS UP thread_local.
// See TEST_CASE("repeatable random sequence")
auto GetRandInRange(const double x0, const double x1) noexcept -> double
{
  Expects(x0 < x1);

  static constexpr auto ENG_MAX = static_cast<double>(GOOM_RAND_MAX - 1);
  const auto t                  = static_cast<double>(RandXoshiroFunc(GOOM_RAND_MAX)) / ENG_MAX;
  return std::lerp(x0, x1, t);
  //  thread_local std::uniform_real_distribution<> dis(0, 1);
  //  return std::lerp(x0, x1, static_cast<float>(dis(eng)));
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
