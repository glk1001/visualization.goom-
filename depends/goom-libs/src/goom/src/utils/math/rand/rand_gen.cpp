module;

#include "xoshiro.hpp"

#include <cstdint>

module Goom.Utils.Math.Rand.RandGen;

namespace GOOM::UTILS::MATH::RAND::GEN
{

// NOTE: C++ std::uniform_int_distribution is too expensive (about double time) so we use
// Xoshiro and Lemire multiplication/shift technique. For timings, see tests/test_goomrand.cpp.

using RandType = XoshiroCpp::Xoshiro128Plus;
//using RandType = std::mt19937;

static constexpr auto RAND_BITS = 32U;

static_assert(GOOM_RAND_MAX == RandType::max());
static_assert(GOOM_RAND_MAX == (1UL << RAND_BITS) - 1);
static_assert(((static_cast<uint64_t>(GOOM_RAND_MAX) + 1) >> RAND_BITS) == 1);

namespace
{

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables): Hard to get around!
uint64_t sRandSeed = 1UL;
thread_local RandType sXoshiroEng{GetRandSeed()}; // NOLINT(cert-err58-cpp)
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables): Hard to get around!

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

// NOLINTBEGIN(readability-identifier-length): Stick to Lemire's naming.
auto Generate(const uint32_t n) noexcept -> uint32_t
{
  // const auto x = static_cast<uint32_t>(sXoshiroEng());
  // const auto m = (static_cast<uint64_t>(x) * static_cast<uint64_t>(n)) >> RAND_BITS;
  // return static_cast<uint32_t>(m);

  const auto s = n;

  auto x = sXoshiroEng();

  auto m = static_cast<uint64_t>(x) * static_cast<uint64_t>(s);
  auto l = static_cast<uint32_t>(m); // == m % (GOOM_RAND_MAX + 1)

  if (l < s)
  {
    const uint32_t t = -s % s; // (GOOM_RAND_MAX % s) + 1
    while (l < t)
    {
      x = sXoshiroEng();
      m = static_cast<uint64_t>(x) * static_cast<uint64_t>(s);
      l = static_cast<uint32_t>(m);
    }
  }

  return m >> RAND_BITS;
}
// NOLINTEND(readability-identifier-length)

} // namespace GOOM::UTILS::MATH::RAND::GEN
