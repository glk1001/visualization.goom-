// NOLINTBEGIN(cert-err58-cpp): Catch2 3.6.0 issue

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <cstdint>
#include <format>
#include <limits>

import Goom.Tests.Utils.Math.RandHelper;
import Goom.Utils.Math.Rand.RandGen;

namespace GOOM::UNIT_TESTS
{

using UTILS::MATH::RAND::GEN::GEN_RAND_MAX;
using UTILS::MATH::RAND::GEN::Generate;
using UTILS::MATH::RAND::GEN::GetRandSeed;
using UTILS::MATH::RAND::GEN::SetRandSeed;

// NOLINTBEGIN(bugprone-chained-comparison): Catch2 needs to fix this.
// NOLINTBEGIN(readability-function-cognitive-complexity)

static const auto RAND_GENERATOR = []([[maybe_unused]] const auto min, const auto max)
{ return Generate(max); };

TEST_CASE("Lemire")
{
  // Check cast truncation
  static constexpr auto M_UINT64 = 0xAAAAFFFFFFEUL;
  static_assert(M_UINT64 > std::numeric_limits<uint32_t>::max());

  static constexpr auto M_UINT64_TRUNC32 = static_cast<uint32_t>(M_UINT64);
  static constexpr auto M_UINT64_MOD_MAX = M_UINT64 % (static_cast<uint64_t>(GEN_RAND_MAX) + 1U);
  STATIC_REQUIRE(M_UINT64_TRUNC32 == M_UINT64_MOD_MAX);

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4146) // Minus unsigned works fine for clang.
#endif
  // Check -s % s
  static constexpr auto M_UINT32 = 0xAAAFFEU;
  static_assert(M_UINT32 < std::numeric_limits<uint32_t>::max());

  static constexpr auto M_UINT32_COMPLEMENT   = -M_UINT32;
  static constexpr auto GOOM_RAND_MAX_MINUS_M = GEN_RAND_MAX - M_UINT32;
  static_assert(M_UINT32_COMPLEMENT == (GOOM_RAND_MAX_MINUS_M + 1));

  static constexpr auto M_UINT32_MOD        = -M_UINT32 % M_UINT32;
  static constexpr auto GOOM_RAND_MAX_MOD_M = GEN_RAND_MAX % M_UINT32;
  STATIC_REQUIRE(M_UINT32_MOD == (GOOM_RAND_MAX_MOD_M + 1));
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
}

TEST_CASE("Rand Generate")
{
  // After a big enough loop, a good random distribution should have
  // covered the entire range: nMin <= n <= nMax
  static constexpr auto NUM_LOOPS                        = 1000'000U;
  static constexpr auto ACCEPTABLE_OUT_OF_UNIFORM_MARGIN = 550U;

  static constexpr auto SEED = 10UL;
  SetRandSeed(SEED);
  REQUIRE(SEED == GetRandSeed());

  static constexpr auto N_UPPER = 100U;
  const auto countsResults2     = GetCountResults(NUM_LOOPS, 0U, N_UPPER, RAND_GENERATOR);
  REQUIRE(countsResults2.min == 0U);
  REQUIRE(countsResults2.max == N_UPPER - 1);
  REQUIRE(countsResults2.numCounts == N_UPPER);
  REQUIRE(countsResults2.sumOfAllCounts == NUM_LOOPS);
  UNSCOPED_INFO(std::format("minCountAt = {}", countsResults2.minCountAt));
  UNSCOPED_INFO(std::format("maxCountAt = {}", countsResults2.maxCountAt));
  REQUIRE(countsResults2.maxCount - countsResults2.minCount < ACCEPTABLE_OUT_OF_UNIFORM_MARGIN);


  static constexpr auto BENCHMARK_NUM_LOOPS = 10000U;
  BENCHMARK("uint32_t rand loop 2")
  {
    return GetCountResults(BENCHMARK_NUM_LOOPS, 0U, N_UPPER, RAND_GENERATOR);
  };
}

// NOLINTEND(readability-function-cognitive-complexity)
// NOLINTEND(bugprone-chained-comparison)

} // namespace GOOM::UNIT_TESTS

// NOLINTEND(cert-err58-cpp): Catch2 3.6.0 issue
