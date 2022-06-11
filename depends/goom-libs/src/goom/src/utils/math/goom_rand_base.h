#pragma once

#include "goom_config.h"
#include "misc.h"
#include "utils/enumutils.h"

#include <array>
#include <cstdint>
#include <format>
#include <map>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace GOOM::UTILS::MATH
{

class IGoomRand
{
public:
  IGoomRand() noexcept = default;
  IGoomRand(const IGoomRand&) noexcept = delete;
  IGoomRand(IGoomRand&&) noexcept = delete;
  auto operator=(const IGoomRand&) noexcept -> IGoomRand& = delete;
  auto operator=(IGoomRand&&) noexcept -> IGoomRand& = delete;
  virtual ~IGoomRand() noexcept = default;

  // Return random integer in the range 0 <= n < n1.
  [[nodiscard]] virtual auto GetNRand(uint32_t n1) const noexcept -> uint32_t = 0;
  // Return random integer in the range 0 <= n < randMax.
  [[nodiscard]] virtual auto GetRand() const noexcept -> uint32_t = 0;

  // Return random number in the range n0 <= n < n1.
  template<typename T>
  struct NumberRange
  {
    T min;
    T max;
  };
  template<typename T>
  [[nodiscard]] auto GetRandInRange(const NumberRange<T>& numberRange) const noexcept -> T;
  [[nodiscard]] virtual auto GetRandInRange(uint32_t n0, uint32_t n1) const noexcept
      -> uint32_t = 0;
  [[nodiscard]] virtual auto GetRandInRange(int32_t n0, int32_t n1) const noexcept -> int32_t = 0;
  [[nodiscard]] virtual auto GetRandInRange(float n0, float n1) const noexcept -> float = 0;

  template<class RandomIt>
  void Shuffle(RandomIt first, RandomIt last) const noexcept;

  // Return prob(m/n)
  [[nodiscard]] virtual auto ProbabilityOfMInN(uint32_t m, uint32_t n) const noexcept -> bool = 0;
  [[nodiscard]] virtual auto ProbabilityOf(float x) const noexcept -> bool = 0;
};

template<class E>
class Weights
{
public:
  using EventWeightPairs = std::vector<std::pair<E, float>>;

  Weights(const IGoomRand& goomRand, const EventWeightPairs& weights) noexcept;

  [[nodiscard]] auto GetNumElements() const noexcept -> size_t;
  [[nodiscard]] auto GetWeight(const E& enumClass) const noexcept -> float;
  [[nodiscard]] auto GetSumOfWeights() const noexcept -> float;

  [[nodiscard]] auto GetRandomWeighted() const noexcept -> E;

private:
  template<class F>
  friend class ConditionalWeights;
  [[nodiscard]] auto GetRandomWeighted(const E& given) const noexcept -> E;
  const IGoomRand& m_goomRand;
  using WeightArray = std::array<float, NUM<E>>;
  const WeightArray m_weights;
  const float m_sumOfWeights;
  [[nodiscard]] static auto GetWeights(const EventWeightPairs& eventWeightPairs) noexcept
      -> WeightArray;
  [[nodiscard]] static auto GetSumOfWeights(const EventWeightPairs& eventWeightPairs) noexcept
      -> float;
};

template<class E>
class ConditionalWeights
{
public:
  // Array of pairs: 'given' enum and corresponding array of weight multipliers.
  using EventWeightMultiplierPairs = std::vector<std::pair<E, std::map<E, float>>>;

  ConditionalWeights(const IGoomRand& goomRand,
                     const typename Weights<E>::EventWeightPairs& eventWeightPairs,
                     bool disallowEventsSameAsGiven = true) noexcept;
  ConditionalWeights(const IGoomRand& goomRand,
                     const typename Weights<E>::EventWeightPairs& weights,
                     const EventWeightMultiplierPairs& weightMultiplierPairs,
                     bool disallowEventsSameAsGiven = true) noexcept;

  [[nodiscard]] auto GetWeight(const E& given, const E& enumClass) const noexcept -> float;
  [[nodiscard]] auto GetSumOfWeights(const E& given) const noexcept -> float;

  [[nodiscard]] auto GetRandomWeighted(const E& given) const noexcept -> E;

private:
  const Weights<E> m_unconditionalWeights;
  const std::map<E, Weights<E>> m_conditionalWeights;
  const bool m_disallowEventsSameAsGiven;
  [[nodiscard]] static auto GetConditionalWeightMap(
      const IGoomRand& goomRand,
      const typename Weights<E>::EventWeightPairs& eventWeightPairs,
      const EventWeightMultiplierPairs& weightMultiplierPairs) noexcept -> std::map<E, Weights<E>>;
};

template<typename T>
inline auto IGoomRand::GetRandInRange(const NumberRange<T>& numberRange) const noexcept -> T
{
  if (std::is_integral<T>())
  {
    return GetRandInRange(numberRange.min, numberRange.max + 1);
  }
  return GetRandInRange(numberRange.min, numberRange.max);
}

template<class RandomIt>
inline void IGoomRand::Shuffle(RandomIt first, RandomIt last) const noexcept
{
  using DiffType = typename std::iterator_traits<RandomIt>::difference_type;

  const DiffType n = last - first;
  for (DiffType i = n - 1; i > 0; --i)
  {
    std::swap(first[i], first[GetRandInRange(0, static_cast<int32_t>(i + 1))]);
  }
}

template<class E>
Weights<E>::Weights(const IGoomRand& goomRand, const EventWeightPairs& weights) noexcept
  : m_goomRand{goomRand}, m_weights{GetWeights(weights)}, m_sumOfWeights{GetSumOfWeights(weights)}
{
  Expects(m_sumOfWeights > SMALL_FLOAT);
}

template<class E>
inline auto Weights<E>::GetWeights(const EventWeightPairs& eventWeightPairs) noexcept -> WeightArray
{
  WeightArray weightArray{0};
  for (const auto& [e, w] : eventWeightPairs)
  {
    weightArray.at(static_cast<size_t>(e)) = w;
  }

  return weightArray;
}

template<class E>
inline auto Weights<E>::GetSumOfWeights(const EventWeightPairs& eventWeightPairs) noexcept -> float
{
  float sumOfWeights = 0.0F;
  for (const auto& eventWeightPair : eventWeightPairs)
  {
    sumOfWeights += eventWeightPair.second;
  }
  return sumOfWeights - SMALL_FLOAT;
}

template<class E>
inline auto Weights<E>::GetNumElements() const noexcept -> size_t
{
  return m_weights.size();
}

template<class E>
inline auto Weights<E>::GetWeight(const E& enumClass) const noexcept -> float
{
  for (size_t i = 0; i < m_weights.size(); ++i)
  {
    if (static_cast<E>(i) == enumClass)
    {
      return m_weights[i];
    }
  }

  return 0.0F;
}

template<class E>
inline auto Weights<E>::GetSumOfWeights() const noexcept -> float
{
  return m_sumOfWeights;
}

template<class E>
inline auto Weights<E>::GetRandomWeighted() const noexcept -> E
{
  return GetRandomWeighted(E::_num);
}

template<class E>
inline auto Weights<E>::GetRandomWeighted(const E& given) const noexcept -> E
{
  const float sumOfWeights = (given == E::_num)
                                 ? m_sumOfWeights
                                 : (m_sumOfWeights - m_weights[static_cast<size_t>(given)]);

  float randVal = m_goomRand.GetRandInRange(0.0F, sumOfWeights);

  for (size_t i = 0; i < m_weights.size(); ++i)
  {
    if (static_cast<E>(i) == given)
    {
      continue;
    }
    if (randVal < m_weights[i])
    {
      return static_cast<E>(i);
    }
    randVal -= m_weights[i];
  }

  FailFast();
  return static_cast<E>(0);
}

template<class E>
inline ConditionalWeights<E>::ConditionalWeights(
    const IGoomRand& goomRand,
    const typename Weights<E>::EventWeightPairs& eventWeightPairs,
    const bool disallowEventsSameAsGiven) noexcept
  : m_unconditionalWeights{goomRand, eventWeightPairs},
    m_conditionalWeights{},
    m_disallowEventsSameAsGiven{disallowEventsSameAsGiven}
{
}

template<class E>
inline ConditionalWeights<E>::ConditionalWeights(
    const IGoomRand& goomRand,
    const typename Weights<E>::EventWeightPairs& weights,
    const EventWeightMultiplierPairs& weightMultiplierPairs,
    const bool disallowEventsSameAsGiven) noexcept
  : m_unconditionalWeights{goomRand, weights},
    m_conditionalWeights{GetConditionalWeightMap(goomRand, weights, weightMultiplierPairs)},
    m_disallowEventsSameAsGiven{disallowEventsSameAsGiven}
{
}

template<class E>
inline auto ConditionalWeights<E>::GetWeight(const E& given, const E& enumClass) const noexcept
    -> float
{
  const auto iter = m_conditionalWeights.find(given);
  if (iter == cend(m_conditionalWeights))
  {
    return m_unconditionalWeights.GetWeight(enumClass);
  }
  return iter->second.GetWeight(enumClass);
}

template<class E>
inline auto ConditionalWeights<E>::GetSumOfWeights(const E& given) const noexcept -> float
{
  const auto iter = m_conditionalWeights.find(given);
  if (iter == cend(m_conditionalWeights))
  {
    return m_unconditionalWeights.GetSumOfWeights();
  }
  return iter->second.GetSumOfWeights();
}

template<class E>
inline auto ConditionalWeights<E>::GetRandomWeighted(const E& given) const noexcept -> E
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

  const Weights<E>& weights = iter->second;
  return weights.GetRandomWeighted();
}

template<class E>
auto ConditionalWeights<E>::GetConditionalWeightMap(
    const IGoomRand& goomRand,
    const typename Weights<E>::EventWeightPairs& eventWeightPairs,
    const EventWeightMultiplierPairs& weightMultiplierPairs) noexcept -> std::map<E, Weights<E>>
{
  std::map<E, Weights<E>> conditionalWeights{};
  for (const auto& weightMultiplierPair : weightMultiplierPairs)
  {
    const E& given = weightMultiplierPair.first;
    const std::map<E, float>& multiplierPairs = weightMultiplierPair.second;

    typename Weights<E>::EventWeightPairs newEventWeightPairs = eventWeightPairs;
    for (auto& newEventWeightPair : newEventWeightPairs)
    {
      if (multiplierPairs.find(newEventWeightPair.first) == cend(multiplierPairs))
      {
        continue;
      }
      newEventWeightPair.second *= multiplierPairs.at(newEventWeightPair.first);
    }

    conditionalWeights.emplace(given, Weights<E>{goomRand, newEventWeightPairs});
  }

  return conditionalWeights;
}

} // namespace GOOM::UTILS::MATH
