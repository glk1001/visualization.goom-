module;

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <numeric>
#include <type_traits>
#include <vector>

export module Goom.Utils.Math.GoomRandBase;

import Goom.Utils.EnumUtils;
import Goom.Utils.Math.Misc;
import Goom.Lib.AssertUtils;

export namespace GOOM::UTILS::MATH
{

template<typename T>
class NumberRange
{
public:
  constexpr NumberRange() = default;
  constexpr NumberRange(T min, T max) noexcept;

  [[nodiscard]] constexpr auto Min() const noexcept -> T;
  [[nodiscard]] constexpr auto Max() const noexcept -> T;
  [[nodiscard]] constexpr auto Range() const noexcept -> T;

private:
  T m_min;
  T m_max;
};

template<typename T>
NumberRange(T, T) -> NumberRange<T>;

template<typename T>
constexpr NumberRange<T>::NumberRange(const T min, const T max) noexcept : m_min{min}, m_max{max}
{
  Expects(min <= max);
}

inline constexpr auto UNIT_RANGE        = NumberRange{0.0F, 1.0F};
inline constexpr auto FULL_CIRCLE_RANGE = NumberRange{0.0F, TWO_PI};
inline constexpr auto HALF_CIRCLE_RANGE = NumberRange{0.0F, PI};

template<typename T>
[[nodiscard]] constexpr auto GetMidpoint(const NumberRange<T>& numberRange) noexcept;

class IGoomRand
{
public:
  IGoomRand() noexcept                                    = default;
  IGoomRand(const IGoomRand&) noexcept                    = delete;
  IGoomRand(IGoomRand&&) noexcept                         = delete;
  auto operator=(const IGoomRand&) noexcept -> IGoomRand& = delete;
  auto operator=(IGoomRand&&) noexcept -> IGoomRand&      = delete;
  virtual ~IGoomRand() noexcept                           = default;

  // Return random integer in the range 0 <= n < n1.
  [[nodiscard]] virtual auto GetNRand(uint32_t n1) const noexcept -> uint32_t = 0;

  // Return random number in the range n0 <= n < n1.
  template<typename T>
  [[nodiscard]] auto GetRandInRange(const NumberRange<T>& numberRange) const noexcept -> T;

  template<std::ranges::random_access_range Range>
  auto Shuffle(Range& range) const noexcept -> void;

  [[nodiscard]] virtual auto ProbabilityOf(float x) const noexcept -> bool = 0;

protected:
  // Return random number in the range n0 <= n <= n1.
  [[nodiscard]] virtual auto GetRandInRange(uint32_t n0,
                                            uint32_t n1) const noexcept -> uint32_t           = 0;
  [[nodiscard]] virtual auto GetRandInRange(int32_t n0, int32_t n1) const noexcept -> int32_t = 0;
  [[nodiscard]] virtual auto GetRandInRange(float n0, float n1) const noexcept -> float       = 0;
  [[nodiscard]] virtual auto GetRandInRange(double n0, double n1) const noexcept -> double    = 0;
};

template<EnumType E>
class Weights
{
public:
  struct KeyValue
  {
    E key;
    float weight;
  };
  using EventWeightPairs = std::vector<KeyValue>;

  Weights() noexcept = default;
  Weights(const IGoomRand& goomRand, const EventWeightPairs& weights) noexcept;

  [[nodiscard]] auto GetNumElements() const noexcept -> size_t;
  [[nodiscard]] auto GetNumSetWeights() const noexcept -> size_t;
  [[nodiscard]] auto GetWeight(const E& enumClass) const noexcept -> float;
  [[nodiscard]] auto GetSumOfWeights() const noexcept -> float;

  [[nodiscard]] auto GetRandomWeighted() const noexcept -> E;

private:
  template<EnumType F>
  friend class ConditionalWeights;
  [[nodiscard]] auto GetRandomWeighted(const E& given) const noexcept -> E;

  const IGoomRand* m_goomRand = nullptr;
  using WeightArray           = std::array<float, NUM<E>>;
  struct WeightData
  {
    WeightArray weightArray{0.0F};
    size_t numSetWeights = 0;
  };
  WeightData m_weightData{};
  float m_sumOfWeights = 0.0F;

  [[nodiscard]] static auto GetWeightData(const EventWeightPairs& eventWeightPairs) noexcept
      -> WeightData;
  [[nodiscard]] static auto GetSumOfWeights(const EventWeightPairs& eventWeightPairs) noexcept
      -> float;
};

template<EnumType E>
class ConditionalWeights
{
public:
  struct KeyValue
  {
    E key{};
    std::map<E, float> weightMultipliers{};
  };
  using EventWeightMultiplierPairs = std::vector<KeyValue>;

