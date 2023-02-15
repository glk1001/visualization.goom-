#include "catch2/catch.hpp"
#include "utils/parallel_utils.h"

#include <set>
#include <thread>
#include <vector>

namespace GOOM::UNIT_TESTS
{

using UTILS::Parallel;

TEST_CASE("Test Parallel Utils", "[ParallelFor]")
{
  auto threadsUsed = std::set<std::thread::id>{};
  auto mutex       = std::mutex{};

  static constexpr auto ARRAY_LEN = 100000U;
  auto testArray                  = std::vector<uint64_t>(ARRAY_LEN);
  static constexpr auto FIXED_VAL = 33UL;
  const auto func                 = [](const uint64_t i) { return i * FIXED_VAL + i * i; };
  const auto assignFunc           = [&testArray, &func, &threadsUsed, &mutex](const uint64_t i)
  {
    testArray[i]    = func(i);
    const auto lock = std::lock_guard<std::mutex>{mutex};
    threadsUsed.emplace(std::this_thread::get_id());
  };

  const auto checkResults =
      [&testArray, &threadsUsed, &func](const Parallel& parallel, const uint32_t numThreadsUsed)
  {
    for (auto i = 0U; i < ARRAY_LEN; ++i)
    {
      REQUIRE(testArray[i] == func(i));
    }
    REQUIRE(parallel.GetNumThreadsUsed() == numThreadsUsed);
    REQUIRE(threadsUsed.size() == numThreadsUsed);
  };

  auto parallel       = std::make_unique<Parallel>(-1);
  auto numThreadsUsed = std::thread::hardware_concurrency() - 1;
  threadsUsed.clear();
  parallel->ForLoop(ARRAY_LEN, assignFunc);
  checkResults(*parallel, numThreadsUsed);

  parallel       = std::make_unique<Parallel>(-2);
  numThreadsUsed = std::thread::hardware_concurrency() - 2;
  threadsUsed.clear();
  parallel->ForLoop(ARRAY_LEN, assignFunc);
  checkResults(*parallel, numThreadsUsed);

  parallel       = std::make_unique<Parallel>(1);
  numThreadsUsed = 1;
  threadsUsed.clear();
  parallel->ForLoop(ARRAY_LEN, assignFunc);
  checkResults(*parallel, numThreadsUsed);
}

} // namespace GOOM::UNIT_TESTS
