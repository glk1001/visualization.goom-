module;

#include <cstdint>
#include <functional>
#include <limits>
#include <map>

export module Goom.Tests.Utils.Math.RandHelper;

import Goom.Utils.Math.GoomRandBase;
import Goom.Utils.Math.RandUtils;

template<typename T>
struct CountResults
{
  T min{std::numeric_limits<T>::max()};
  T max{std::numeric_limits<T>::min()};
  uint32_t numCounts = 0U;
  uint32_t minCount{std::numeric_limits<uint32_t>::max()};
  uint32_t maxCount{std::numeric_limits<uint32_t>::min()};
  T minCountAt{};
  T maxCountAt{};
};

export namespace GOOM::UNIT_TESTS
{

template<typename T, typename GetRandInRangeFunc>
auto GetCountResults(const size_t numLoop,
                     const T nMin,
                     const T nMax,
                     const GetRandInRangeFunc& getRandInRange) -> CountResults<T>
{
  auto countResults = CountResults<T>{};

  std::map<T, uint32_t> counts{};
  for (auto i = 0U; i < numLoop; ++i)
  {
    const auto rand = getRandInRange(nMin, nMax);

    ++counts[rand];

    countResults.min = std::min(countResults.min, rand);
    countResults.max = std::max(countResults.max, rand);

    if (countResults.minCount > counts[rand])
    {
      countResults.minCount = counts[rand];
      countResults.minCountAt = rand;
    }
    if (countResults.maxCount < counts[rand])
    {
      countResults.maxCount = counts[rand];
      countResults.maxCountAt = rand;
    }
  }

  countResults.numCounts = static_cast<uint32_t>(counts.size());

  return countResults;
}

} // namespace GOOM::UNIT_TESTS
