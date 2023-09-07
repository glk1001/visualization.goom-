// clang-format off
// NOLINTBEGIN: Not my code

#pragma once

#include "goom/goom_config.h"
#include "thread_pool.h"

#include <algorithm>
#include <cstdint>
#include <future>
#include <thread>
#include <vector>

// clang-format off

namespace GOOM::UTILS
{

auto GetNumAvailablePoolThreads() noexcept -> int32_t;

class Parallel
{
public:
  // numPoolThreads > 0:  use this number of threads in the pool
  // numPoolThreads <= 0: use (max cores - numPoolThreads) in the pool
  explicit Parallel(int32_t numPoolThreads = 0) noexcept;

  auto GetNumThreadsUsed() const noexcept -> size_t;
  auto GetThreadIds() const noexcept -> std::vector<std::thread::id>;

  template<typename Callable>
  auto ForLoop(size_t numIters, Callable loopFunc) noexcept -> void;

private:
  ThreadPool m_threadPool;
  bool m_forLoopInUse = false;
};

inline Parallel::Parallel(const int32_t numPoolThreads) noexcept
  : m_threadPool{
        (numPoolThreads <= 0)
            ? static_cast<size_t>(std::max(
                  1, static_cast<int32_t>(std::thread::hardware_concurrency()) + numPoolThreads))
            : static_cast<size_t>(std::min(
                  numPoolThreads, static_cast<int32_t>(std::thread::hardware_concurrency())))}
{
}

inline auto Parallel::GetNumThreadsUsed() const noexcept -> size_t
{
  return m_threadPool.GetNumWorkers();
}

inline auto Parallel::GetThreadIds() const noexcept -> std::vector<std::thread::id>
{
  return m_threadPool.GetThreadIds();
}

template<typename Callable>
auto Parallel::ForLoop(const size_t numIters, const Callable loopFunc) noexcept -> void
{
  Expects(0 != numIters);
  Expects(not m_forLoopInUse);
  m_forLoopInUse = true;

  const size_t numThreads = std::min(numIters, m_threadPool.GetNumWorkers());

  if (1 == numThreads)
  {
    for (size_t i = 0; i < numIters; ++i)
    {
      loopFunc(i);
    }
    m_forLoopInUse = false;
    return;
  }

  const size_t chunkSize = numIters / numThreads; // >= 1
  const size_t numLeftoverIters = numIters - (numThreads * chunkSize);

  const auto loopContents =
      [&chunkSize, &numThreads, &numLeftoverIters, &loopFunc](const uint32_t threadIndex)
  {
    const size_t inclusiveStartIndex = threadIndex * chunkSize;
    const size_t exclusiveEndIndex =
        inclusiveStartIndex + chunkSize + (threadIndex < (numThreads - 1) ? 0 : numLeftoverIters);

    for (size_t k = inclusiveStartIndex; k < exclusiveEndIndex; ++k)
    {
      loopFunc(k);
    }
  };

  std::vector<std::future<void>> futures{};
  for (uint32_t j = 0; j < numThreads; ++j)
  {
    futures.emplace_back(m_threadPool.ScheduleAndGetFuture(loopContents, j));
  }

  for (const auto& future : futures)
  {
    future.wait();
  }

  m_forLoopInUse = false;
}

inline auto GetNumAvailablePoolThreads() noexcept -> int32_t
{
  static constexpr auto DESIRED_NUM_THREADS = 4;
  static constexpr auto MIN_NUM_THREADS     = 2;
  static constexpr auto NUM_FREE_THREADS    = 2;

  const auto numHardwareThreads = static_cast<int32_t>(std::thread::hardware_concurrency());

  if (DESIRED_NUM_THREADS <= (numHardwareThreads - NUM_FREE_THREADS))
  {
    return DESIRED_NUM_THREADS;
  }

  return std::max(MIN_NUM_THREADS, numHardwareThreads - NUM_FREE_THREADS);
}

} // namespace GOOM::UTILS

// NOLINTEND
// clang-format on
