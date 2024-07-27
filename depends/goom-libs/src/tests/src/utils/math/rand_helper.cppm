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
  std::map<T, uint32_t> counts{};
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

  for (auto i = 0U; i < numLoop; ++i)
  {
    const auto rand = getRandInRange(nMin, nMax);

    countResults.min = std::min(countResults.min, rand);
    countResults.max = std::max(countResults.max, rand);
    ++countResults.counts[rand];
  }

  return countResults;
}

} // namespace GOOM::UNIT_TESTS
