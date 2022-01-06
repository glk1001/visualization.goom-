#include "tentacle2d.h"

#include "utils/mathutils.h"

#include <format>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

using UTILS::ExpDampingFunction;
using UTILS::FlatDampingFunction;
using UTILS::LinearDampingFunction;
using UTILS::PiecewiseDampingFunction;

Tentacle2D::Tentacle2D([[maybe_unused]] const size_t id,
                       const size_t numNodes,
                       const double xMin,
                       const double xMax,
                       const double yMin,
                       const double yMax,
                       const double basePrevYWeight,
                       const double baseCurrentYWeight) noexcept
  : m_numNodes{numNodes},
    m_xMin{xMin},
    m_xMax{xMax},
    m_yMin{yMin},
    m_yMax{yMax},
    m_basePrevYWeight{basePrevYWeight},
    m_baseCurrentYWeight{baseCurrentYWeight},
    m_dampingFunc{CreateDampingFunc(m_basePrevYWeight, m_xMin, m_xMax)}
{
}

void Tentacle2D::SetXDimensions(const double x0, const double x1)
{
  if (m_startedIterating)
  {
    throw std::runtime_error("Can't set x dimensions after iteration start.");
  }

  m_xMin = x0;
  m_xMax = x1;
  ValidateXDimensions();

  m_dampingFunc = CreateDampingFunc(m_basePrevYWeight, m_xMin, m_xMax);
}

void Tentacle2D::ValidateSettings() const
{
  ValidateXDimensions();
  ValidateYDimensions();
  ValidateNumNodes();
  ValidatePrevYWeight();
  ValidateCurrentYWeight();
}

void Tentacle2D::ValidateXDimensions() const
{
  if (m_xMax <= m_xMin)
  {
    throw std::logic_error(std20::format("xMax must be > xMin, not ({}, {}).", m_xMin, m_xMax));
  }
}

void Tentacle2D::ValidateYDimensions() const
{
  if (m_yMax <= m_yMin)
  {
    throw std::logic_error(std20::format("yMax must be > yMin, not ({}, {}).", m_yMin, m_yMax));
  }
}

void Tentacle2D::ValidateNumNodes() const
{
  if (m_numNodes < MIN_NUM_NODES)
  {
    throw std::runtime_error(
        std20::format("numNodes must be >= {}, not {}.", MIN_NUM_NODES, m_numNodes));
  }
}

constexpr double SMALL_WEIGHT = 0.001;

void Tentacle2D::ValidatePrevYWeight() const
{
  if (m_basePrevYWeight < SMALL_WEIGHT)
  {
    throw std::runtime_error(
        std20::format("prevYWeight must be >= {}, not {}.", SMALL_WEIGHT, m_basePrevYWeight));
  }
}

void Tentacle2D::ValidateCurrentYWeight() const
{
  if (m_baseCurrentYWeight < SMALL_WEIGHT)
  {
    throw std::runtime_error(
        std20::format("currentYWeight must be >= {}, not {}.", SMALL_WEIGHT, m_baseCurrentYWeight));
  }
}

inline auto Tentacle2D::Damp(const size_t nodeNum) const -> double
{
  return m_dampingCache[nodeNum];
}

void Tentacle2D::StartIterating()
{
  ValidateSettings();

  m_startedIterating = true;
  m_iterNum = 0;

  const double xStep = (m_xMax - m_xMin) / static_cast<double>(m_numNodes - 1);

  m_xVec.resize(m_numNodes);
  m_yVec.resize(m_numNodes);
  m_dampedYVec.resize(m_numNodes);
  m_dampingCache.resize(m_numNodes);
  double x = m_xMin;
  for (size_t i = 0; i < m_numNodes; ++i)
  {
    m_dampingCache[i] = GetDamping(x);
    m_xVec[i] = x;
    constexpr double DEFAULT_Y_DAMPING_FACTOR = 0.1;
    m_yVec[i] = DEFAULT_Y_DAMPING_FACTOR * m_dampingCache[i];

    x += xStep;
  }
}

void Tentacle2D::Iterate()
{
  ++m_iterNum;

  m_yVec[0] = static_cast<double>(GetFirstY());
  for (size_t i = 1; i < m_numNodes; ++i)
  {
    m_yVec[i] = static_cast<double>(GetNextY(i));
  }

  UpdateDampedValues(m_yVec);
}

