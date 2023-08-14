#include "utils/math/randutils.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <tuple>
#include <vector>

#if __clang_major__ >= 16 // NOLINT: Can't include header for this.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wunsafe-buffer-usage"
#endif
#include <catch2/catch_test_macros.hpp>
#ifndef NDEBUG
#include <catch2/matchers/catch_matchers_all.hpp>
#endif
#if __clang_major__ >= 16 // NOLINT: Can't include header for this.
#pragma GCC diagnostic pop
#endif

namespace GOOM::UNIT_TESTS
{

#ifndef NDEBUG
using Catch::Matchers::StartsWith;
#endif
using UTILS::MATH::RAND::GetRand;
using UTILS::MATH::RAND::GetRandInRange;
using UTILS::MATH::RAND::GetRandSeed;
using UTILS::MATH::RAND::RestoreRandState;
using UTILS::MATH::RAND::SaveRandState;
using UTILS::MATH::RAND::SetRandSeed;

static constexpr auto SMALL_VAL = 0.0002F;

// NOLINTBEGIN(readability-function-cognitive-complexity)
TEST_CASE("save/restore random state")
{
  static constexpr auto NUM_LOOPS = 1000U;
  static constexpr auto SEED      = 1000UL;

  SetRandSeed(SEED);
  REQUIRE(SEED == GetRandSeed());

  const auto rand1 = GetRand();
  const auto rand2 = GetRand();
  REQUIRE(rand1 != rand2);

  namespace fs        = std::filesystem;
  const auto saveFile = fs::path{fs::temp_directory_path()} / "rand.out";
  std::ofstream fileOut{saveFile, std::ofstream::out};
  REQUIRE(not fileOut.fail());
  SaveRandState(fileOut);
  REQUIRE(not fileOut.fail());
  fileOut.close();
  REQUIRE(not fileOut.fail());
  const auto randJustAfterSave = GetRand();

  // Scramble things a bit
  static constexpr auto SEED_EXTRA = 10U;
  SetRandSeed(SEED + SEED_EXTRA);
  auto rand = 0U;
  for (auto i = 0U; i < NUM_LOOPS; ++i)
  {
    rand = GetRand();
  }
  REQUIRE(SEED != GetRandSeed());
  REQUIRE(rand != randJustAfterSave);

  std::ifstream fileIn{saveFile, std::ifstream::in};
  REQUIRE(not fileIn.fail());
  RestoreRandState(fileIn);
  REQUIRE(not fileIn.fail());
  fileOut.close();
  REQUIRE(not fileIn.fail());
  rand = GetRand();
  REQUIRE(SEED == GetRandSeed());
  REQUIRE(rand == randJustAfterSave);
}

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

  SetRandSeed(SEED + 1);
  REQUIRE((SEED + 1) == GetRandSeed());
  auto seq3    = std::vector<uint32_t>(NUM_LOOPS);
  auto fltSeq3 = std::vector<float>(NUM_LOOPS);
  for (auto i = 0U; i < NUM_LOOPS; ++i)
  {
    seq3[i]    = GetRand();
    fltSeq3[i] = GetRandInRange(0.0F, 1.0F);
  }

  REQUIRE(seq1 == seq2);
  REQUIRE(seq1 != seq3);

  REQUIRE(fltSeq1 == fltSeq2);
  REQUIRE(fltSeq1 != fltSeq3);

  SetRandSeed(SEED);
  REQUIRE(SEED == GetRandSeed());
}

template<typename ValType>
auto GetMinMax(const size_t numLoop, const ValType& nMin, const ValType& nMax)
    -> std::tuple<ValType, ValType>
{
  auto min = std::numeric_limits<ValType>::max();
  auto max = std::numeric_limits<ValType>::min();
  for (auto i = 0U; i < numLoop; ++i)
  {
    const auto rand = GetRandInRange(nMin, nMax);
    if (rand < min)
    {
      min = rand;
    }
    if (rand > max)
    {
      max = rand;
    }
  }

  return std::make_tuple(min, max);
}

TEST_CASE("uint32_t min max get random")
{
  // After a big enough loop, a good random distribution should have
  // covered the entire range: nMin <= n < nMax
  static constexpr auto NUM_LOOPS = 100000U;

  static constexpr auto N_MIN1 = 999U;
  static constexpr auto N_MAX1 = 10001U;
  const auto [min1, max1]      = GetMinMax(NUM_LOOPS, N_MIN1, N_MAX1);
  REQUIRE(min1 == N_MIN1);
  REQUIRE(max1 == N_MAX1 - 1);

  static constexpr auto N_MIN2 = 0U;
  static constexpr auto N_MAX2 = 120U;
  const auto [min2, max2]      = GetMinMax(NUM_LOOPS, N_MIN2, N_MAX2);
  REQUIRE(min2 == N_MIN2);
  REQUIRE(max2 == N_MAX2 - 1);

  REQUIRE_NOTHROW(GetRandInRange(5U, 6U));
#ifndef NDEBUG
  REQUIRE_THROWS_WITH(GetRandInRange(5U, 1U), StartsWith("uint n0"));
#endif
}

