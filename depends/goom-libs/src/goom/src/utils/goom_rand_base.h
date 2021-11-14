#pragma once

#include <cstdint>
#include <format>
#include <stdexcept>
#if __cplusplus <= 201402L
#include <tuple>
#endif
#include <type_traits>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace UTILS
{
#else
namespace GOOM::UTILS
{
#endif

class IGoomRand
{
public:
  IGoomRand() noexcept = default;
  IGoomRand(const IGoomRand&) noexcept = delete;
  IGoomRand(IGoomRand&&) noexcept = delete;
  auto operator=(const IGoomRand&) -> IGoomRand& = delete;
  auto operator=(IGoomRand&&) -> IGoomRand& = delete;
  virtual ~IGoomRand() noexcept = default;

  // Return random integer in the range 0 <= n < n1.
  [[nodiscard]] virtual auto GetNRand(uint32_t n1) -> uint32_t=0;
  // Return random integer in the range 0 <= n < randMax.
  [[nodiscard]] virtual auto GetRand() -> uint32_t=0;

  // Return random number in the range n0 <= n < n1.
  [[nodiscard]] virtual auto GetRandInRange(uint32_t n0, uint32_t n1) -> uint32_t=0;
  [[nodiscard]] virtual auto GetRandInRange(int32_t n0, int32_t n1) -> int32_t=0;
  [[nodiscard]] virtual auto GetRandInRange(float n0, float n1) -> float=0;
  template<typename T>
  struct NumberRange
  {
    T min;
    T max;
  };
  template<typename T>
  [[nodiscard]] auto GetRandInRange(const NumberRange<T>& numberRange) -> T;

  template<class RandomIt>
  void Shuffle(RandomIt first, RandomIt last);

  // Return prob(m/n)
  [[nodiscard]] virtual auto ProbabilityOfMInN(uint32_t m, uint32_t n) -> bool=0;
  [[nodiscard]] virtual auto ProbabilityOf(float x) -> bool = 0;
};

template<class E>
class Weights
{
public:
  Weights(IGoomRand& goomRand, const std::vector<std::pair<E, size_t>>& weights);

  [[nodiscard]] auto GetNumElements() const -> size_t;
  [[nodiscard]] auto GetWeight(E enumClass) const -> size_t;

  [[nodiscard]] auto GetSumOfWeights() const -> size_t { return m_sumOfWeights; }

  [[nodiscard]] auto GetRandomWeighted() const -> const E&;

private:
  IGoomRand& m_goomRand;
  const std::vector<std::pair<E, size_t>> m_weights;
  const size_t m_sumOfWeights;
  [[nodiscard]] static auto GetSumOfWeights(const std::vector<std::pair<E, size_t>>& weights)
      -> size_t;
};

template<typename T>
inline auto IGoomRand::GetRandInRange(const NumberRange<T>& numberRange) -> T
{
  if (std::is_integral<T>())
  {
    return GetRandInRange(numberRange.min, numberRange.max + 1);
  }
  return GetRandInRange(numberRange.min, numberRange.max);
}

template<class RandomIt>
inline void IGoomRand::Shuffle(RandomIt first, RandomIt last)
{
  using DiffType = typename std::iterator_traits<RandomIt>::difference_type;

  const DiffType n = last - first;
  for (DiffType i = n - 1; i > 0; --i)
  {
    std::swap(first[i], first[GetRandInRange(0, static_cast<int32_t>(i + 1))]);
  }
}

template<class E>
Weights<E>::Weights(IGoomRand& goomRand, const std::vector<std::pair<E, size_t>>& weights)
  : m_goomRand{goomRand}, m_weights{weights}, m_sumOfWeights{GetSumOfWeights(weights)}
{
}

template<class E>
auto Weights<E>::GetSumOfWeights(const std::vector<std::pair<E, size_t>>& weights) -> size_t
{
  size_t sumOfWeights = 0;
#if __cplusplus <= 201402L
  for (const auto& wgt : weights)
  {
    const auto& w = std::get<1>(wgt);
#else
  for (const auto& [e, w] : weights)
  {
#endif
    sumOfWeights += w;
  }
  return sumOfWeights;
}

template<class E>
auto Weights<E>::GetNumElements() const -> size_t
{
  return m_weights.size();
}

template<class E>
auto Weights<E>::GetWeight(const E enumClass) const -> size_t
{
#if __cplusplus <= 201402L
  for (const auto& wgt : m_weights)
  {
    const auto& e = std::get<0>(wgt);
    const auto& w = std::get<1>(wgt);
#else
  for (const auto& [e, w] : m_weights)
  {
#endif
    if (e == enumClass)
    {
      return w;
    }
  }
  return 0;
}

template<class E>
auto Weights<E>::GetRandomWeighted() const -> const E&
{
  if (m_weights.empty())
  {
    throw std::logic_error("The are no weights set.");
  }

  size_t randVal = m_goomRand.GetRandInRange(0U, static_cast<uint32_t>(m_sumOfWeights));
#if __cplusplus <= 201402L
  for (const auto& wgt : m_weights)
  {
    const auto& e = std::get<0>(wgt);
    const auto& w = std::get<1>(wgt);
#else
  for (const auto& [e, w] : m_weights)
  {
#endif
    if (randVal < w)
    {
      return e;
    }
    randVal -= w;
  }
  throw std::logic_error(std20::format("Should not get here. randVal = {}.", randVal));
}

#if __cplusplus <= 201402L
} // namespace UTILS
} // namespace GOOM
#else
} // namespace GOOM::UTILS
#endif
