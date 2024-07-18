// NOLINTBEGIN(cert-err58-cpp): Catch2 3.6.0 issue

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <format>
#include <vector>

import Goom.Tests.Utils.Math.RandHelper;
import Goom.Utils.Math.Rand.RandUtils;

namespace GOOM::UNIT_TESTS
{

using UTILS::MATH::RAND::GetRand;
using UTILS::MATH::RAND::GetRandInRange;
using UTILS::MATH::RAND::GetRandSeed;
using UTILS::MATH::RAND::SetRandSeed;

// NOLINTBEGIN(bugprone-chained-comparison): Catch2 needs to fix this.
// NOLINTBEGIN(readability-function-cognitive-complexity)

TEST_CASE("repeatable random sequence")
{
  static constexpr auto NUM_LOOPS = 1000U;
  static constexpr auto SEED      = 1000UL;

  SetRandSeed(SEED);
  REQUIRE(SEED == GetRandSeed());
  auto seq1    = std::vector<uint32_t>(NUM_LOOPS);
  auto fltSeq1 = std::vector<float>(NUM_LOOPS);
  for (auto i = 0U; i < NUM_LOOPS; ++i)
  {
    seq1[i]    = GetRand();
    fltSeq1[i] = GetRandInRange(0.0F, 1.0F);
  }

  SetRandSeed(SEED);
  REQUIRE(SEED == GetRandSeed());
  auto seq2    = std::vector<uint32_t>(NUM_LOOPS);
  auto fltSeq2 = std::vector<float>(NUM_LOOPS);
  for (auto i = 0U; i < NUM_LOOPS; ++i)
  {
    seq2[i]    = GetRand();
    fltSeq2[i] = GetRandInRange(0.0F, 1.0F);
  }

  UNSCOPED_INFO(std::format("GetRandSeed() = {}", GetRandSeed()));
  REQUIRE(seq1 == seq2);

  SetRandSeed(SEED + 1);
  REQUIRE((SEED + 1) == GetRandSeed());
  auto seq3    = std::vector<uint32_t>(NUM_LOOPS);
  auto fltSeq3 = std::vector<float>(NUM_LOOPS);
  for (auto i = 0U; i < NUM_LOOPS; ++i)
  {
    seq3[i]    = GetRand();
    fltSeq3[i] = GetRandInRange(0.0F, 1.0F);
  }

  REQUIRE(seq1 != seq3);

  REQUIRE(fltSeq1 == fltSeq2);
  REQUIRE(fltSeq1 != fltSeq3);

  SetRandSeed(SEED);
  REQUIRE(SEED == GetRandSeed());
}

static const auto INT_RAND_GENERATOR = [](const auto min, const auto range)
{ return GetRandInRange(min, range + 1); };
static const auto FLT_RAND_GENERATOR = [](const auto min, const auto range)
{ return GetRandInRange(min, range); };

TEST_CASE("uint32_t min max get random")
{
  // After a big enough loop, a good random distribution should have
  // covered the entire range: nMin <= n < nMax
  static constexpr auto NUM_LOOPS                        = 1000'000U;
  static constexpr auto ACCEPTABLE_OUT_OF_UNIFORM_MARGIN = 600U;

  static constexpr auto SEED = 1000UL;
  SetRandSeed(SEED);

  static constexpr auto N_MIN1   = 999U;
  static constexpr auto N_MAX1   = 10001U;
  static constexpr auto N_RANGE1 = N_MAX1 - N_MIN1;
  const auto countsResults1      = GetCountResults(NUM_LOOPS, N_MIN1, N_RANGE1, INT_RAND_GENERATOR);
  REQUIRE(countsResults1.min == N_MIN1);
  REQUIRE(countsResults1.max == N_MAX1);
  REQUIRE(countsResults1.numCounts == (N_RANGE1 + 1));
  REQUIRE(countsResults1.sumOfAllCounts == NUM_LOOPS);
  UNSCOPED_INFO(std::format(
      "minCount = {}, minCountAt = {}", countsResults1.minCount, countsResults1.minCountAt));
  UNSCOPED_INFO(std::format(
      "maxCount = {}, maxCountAt = {}", countsResults1.maxCount, countsResults1.maxCountAt));
  REQUIRE(countsResults1.maxCount - countsResults1.minCount < ACCEPTABLE_OUT_OF_UNIFORM_MARGIN);

  static constexpr auto N_MIN2   = 0U;
  static constexpr auto N_MAX2   = 100U;
  static constexpr auto N_RANGE2 = N_MAX2 - N_MIN2;
  const auto countsResults2      = GetCountResults(NUM_LOOPS, N_MIN2, N_RANGE2, INT_RAND_GENERATOR);
  REQUIRE(countsResults2.min == N_MIN2);
  REQUIRE(countsResults2.max == N_MAX2);
  REQUIRE(countsResults2.numCounts == (N_RANGE2 + 1));
  REQUIRE(countsResults2.sumOfAllCounts == NUM_LOOPS);
  UNSCOPED_INFO(std::format(
      "minCount = {}, minCountAt = {}", countsResults2.minCount, countsResults2.minCountAt));
  UNSCOPED_INFO(std::format(
      "maxCount = {}, maxCountAt = {}", countsResults2.maxCount, countsResults2.maxCountAt));
  REQUIRE(countsResults2.maxCount - countsResults2.minCount < ACCEPTABLE_OUT_OF_UNIFORM_MARGIN);

  REQUIRE(5U == GetRandInRange(5U, 1U));


  static constexpr auto BENCHMARK_NUM_LOOPS = 10000U;
  BENCHMARK("uint32_t rand loop 1")
  {
    return GetCountResults(BENCHMARK_NUM_LOOPS, N_MIN1, N_RANGE1, INT_RAND_GENERATOR);
  };
  BENCHMARK("uint32_t rand loop 2")
  {
    return GetCountResults(BENCHMARK_NUM_LOOPS, N_MIN2, N_RANGE2, INT_RAND_GENERATOR);
  };
}

TEST_CASE("int32_t min max get random")
{
  // After a big enough loop, a good random distribution should have
  // covered the entire range: nMin <= n < nMax
  static constexpr auto NUM_LOOPS                        = 500'000U;
  static constexpr auto ACCEPTABLE_OUT_OF_UNIFORM_MARGIN = 300U;

  static constexpr auto SEED = 1000UL;
  SetRandSeed(SEED);

  static constexpr auto N_MIN1   = -999;
  static constexpr auto N_MAX1   = 10001;
  static constexpr auto N_RANGE1 = N_MAX1 - N_MIN1;
  const auto countsResults1      = GetCountResults(NUM_LOOPS, N_MIN1, N_RANGE1, INT_RAND_GENERATOR);
  REQUIRE(countsResults1.min == N_MIN1);
  REQUIRE(countsResults1.max == N_MAX1);
  REQUIRE(countsResults1.numCounts == (N_RANGE1 + 1));
  UNSCOPED_INFO(std::format(
      "minCount = {}, minCountAt = {}", countsResults1.minCount, countsResults1.minCountAt));
  UNSCOPED_INFO(std::format(
      "maxCount = {}, maxCountAt = {}", countsResults1.maxCount, countsResults1.maxCountAt));
  REQUIRE(countsResults1.maxCount - countsResults1.minCount < ACCEPTABLE_OUT_OF_UNIFORM_MARGIN);

  static constexpr auto N_MIN2   = -999;
  static constexpr auto N_MAX2   = -50;
  static constexpr auto N_RANGE2 = N_MAX2 - N_MIN2;
  const auto countsResults2      = GetCountResults(NUM_LOOPS, N_MIN2, N_RANGE2, INT_RAND_GENERATOR);
  REQUIRE(countsResults2.min == N_MIN2);
  REQUIRE(countsResults2.max == N_MAX2);
  REQUIRE(countsResults2.numCounts == (N_RANGE2 + 1));
  UNSCOPED_INFO(std::format(
      "minCount = {}, minCountAt = {}", countsResults2.minCount, countsResults2.minCountAt));
  UNSCOPED_INFO(std::format(
      "maxCount = {}, maxCountAt = {}", countsResults2.maxCount, countsResults2.maxCountAt));
  REQUIRE(countsResults2.maxCount - countsResults2.minCount < ACCEPTABLE_OUT_OF_UNIFORM_MARGIN);

  static constexpr auto N_MIN3   = 1;
  static constexpr auto N_MAX3   = 999;
  static constexpr auto N_RANGE3 = N_MAX3 - N_MIN3;
  const auto countsResults3      = GetCountResults(NUM_LOOPS, N_MIN3, N_RANGE3, INT_RAND_GENERATOR);
  REQUIRE(countsResults3.min == N_MIN3);
  REQUIRE(countsResults3.max == N_MAX3);
  REQUIRE(countsResults3.numCounts == (N_RANGE3 + 1));
  UNSCOPED_INFO(std::format(
      "minCount = {}, minCountAt = {}", countsResults3.minCount, countsResults3.minCountAt));
  UNSCOPED_INFO(std::format(
      "maxCount = {}, maxCountAt = {}", countsResults3.maxCount, countsResults3.maxCountAt));
  REQUIRE(countsResults3.maxCount - countsResults3.minCount < ACCEPTABLE_OUT_OF_UNIFORM_MARGIN);

  static constexpr auto N_MIN4   = 0;
  static constexpr auto N_MAX4   = 635;
  static constexpr auto N_RANGE4 = N_MAX4 - N_MIN4;
  const auto countsResults4      = GetCountResults(NUM_LOOPS, N_MIN4, N_RANGE4, INT_RAND_GENERATOR);
  REQUIRE(countsResults4.min == N_MIN4);
  REQUIRE(countsResults4.max == N_MAX4);
  REQUIRE(countsResults4.numCounts == (N_RANGE4 + 1));
  UNSCOPED_INFO(std::format(
      "minCount = {}, minCountAt = {}", countsResults4.minCount, countsResults4.minCountAt));
  UNSCOPED_INFO(std::format(
      "maxCount = {}, maxCountAt = {}", countsResults4.maxCount, countsResults4.maxCountAt));
  REQUIRE(countsResults4.maxCount - countsResults4.minCount < ACCEPTABLE_OUT_OF_UNIFORM_MARGIN);

  REQUIRE(5 == GetRandInRange(5, 1));
  REQUIRE(-6 == GetRandInRange(-6, 1));

  static constexpr auto BENCHMARK_NUM_LOOPS = 10000U;
  BENCHMARK("int32_t rand loop 1")
  {
    return GetCountResults(BENCHMARK_NUM_LOOPS, N_MIN1, N_RANGE1, INT_RAND_GENERATOR);
  };
  BENCHMARK("int32_t rand loop 2")
  {
    return GetCountResults(BENCHMARK_NUM_LOOPS, N_MIN2, N_RANGE2, INT_RAND_GENERATOR);
  };
  BENCHMARK("int32_t rand loop 3")
  {
    return GetCountResults(BENCHMARK_NUM_LOOPS, N_MIN3, N_RANGE3, INT_RAND_GENERATOR);
  };
  BENCHMARK("int32_t rand loop 4")
  {
    return GetCountResults(BENCHMARK_NUM_LOOPS, N_MIN4, N_RANGE4, INT_RAND_GENERATOR);
  };
}

TEST_CASE("float min max get random")
{
  // After a big enough loop, a good random distribution should have
  // covered the entire range: nMin <= n < nMax
  static constexpr auto NUM_LOOPS                        = 1000'000U;
  static constexpr auto REASONABLE_FLOAT_COUNT           = 950000U;
  static constexpr auto ACCEPTABLE_OUT_OF_UNIFORM_MARGIN = 100U;

  static constexpr auto SMALL_VAL = 0.0002F;
  using Catch::Approx;

  static constexpr auto SEED = 1000UL;
  SetRandSeed(SEED);

  static constexpr auto N_MIN1   = 0.0F;
  static constexpr auto N_MAX1   = 1.0F;
  static constexpr auto N_RANGE1 = N_MAX1 - N_MIN1;
  const auto countsResults1      = GetCountResults(NUM_LOOPS, N_MIN1, N_RANGE1, FLT_RAND_GENERATOR);
  REQUIRE(countsResults1.min == Approx(N_MIN1).margin(SMALL_VAL));
  REQUIRE(countsResults1.max == Approx(N_MAX1).margin(SMALL_VAL));
  REQUIRE(countsResults1.numCounts > REASONABLE_FLOAT_COUNT);
  UNSCOPED_INFO(std::format(
      "minCount = {}, minCountAt = {}", countsResults1.minCount, countsResults1.minCountAt));
  UNSCOPED_INFO(std::format(
      "maxCount = {}, maxCountAt = {}", countsResults1.maxCount, countsResults1.maxCountAt));
  REQUIRE(countsResults1.maxCount - countsResults1.minCount < ACCEPTABLE_OUT_OF_UNIFORM_MARGIN);

  static constexpr auto N_MIN2   = -1.0F;
  static constexpr auto N_MAX2   = 0.0F;
  static constexpr auto N_RANGE2 = N_MAX2 - N_MIN2;
  const auto countsResults2      = GetCountResults(NUM_LOOPS, N_MIN2, N_RANGE2, FLT_RAND_GENERATOR);
  REQUIRE(countsResults2.min == Approx(N_MIN2).margin(SMALL_VAL));
  REQUIRE(countsResults2.max == Approx(N_MAX2).margin(SMALL_VAL));
  REQUIRE(countsResults2.numCounts > REASONABLE_FLOAT_COUNT);
  UNSCOPED_INFO(std::format(
      "minCount = {}, minCountAt = {}", countsResults2.minCount, countsResults2.minCountAt));
  UNSCOPED_INFO(std::format(
      "maxCount = {}, maxCountAt = {}", countsResults2.maxCount, countsResults2.maxCountAt));
  REQUIRE(countsResults2.maxCount - countsResults2.minCount < ACCEPTABLE_OUT_OF_UNIFORM_MARGIN);

  static constexpr auto N_MIN3   = -10.0F;
  static constexpr auto N_MAX3   = +10.0F;
  static constexpr auto N_RANGE3 = N_MAX3 - N_MIN3;
  const auto countsResults3      = GetCountResults(NUM_LOOPS, N_MIN3, N_RANGE3, FLT_RAND_GENERATOR);
  REQUIRE(countsResults3.min == Approx(N_MIN3).margin(SMALL_VAL));
  REQUIRE(countsResults3.max == Approx(N_MAX3).margin(SMALL_VAL));
  REQUIRE(countsResults3.numCounts > REASONABLE_FLOAT_COUNT);
  UNSCOPED_INFO(std::format(
      "minCount = {}, minCountAt = {}", countsResults3.minCount, countsResults3.minCountAt));
  UNSCOPED_INFO(std::format(
      "maxCount = {}, maxCountAt = {}", countsResults3.maxCount, countsResults3.maxCountAt));
  REQUIRE(countsResults3.maxCount - countsResults3.minCount < ACCEPTABLE_OUT_OF_UNIFORM_MARGIN);

  static constexpr auto POS = 5.0F;
  static constexpr auto NEG = -6.0F;
  REQUIRE(Approx(POS) == GetRandInRange(POS, 0.0F));
  REQUIRE(Approx(NEG) == GetRandInRange(NEG, 0.0F));

  static constexpr auto BENCHMARK_NUM_LOOPS = 10000U;
  BENCHMARK("float rand loop 1")
  {
    return GetCountResults(BENCHMARK_NUM_LOOPS, N_MIN1, N_RANGE1, FLT_RAND_GENERATOR);
  };
  BENCHMARK("float rand loop 2")
  {
    return GetCountResults(BENCHMARK_NUM_LOOPS, N_MIN2, N_RANGE2, FLT_RAND_GENERATOR);
  };
  BENCHMARK("float rand loop 3")
  {
    return GetCountResults(BENCHMARK_NUM_LOOPS, N_MIN3, N_RANGE3, FLT_RAND_GENERATOR);
  };
}

// NOLINTEND(readability-function-cognitive-complexity)
// NOLINTEND(bugprone-chained-comparison)

} // namespace GOOM::UNIT_TESTS

// NOLINTEND(cert-err58-cpp): Catch2 3.6.0 issue
