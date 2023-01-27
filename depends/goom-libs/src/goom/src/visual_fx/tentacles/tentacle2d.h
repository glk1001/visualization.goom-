#pragma once

#include "utils/math/damping_functions.h"

#include <memory>
#include <utility>
#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

class Tentacle2D
{
  using XAndYVectors = std::pair<std::vector<double>&, std::vector<double>&>;

public:
  static constexpr uint32_t MIN_NUM_NODES = 5U;

  struct LinearDimensions
  {
    double min;
    double max;
  };
  struct Dimensions
  {
    LinearDimensions xDimensions;
    LinearDimensions yDimensions;
  };
  struct BaseYWeights
  {
    float previous;
    float current;
  };

  Tentacle2D(uint32_t numNodes,
             const Dimensions& dimensions,
             const BaseYWeights& baseYWeights) noexcept;

  auto StartIterating() -> void;
  auto Iterate() -> void;

  auto SetXDimensions(const LinearDimensions& xDimensions) -> void;
  auto SetIterZeroYVal(double value) -> void;
  auto SetIterZeroLerpFactor(double value) -> void;
  auto SetBaseYWeights(const BaseYWeights& value) -> void;

  [[nodiscard]] auto GetDampedXAndYVectors() const -> const XAndYVectors&;

  [[nodiscard]] auto GetNumNodes() const -> uint32_t;
  [[nodiscard]] auto GetTentacleLength() const -> double;
  [[nodiscard]] auto GetXMin() const -> double;
  [[nodiscard]] auto GetXMax() const -> double;

private:
  const uint32_t m_numRequestedNodes;
  const uint32_t m_numActualNodes;
  Dimensions m_dimensions;

  double m_basePreviousYWeight;
  double m_baseCurrentYWeight;

  uint32_t m_iterNum      = 0U;
  bool m_startedIterating = false;

  static constexpr double DEFAULT_ITER_ZERO_Y_VAL       = 0.9;
  double m_iterZeroYVal                                 = DEFAULT_ITER_ZERO_Y_VAL;
  static constexpr double DEFAULT_ITER_ZERO_LERP_FACTOR = 0.8;
  double m_iterZeroLerpFactor                           = DEFAULT_ITER_ZERO_LERP_FACTOR;

  std::vector<double> m_xVec{};
  std::vector<double> m_yVec{};
  std::vector<double> m_dampedXVec{};
  std::vector<double> m_dampedYVec{};
  std::vector<double> m_dampingCache{};
  static constexpr auto NUM_IGNORE_FIRST_VALS = 10U;
  XAndYVectors m_dampedVectors{std::ref(m_dampedXVec), std::ref(m_dampedYVec)};
  std::unique_ptr<UTILS::MATH::IDampingFunction> m_dampingFunc;
  [[nodiscard]] auto GetDampingFuncValue(double x) noexcept -> double;
  auto InitVectors() noexcept -> void;
  auto DoSomeInitialIterations() noexcept -> void;

  [[nodiscard]] auto GetFirstY() -> float;
  [[nodiscard]] auto GetNextY(size_t nodeNum) -> float;

  auto ValidateSettings() const -> void;

  auto UpdateDampedValues() -> void;
  [[nodiscard]] auto GetDampedValue(size_t nodeNum, double value) const -> double;

  using DampingFuncPtr = std::unique_ptr<UTILS::MATH::IDampingFunction>;
  [[nodiscard]] auto CreateDampingFunc() const noexcept -> DampingFuncPtr;
  [[nodiscard]] auto CreateExpDampingFunc() const noexcept -> DampingFuncPtr;
  [[nodiscard]] auto CreateLinearDampingFunc() const noexcept -> DampingFuncPtr;
};

inline auto Tentacle2D::SetIterZeroYVal(const double value) -> void
{
  m_iterZeroYVal = value;
}

inline auto Tentacle2D::SetIterZeroLerpFactor(const double value) -> void
{
  m_iterZeroLerpFactor = value;
}

inline auto Tentacle2D::SetBaseYWeights(const BaseYWeights& value) -> void
{
  m_basePreviousYWeight = static_cast<double>(value.previous);
  m_baseCurrentYWeight  = static_cast<double>(value.current);
}

inline auto Tentacle2D::GetNumNodes() const -> uint32_t
{
  return m_numRequestedNodes;
}

inline auto Tentacle2D::GetTentacleLength() const -> double
{
  return m_dimensions.xDimensions.max - m_dimensions.xDimensions.min;
}

inline auto Tentacle2D::GetXMin() const -> double
{
  return m_dimensions.xDimensions.min;
}

inline auto Tentacle2D::GetXMax() const -> double
{
  return m_dimensions.xDimensions.max;
}

inline auto Tentacle2D::GetDampedXAndYVectors() const -> const Tentacle2D::XAndYVectors&
{
  return m_dampedVectors;
}

} // namespace GOOM::VISUAL_FX::TENTACLES