  ConditionalWeights(const IGoomRand& goomRand,
                     const typename Weights<E>::EventWeightPairs& eventWeightPairs,
                     bool disallowEventsSameAsGiven = true) noexcept;
  ConditionalWeights(const IGoomRand& goomRand,
                     const typename Weights<E>::EventWeightPairs& weights,
                     const EventWeightMultiplierPairs& weightMultiplierPairs,
                     bool disallowEventsSameAsGiven = true) noexcept;

  struct ConditionalEvents
  {
    E previousEvent;
    E event;
  };
  [[nodiscard]] auto GetNumSetWeights() const noexcept -> size_t;
  [[nodiscard]] auto GetWeight(const ConditionalEvents& events) const noexcept -> float;
  [[nodiscard]] auto GetSumOfWeights(const E& given) const noexcept -> float;

  [[nodiscard]] auto GetRandomWeighted(const E& given) const noexcept -> E;

private:
  Weights<E> m_unconditionalWeights;
  std::map<E, Weights<E>> m_conditionalWeights;
  bool m_disallowEventsSameAsGiven;
  [[nodiscard]] static auto GetConditionalWeightMap(
      const IGoomRand& goomRand,
      const typename Weights<E>::EventWeightPairs& eventWeightPairs,
      const EventWeightMultiplierPairs& weightMultiplierPairs) noexcept -> std::map<E, Weights<E>>;
};

} // namespace GOOM::UTILS::MATH