void Tentacle2D::UpdateDampedValues(const std::vector<double>& yValues)
{
  constexpr size_t NUM_SMOOTH_NODES = std::min(size_t(10), MIN_NUM_NODES);
  const auto tSmooth = [](const double t)
  {
    constexpr double PARABOLA_COEFF = 2.0;
    return t * (PARABOLA_COEFF - t);
  };

  const double tStep = 1.0 / (NUM_SMOOTH_NODES - 1);
  double tNext = tStep;
  m_dampedYVec[0] = 0.0;
  for (size_t i = 1; i < NUM_SMOOTH_NODES; ++i)
  {
    const double t = tSmooth(tNext);
    m_dampedYVec[i] = stdnew::lerp(m_dampedYVec[i - 1], GetDampedVal(i, yValues[i]), t);
    tNext += tStep;
  }

  for (size_t i = NUM_SMOOTH_NODES; i < m_numNodes; ++i)
  {
    m_dampedYVec[i] = GetDampedVal(i, yValues[i]);
  }
}

inline auto Tentacle2D::GetFirstY() -> float
{
  return static_cast<float>(stdnew::lerp(m_yVec[0], m_iterZeroYVal, m_iterZeroLerpFactor));
}

inline auto Tentacle2D::GetNextY(const size_t nodeNum) -> float
{
  const double prevY = m_yVec[nodeNum - 1];
  const double currentY = m_yVec[nodeNum];

  return static_cast<float>((m_basePrevYWeight * prevY) + (m_baseCurrentYWeight * currentY));
}

inline auto Tentacle2D::GetDampedVal(const size_t nodeNum, const double val) const -> double
{
  if (!m_doDamping)
  {
    return val;
  }
  return Damp(nodeNum) * val;
}

auto Tentacle2D::GetDampedXAndYVectors() const -> const Tentacle2D::XAndYVectors&
{
  constexpr size_t MIN_VEC_SIZE = 2;

  if (m_xVec.size() < MIN_VEC_SIZE)
  {
    throw std::runtime_error(std20::format(
        "GetDampedXAndYVectors: xVec.size() must be >= {}, not {}.", MIN_VEC_SIZE, m_xVec.size()));
  }
  if (m_dampedYVec.size() < MIN_VEC_SIZE)
  {
    throw std::runtime_error(
        std20::format("GetDampedXAndYVectors: yVec.size() must be >= {}, not {}.", MIN_VEC_SIZE,
                      m_dampedYVec.size()));
  }
  if (std::get<0>(m_xAndYVectors).size() < MIN_VEC_SIZE)
  {
    throw std::runtime_error(std20::format(
        "GetDampedXAndYVectors: std::get<0>(m_xAndYVectors).size() must be >= {}, not {}.",
        MIN_VEC_SIZE, std::get<0>(m_xAndYVectors).size()));
  }

  return m_dampedVectors;
}

auto Tentacle2D::CreateDampingFunc(const double prevYWeight, const double xMin, const double xMax)
    -> Tentacle2D::DampingFuncPtr
{
  constexpr double LINEAR_CUTOFF_WEIGHT = 0.6;
  if (prevYWeight < LINEAR_CUTOFF_WEIGHT)
  {
    return CreateLinearDampingFunc(xMin, xMax);
  }
  return CreateExpDampingFunc(xMin, xMax);
}

auto Tentacle2D::CreateExpDampingFunc(const double xMin, const double xMax)
    -> Tentacle2D::DampingFuncPtr
{
  const double xRiseStart = xMin + (0.25 * xMax);
  constexpr double AMPLITUDE = 0.1;
  constexpr double DAMP_START = 5.0;
  constexpr double DAMP_MAX = 30.0;

  return DampingFuncPtr{
      std::make_unique<ExpDampingFunction>(AMPLITUDE, xRiseStart, DAMP_START, xMax, DAMP_MAX)};
}

auto Tentacle2D::CreateLinearDampingFunc(const double xMin, const double xMax)
    -> Tentacle2D::DampingFuncPtr
{
  std::vector<std::tuple<double, double, DampingFuncPtr>> pieces{};

  constexpr double FLAT_DAMPING_VALUE = 0.1;
  const double flatXMin = xMin;
  const double flatXMax = 0.1 * xMax;
  pieces.emplace_back(flatXMin, flatXMax,
                      DampingFuncPtr{std::make_unique<FlatDampingFunction>(FLAT_DAMPING_VALUE)});

  constexpr float Y_SCALE = 30.0;
  const double linearXMin = flatXMax;
  const double linearXMax = 10.0 * xMax;
  pieces.emplace_back(linearXMin, linearXMax,
                      DampingFuncPtr{std::make_unique<LinearDampingFunction>(
                          flatXMax, FLAT_DAMPING_VALUE, xMax, Y_SCALE)});

  return DampingFuncPtr{std::make_unique<PiecewiseDampingFunction>(pieces)};
}

} // namespace GOOM::VISUAL_FX::TENTACLES
