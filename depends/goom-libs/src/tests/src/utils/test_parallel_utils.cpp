// NOLINTBEGIN(cert-err58-cpp): Catch2 3.6.0 issue

#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

import Goom.Utils.Parallel;

namespace GOOM::UNIT_TESTS
{

using UTILS::Parallel;

// NOLINTBEGIN(bugprone-chained-comparison): Catch2 needs to fix this.
TEST_CASE("Test Parallel Utils", "[ParallelFor]")
{
  const auto numSupportedConcurrentThreads = std::thread::hardware_concurrency();
  if (static constexpr auto MIN_CONCURRENT_THREADS = 3U;
      numSupportedConcurrentThreads < MIN_CONCURRENT_THREADS)
  {
    return;
  }

  auto threadsUsed = std::set<std::thread::id>{};
  auto mutex       = std::mutex{};

  static constexpr auto ARRAY_LEN = 100000U;
  auto testArray                  = std::vector<uint64_t>(ARRAY_LEN);
  static constexpr auto FIXED_VAL = 33UL;
  const auto func                 = [](const uint64_t i) -> uint64_t
  { return static_cast<uint64_t>((i * FIXED_VAL) + (i * i)); };
  const auto assignFunc = [&testArray, &func, &threadsUsed, &mutex](const uint64_t i)
  {
    testArray[static_cast<uint32_t>(i)] = func(i);
    const auto lock                     = std::lock_guard<std::mutex>{mutex};
    threadsUsed.emplace(std::this_thread::get_id());
  };

  const auto checkResults = [&numSupportedConcurrentThreads, &testArray, &threadsUsed, &func](
                                const Parallel& parallel, const uint32_t expectedNumThreadsUsed)
  {
    for (auto i = 0U; i < ARRAY_LEN; ++i)
    {
      REQUIRE(testArray[i] == func(i));
    }
    UNSCOPED_INFO("numSupportedConcurrentThreads = " << numSupportedConcurrentThreads);
    REQUIRE(parallel.GetNumThreadsUsed() == expectedNumThreadsUsed);
    REQUIRE(threadsUsed.size() == expectedNumThreadsUsed);
  };

  auto parallel               = std::make_unique<Parallel>(-1);
  auto expectedNumThreadsUsed = numSupportedConcurrentThreads - 1;
  threadsUsed.clear();
  parallel->ForLoop(ARRAY_LEN, assignFunc);
  checkResults(*parallel, expectedNumThreadsUsed);

  parallel               = std::make_unique<Parallel>(-2);
  expectedNumThreadsUsed = numSupportedConcurrentThreads - 2;
  threadsUsed.clear();
  parallel->ForLoop(ARRAY_LEN, assignFunc);
  checkResults(*parallel, expectedNumThreadsUsed);

  parallel               = std::make_unique<Parallel>(1);
  expectedNumThreadsUsed = 1;
  threadsUsed.clear();
  parallel->ForLoop(ARRAY_LEN, assignFunc);
  checkResults(*parallel, expectedNumThreadsUsed);
}
// NOLINTEND(bugprone-chained-comparison)

} // namespace GOOM::UNIT_TESTS

// NOLINTEND(cert-err58-cpp): Catch2 3.6.0 issue
