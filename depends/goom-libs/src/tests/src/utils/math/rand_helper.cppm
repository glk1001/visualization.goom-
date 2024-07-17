module;

#include <algorithm>
#include <cstdint>
#include <functional>
#include <limits>
//#include <map>
#include <numeric>
//#include <print>
#include <unordered_map>

export module Goom.Tests.Utils.Math.RandHelper;

import Goom.Utils.Math.GoomRandBase;
import Goom.Utils.Math.Rand.RandUtils;

template<typename T>
struct CountResults
{
  T min{std::numeric_limits<T>::max()};
  T max{std::numeric_limits<T>::min()};
  uint32_t numCounts      = 0U;
  uint32_t sumOfAllCounts = 0U;
  uint32_t minCount{std::numeric_limits<uint32_t>::max()};
  uint32_t maxCount{std::numeric_limits<uint32_t>::min()};
  T minCountAt{};
  T maxCountAt{};
};

export namespace GOOM::UNIT_TESTS
{

template<typename T, typename GetRandInRangeFunc>
auto GetCountResults(const size_t numLoops,
                     const T nMin,
                     const T nMax,
                     const GetRandInRangeFunc& getRandInRange) -> CountResults<T>
{
  auto countResults = CountResults<T>{};

  //using CountMap = std::map<T, uint32_t>;
  using CountMap = std::unordered_map<T, uint32_t>;
  auto counts    = CountMap{};
  for (auto i = 0U; i < numLoops; ++i)
  {
    const auto rand = getRandInRange(nMin, nMax);

    countResults.min = std::min(countResults.min, rand);
    countResults.max = std::max(countResults.max, rand);

    ++counts[rand];
  }

  countResults.numCounts  = static_cast<uint32_t>(counts.size());
  const auto lowest       = std::ranges::min_element(counts, {}, &CountMap::value_type::second);
  const auto highest      = std::ranges::max_element(counts, {}, &CountMap::value_type::second);
  countResults.minCountAt = lowest->first;
  countResults.minCount   = lowest->second;
  countResults.maxCountAt = highest->first;
  countResults.maxCount   = highest->second;

  countResults.sumOfAllCounts =
      std::accumulate(cbegin(counts),
                      cend(counts),
                      0U,
                      [](const uint32_t value, const typename CountMap::value_type& element)
                      { return value + element.second; });
  //
  // for (const auto& keyValue : counts)
  // {
  //   std::println("{}: {}", keyValue.first, keyValue.second);
  // }

  return countResults;
}

} // namespace GOOM::UNIT_TESTS
