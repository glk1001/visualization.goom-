#include "tentacle2d.h"

#include "goom_config.h"
#include "utils/math/misc.h"

#include <memory>
#include <tuple>
#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

using UTILS::MATH::ExpDampingFunction;
using UTILS::MATH::FlatDampingFunction;
using UTILS::MATH::LinearDampingFunction;
using UTILS::MATH::PiecewiseDampingFunction;

static constexpr auto SMALL_WEIGHT = 0.001;

static constexpr auto DEFAULT_Y_DAMPING_FACTOR = 0.1;

static constexpr auto DAMPING_AMPLITUDE          = 0.1;
static constexpr auto DAMPING_Y_AT_START_TO_RISE = 5.0;
static constexpr auto DAMPING_Y_AT_X_MAX         = 30.0;
static constexpr auto LINEAR_DAMPING_FLAT_VALUE  = 0.1;
static constexpr auto LINEAR_DAMPING_Y_SCALE     = 30.0;

Tentacle2D::Tentacle2D(const uint32_t numNodes,
                       const Dimensions& dimensions,
                       const BaseYWeights& baseYWeights) noexcept
  : m_numRequestedNodes{numNodes},
    m_numActualNodes{numNodes + NUM_IGNORE_FIRST_VALS},
    m_dimensions{dimensions},
    m_basePreviousYWeight{static_cast<double>(baseYWeights.previous)},
    m_baseCurrentYWeight{static_cast<double>(baseYWeights.current)},
    m_dampingFunc{CreateDampingFunc(m_basePreviousYWeight, m_dimensions.xDimensions)}
{
}

// NOLINTNEXTLINE: readability-convert-member-functions-to-static
inline auto Tentacle2D::ValidateSettings() const -> void
{
  Expects(m_dimensions.xDimensions.min < m_dimensions.xDimensions.max);
  Expects(m_dimensions.yDimensions.min < m_dimensions.yDimensions.max);
  Expects(m_numRequestedNodes >= MIN_NUM_NODES);
  Expects(m_basePreviousYWeight > SMALL_WEIGHT);
  Expects(m_baseCurrentYWeight > SMALL_WEIGHT);
}

inline auto Tentacle2D::GetFirstY() -> float
{
  return static_cast<float>(STD20::lerp(m_yVec[0], m_iterZeroYVal, m_iterZeroLerpFactor));
}

inline auto Tentacle2D::GetNextY(const size_t nodeNum) -> float
{
  const auto prevY    = m_yVec[nodeNum - 1];
  const auto currentY = m_yVec[nodeNum];

  return static_cast<float>((m_basePreviousYWeight * prevY) + (m_baseCurrentYWeight * currentY));
}

inline auto Tentacle2D::GetDampedValue(const size_t nodeNum, const double value) const -> double
{
  return m_dampingCache[nodeNum] * value;
}

auto Tentacle2D::StartIterating() -> void
{
  ValidateSettings();

  InitVectors();

  m_iterNum = 0;

  DoSomeInitialIterations();
}

auto Tentacle2D::InitVectors() noexcept -> void
{
  m_xVec.resize(m_numActualNodes);
  m_yVec.resize(m_numActualNodes);
  m_dampingCache.resize(m_numActualNodes);
  m_dampedXVec.resize(m_numRequestedNodes);
  m_dampedYVec.resize(m_numRequestedNodes);

  const auto xStep = (m_dimensions.xDimensions.max - m_dimensions.xDimensions.min) /
                     static_cast<double>(m_numRequestedNodes - 1);
  auto x = m_dimensions.xDimensions.min;

  for (auto i = 0U; i < m_numActualNodes; ++i)
  {
    m_dampingCache[i] = GetDampingFuncValue(x);
    m_xVec[i]         = x;
    m_yVec[i]         = DEFAULT_Y_DAMPING_FACTOR * m_dampingCache[i];
    x += xStep;
  }

  for (auto i = 0U; i < m_numRequestedNodes; ++i)
  {
    m_dampedXVec[i] = m_xVec[i];
  }
}

inline auto Tentacle2D::GetDampingFuncValue(const double x) noexcept -> double
{
  return (*m_dampingFunc)(x);
}

inline auto Tentacle2D::DoSomeInitialIterations() noexcept -> void
{
  static constexpr auto NUM_START_ITERATIONS = 50U;
  for (auto i = 0U; i < NUM_START_ITERATIONS; ++i)
  {
    Iterate();
  }
}

auto Tentacle2D::Iterate() -> void
{
  ++m_iterNum;

  m_yVec[0] = static_cast<double>(GetFirstY());
  for (auto i = 1U; i < m_numActualNodes; ++i)
  {
    m_yVec[i] = static_cast<double>(GetNextY(i));
  }

  UpdateDampedValues();
}

auto Tentacle2D::UpdateDampedValues() -> void
{
  auto iActual = NUM_IGNORE_FIRST_VALS;
  for (auto i = 0U; i < m_numRequestedNodes; ++i)
  {
    m_dampedYVec[i] = GetDampedValue(iActual, m_yVec[iActual]);
    ++iActual;
  }
}

auto Tentacle2D::CreateDampingFunc(const double basePreviousYWeight,
                                   const LinearDimensions& xDimensions) noexcept
    -> Tentacle2D::DampingFuncPtr
{
  if (static constexpr auto LINEAR_CUTOFF_WEIGHT = 0.6; basePreviousYWeight < LINEAR_CUTOFF_WEIGHT)
  {
    return CreateLinearDampingFunc(xDimensions);
  }
  return CreateExpDampingFunc(xDimensions);
}

auto Tentacle2D::CreateExpDampingFunc(const LinearDimensions& xDimensions) noexcept
    -> Tentacle2D::DampingFuncPtr
{
  const auto xToStartRise = xDimensions.min + (0.25 * xDimensions.max);

  return DampingFuncPtr{std::make_unique<ExpDampingFunction>(
      ExpDampingFunction::ExpProperties{DAMPING_AMPLITUDE,
                                        xToStartRise,
                                        DAMPING_Y_AT_START_TO_RISE,
                                        xDimensions.max,
                                        DAMPING_Y_AT_X_MAX})};
}

auto Tentacle2D::CreateLinearDampingFunc(const LinearDimensions& xDimensions) noexcept
    -> Tentacle2D::DampingFuncPtr
{
  auto pieces = std::vector<std::tuple<double, double, DampingFuncPtr>>{};

  const auto flatXMin = xDimensions.min;
  const auto flatXMax = 0.1 * xDimensions.max;
  pieces.emplace_back(
      flatXMin,
      flatXMax,
      DampingFuncPtr{std::make_unique<FlatDampingFunction>(LINEAR_DAMPING_FLAT_VALUE)});

  const auto linearXMin = flatXMax;
  const auto linearXMax = 10.0 * xDimensions.max;
  pieces.emplace_back(
      linearXMin,
      linearXMax,
      DampingFuncPtr{
          std::make_unique<LinearDampingFunction>(LinearDampingFunction::LinearProperties{
              flatXMax, LINEAR_DAMPING_FLAT_VALUE, xDimensions.max, LINEAR_DAMPING_Y_SCALE})});

  return DampingFuncPtr{std::make_unique<PiecewiseDampingFunction>(pieces)};
}

} // namespace GOOM::VISUAL_FX::TENTACLES