TEST_CASE("int32_t min max get random")
{
  // After a big enough loop, a good random distribution should have
  // covered the entire range: nMin <= n < nMax
  static constexpr auto NUM_LOOPS = 100000U;

  static constexpr auto N_MIN1 = -999;
  static constexpr auto N_MAX1 = 10001;
  const auto [min1, max1]      = GetMinMax(NUM_LOOPS, N_MIN1, N_MAX1);
  REQUIRE(min1 == N_MIN1);
  REQUIRE(max1 == N_MAX1 - 1);

  static constexpr auto N_MIN2 = -999;
  static constexpr auto N_MAX2 = -50;
  const auto [min2, max2]      = GetMinMax(NUM_LOOPS, N_MIN2, N_MAX2);
  REQUIRE(min2 == N_MIN2);
  REQUIRE(max2 == N_MAX2 - 1);

  static constexpr auto N_MIN3 = 1;
  static constexpr auto N_MAX3 = 999;
  const auto [min3, max3]      = GetMinMax(NUM_LOOPS, N_MIN3, N_MAX3);
  REQUIRE(min3 == N_MIN3);
  REQUIRE(max3 == N_MAX3 - 1);

  static constexpr auto N_MIN4 = 0;
  static constexpr auto N_MAX4 = 635;
  const auto [min4, max4]      = GetMinMax(NUM_LOOPS, N_MIN4, N_MAX4);
  REQUIRE(min4 == N_MIN4);
  REQUIRE(max4 == N_MAX4 - 1);

  REQUIRE_NOTHROW(GetRandInRange(5, 6));
  REQUIRE_NOTHROW(GetRandInRange(-6, -5));
  REQUIRE_NOTHROW(GetRandInRange(-6, 10));
#ifndef NDEBUG
  REQUIRE_THROWS_WITH(GetRandInRange(-5, -6), StartsWith("int n0"));
  REQUIRE_THROWS_WITH(GetRandInRange(5, 1), StartsWith("int n0"));
  REQUIRE_THROWS_WITH(GetRandInRange(5, -1), StartsWith("int n0"));
#endif
}

TEST_CASE("float min max get random")
{
  // After a big enough loop, a good random distribution should have
  // covered the entire range: nMin <= n < nMax
  static constexpr auto NUM_LOOPS = 1000000U;

  static constexpr auto N_MIN1 = 0.0F;
  static constexpr auto N_MAX1 = 1.0F;
  const auto [min1, max1]      = GetMinMax(NUM_LOOPS, N_MIN1, N_MAX1);
  REQUIRE(std::fabs(min1 - N_MIN1) < SMALL_VAL);
  REQUIRE(std::fabs(max1 - N_MAX1) < SMALL_VAL);

  static constexpr auto N_MIN2 = -1.0F;
  static constexpr auto N_MAX2 = 0.0F;
  const auto [min2, max2]      = GetMinMax(NUM_LOOPS, N_MIN2, N_MAX2);
  REQUIRE(std::fabs(min2 - N_MIN2) < SMALL_VAL);
  REQUIRE(std::fabs(max2 - N_MAX2) < SMALL_VAL);

  static constexpr auto N_MIN3 = -10.0F;
  static constexpr auto N_MAX3 = +10.0F;
  const auto [min3, max3]      = GetMinMax(NUM_LOOPS, N_MIN3, N_MAX3);
  REQUIRE(std::fabs(min3 - N_MIN3) < SMALL_VAL);
  REQUIRE(std::fabs(max3 - N_MAX3) < SMALL_VAL);

  static constexpr auto POS_MIN = 5.0F;
  static constexpr auto POS_MAX = 6.0F;
  static constexpr auto NEG_MIN = -6.0F;
  static constexpr auto NEG_MAX = 5.0F;
  REQUIRE_NOTHROW(GetRandInRange(POS_MIN, POS_MAX));
  REQUIRE_NOTHROW(GetRandInRange(NEG_MIN, NEG_MAX));
  REQUIRE_NOTHROW(GetRandInRange(NEG_MIN, POS_MAX));
#ifndef NDEBUG
  REQUIRE_THROWS_WITH(GetRandInRange(NEG_MAX, NEG_MIN), StartsWith("float x0"));
  REQUIRE_THROWS_WITH(GetRandInRange(POS_MIN, 1.0F), StartsWith("float x0"));
  REQUIRE_THROWS_WITH(GetRandInRange(POS_MIN, -1.0F), StartsWith("float x0"));
#endif
}
// NOLINTEND(readability-function-cognitive-complexity)

} // namespace GOOM::UNIT_TESTS