namespace GOOM::UTILS::MATH
{

template<typename T>
constexpr auto NumberRange<T>::Min() const noexcept -> T
{
  return m_min;
}

template<typename T>
constexpr auto NumberRange<T>::Max() const noexcept -> T
{
  return m_max;
}

template<typename T>
constexpr auto NumberRange<T>::Range() const noexcept -> T
{
  return Max() - Min();
}

template<typename T>
constexpr auto GetMidpoint(const NumberRange<T>& numberRange) noexcept
{
  return std::midpoint(numberRange.Min(), numberRange.Max());
}

template<typename T>
auto IGoomRand::GetRandInRange(const NumberRange<T>& numberRange) const noexcept -> T
{
  if (std::is_integral<T>())
  {
    return GetRandInRange(numberRange.Min(), numberRange.Max() + 1);
  }
  return GetRandInRange(numberRange.Min(), numberRange.Max());
}

template<std::ranges::random_access_range Range>
auto IGoomRand::Shuffle(Range& range) const noexcept -> void
{
  auto first = std::ranges::begin(range);
  auto last  = std::ranges::end(range);

  for (auto i = last - first - 1; i > 0; --i)
  {
    using std::swap;
    swap(first[i], first[GetRandInRange(0U, static_cast<uint32_t>(i) + 1)]);
  }
}

template<EnumType E>
Weights<E>::Weights(const IGoomRand& goomRand, const EventWeightPairs& weights) noexcept
  : m_goomRand{&goomRand},
    m_weightData{GetWeightData(weights)},
    m_sumOfWeights{GetSumOfWeights(weights)}
{
  Expects(m_sumOfWeights > SMALL_FLOAT);
}

template<EnumType E>
auto Weights<E>::GetWeightData(const EventWeightPairs& eventWeightPairs) noexcept -> WeightData
{
  WeightData weightData{};
  for (const auto& [e, w] : eventWeightPairs)
  {
    weightData.weightArray.at(static_cast<size_t>(e)) = w;
    ++weightData.numSetWeights;
  }

  return weightData;
}

template<EnumType E>
auto Weights<E>::GetSumOfWeights(const EventWeightPairs& eventWeightPairs) noexcept -> float
{
  auto sumOfWeights = 0.0F;
  for (const auto& eventWeightPair : eventWeightPairs)
  {
    sumOfWeights += eventWeightPair.weight;
  }
  return sumOfWeights - SMALL_FLOAT;
}

template<EnumType E>
auto Weights<E>::GetNumElements() const noexcept -> size_t
{
  return m_weightData.weightArray.size();
}

template<EnumType E>
auto Weights<E>::GetNumSetWeights() const noexcept -> size_t
{
  return m_weightData.numSetWeights;
}

template<EnumType E>
auto Weights<E>::GetWeight(const E& enumClass) const noexcept -> float
{
  for (auto i = 0U; i < m_weightData.weightArray.size(); ++i)
  {
    if (static_cast<E>(i) == enumClass)
    {
      return m_weightData.weightArray[i];
    }
  }

  return 0.0F;
}

template<EnumType E>
auto Weights<E>::GetSumOfWeights() const noexcept -> float
{
  return m_sumOfWeights;
}

template<EnumType E>
auto Weights<E>::GetRandomWeighted() const noexcept -> E
{
  Expects(NUM<E> == m_weightData.weightArray.size());

  auto randVal = m_goomRand->GetRandInRange(NumberRange{0.0F, m_sumOfWeights});

  for (auto i = 0U; i < m_weightData.weightArray.size(); ++i)
  {
    if (randVal < m_weightData.weightArray[i])
    {
      // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange): Seems broken
      return static_cast<E>(i);
    }
    randVal -= m_weightData.weightArray[i];
  }

  FailFast();
}

template<EnumType E>
auto Weights<E>::GetRandomWeighted(const E& given) const noexcept -> E
{
  Expects(NUM<E> == m_weightData.weightArray.size());

  const auto sumOfWeights = m_sumOfWeights - m_weightData.weightArray[static_cast<size_t>(given)];

  auto randVal = m_goomRand->GetRandInRange(NumberRange{0.0F, sumOfWeights});

  for (auto i = 0U; i < m_weightData.weightArray.size(); ++i)
  {
    if (static_cast<E>(i) == given)
    {
      continue;
    }
    if (randVal < m_weightData.weightArray[i])
    {
      return static_cast<E>(i);
    }
    randVal -= m_weightData.weightArray[i];
  }

  FailFast();
}

template<EnumType E>
ConditionalWeights<E>::ConditionalWeights(
    const IGoomRand& goomRand,
    const typename Weights<E>::EventWeightPairs& eventWeightPairs,
    const bool disallowEventsSameAsGiven) noexcept
  : m_unconditionalWeights{goomRand, eventWeightPairs},
    m_conditionalWeights{},
    m_disallowEventsSameAsGiven{disallowEventsSameAsGiven}
{
}

template<EnumType E>
ConditionalWeights<E>::ConditionalWeights(const IGoomRand& goomRand,
                                          const typename Weights<E>::EventWeightPairs& weights,
                                          const EventWeightMultiplierPairs& weightMultiplierPairs,
                                          const bool disallowEventsSameAsGiven) noexcept
  : m_unconditionalWeights{goomRand, weights},
    m_conditionalWeights{GetConditionalWeightMap(goomRand, weights, weightMultiplierPairs)},
    m_disallowEventsSameAsGiven{disallowEventsSameAsGiven}
{
}

template<EnumType E>
auto ConditionalWeights<E>::GetNumSetWeights() const noexcept -> size_t
{
  return m_unconditionalWeights.GetNumSetWeights();
}

template<EnumType E>
auto ConditionalWeights<E>::GetWeight(const ConditionalEvents& events) const noexcept -> float
{
  const auto iter = m_conditionalWeights.find(events.previousEvent);
  if (iter == cend(m_conditionalWeights))
  {
    return m_unconditionalWeights.GetWeight(events.event);
  }
  return iter->second.GetWeight(events.event);
}

template<EnumType E>
auto ConditionalWeights<E>::GetSumOfWeights(const E& given) const noexcept -> float
{
  const auto iter = m_conditionalWeights.find(given);
  if (iter == cend(m_conditionalWeights))
  {
    return m_unconditionalWeights.GetSumOfWeights();
  }
  return iter->second.GetSumOfWeights();
}

template<EnumType E>
auto ConditionalWeights<E>::GetRandomWeighted(const E& given) const noexcept -> E
{
  const auto iter = m_conditionalWeights.find(given);
  if (iter == cend(m_conditionalWeights))
  {
    if (m_disallowEventsSameAsGiven)
    {
      return m_unconditionalWeights.GetRandomWeighted(given);
    }
    return m_unconditionalWeights.GetRandomWeighted();
  }

  const auto& weights = iter->second;
  return weights.GetRandomWeighted();
}

template<EnumType E>
auto ConditionalWeights<E>::GetConditionalWeightMap(
    const IGoomRand& goomRand,
    const typename Weights<E>::EventWeightPairs& eventWeightPairs,
    const EventWeightMultiplierPairs& weightMultiplierPairs) noexcept -> std::map<E, Weights<E>>
{
  auto conditionalWeights = std::map<E, Weights<E>>{};
  for (const auto& [given, multiplierPairs] : weightMultiplierPairs)
  {
    typename Weights<E>::EventWeightPairs newEventWeightPairs = eventWeightPairs;
    for (auto& newEventWeightPair : newEventWeightPairs)
    {
      if (multiplierPairs.find(newEventWeightPair.key) == cend(multiplierPairs))
      {
        continue;
      }
      newEventWeightPair.weight *= multiplierPairs.at(newEventWeightPair.key);
    }

    conditionalWeights.emplace(given, Weights<E>{goomRand, newEventWeightPairs});
  }

  return conditionalWeights;
}

} // namespace GOOM::UTILS::MATH
