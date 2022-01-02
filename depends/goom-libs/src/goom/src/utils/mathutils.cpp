#include "mathutils.h"

#include <array>
#include <cmath>
#include <format>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace GOOM::UTILS
{

ExpIncreasingFunction::ExpIncreasingFunction(const double xMin,
                                             const double xMax,
                                             const double k) noexcept
  : IIncreasingFunction{xMax, xMin}, m_k{k}
{
}

auto ExpIncreasingFunction::operator()(const double x) -> double
{
  return 1.0 - std::exp(-m_k * std::fabs(x));
}

LogIncreasingFunction::LogIncreasingFunction(const double amp,
                                             const double xMin,
                                             const double xStart) noexcept
  : IIncreasingFunction(xMin, 10000000000), m_amplitude(amp), m_xMin(xMin), m_xStart(xStart)
{
}

double LogIncreasingFunction::operator()(const double x)
{
  return m_amplitude / std::log(x - m_xMin + m_xStart);
}


LogDampingFunction::LogDampingFunction(const double amp,
                                       const double xMin,
                                       const double xStart) noexcept
  : m_amplitude{amp}, m_xMin{xMin}, m_xStart{xStart}
{
}

double LogDampingFunction::operator()(const double x)
{
  return m_amplitude * std::log(x - m_xMin + m_xStart);
}

ExpDampingFunction::ExpDampingFunction(const double amp,
                                       const double xToStartRise,
                                       const double yAtStartToRise,
                                       const double xMax,
                                       const double yAtXMax)
  : m_amplitude{amp}
{
  constexpr double MIN_AMP = 0.00001;
  if (std::fabs(m_amplitude) < MIN_AMP)
  {
    throw std::runtime_error(
        std20::format("abs(amplitude) should be >= {}, not {}.", MIN_AMP, m_amplitude));
  }
  if (yAtStartToRise <= m_amplitude)
  {
    throw std::runtime_error(std20::format("yAtStartToRise should be > {} = amplitude, not {}.",
                                           m_amplitude, yAtStartToRise));
  }
  if (yAtXMax <= m_amplitude)
  {
    throw std::runtime_error(
        std20::format("yAtXMax should be > {} = amplitude, not {}.", m_amplitude, yAtXMax));
  }
  const double y0 = yAtStartToRise / m_amplitude - 1.0;
  const double y1 = yAtXMax / m_amplitude - 1.0;
  const double logY0 = std::log(y0);
  const double logY1 = std::log(y1);
  m_b = (xToStartRise * logY1 - xMax * logY0) / (logY1 - logY0);
  m_k = logY1 / (xMax - m_b);
}

auto ExpDampingFunction::operator()(const double x) -> double
{
  return m_amplitude * (1.0 + std::exp(m_k * (x - m_b)));
}

FlatDampingFunction::FlatDampingFunction(const double y) noexcept : m_y{y}
{
}

auto FlatDampingFunction::operator()([[maybe_unused]] const double x) -> double
{
  return m_y;
}

LinearDampingFunction::LinearDampingFunction(const double x0,
                                             const double y0,
                                             const double x1,
                                             const double y1) noexcept
  : m_m{(y1 - y0) / (x1 - x0)}, m_x1{x1}, m_y1{y1}
{
}

auto LinearDampingFunction::operator()(const double x) -> double
{
  return m_m * (x - m_x1) + m_y1;
}

PiecewiseDampingFunction::PiecewiseDampingFunction(
    std::vector<std::tuple<double, double, std::unique_ptr<IDampingFunction>>>& func) noexcept
  : m_pieces{std::move(func)}
{
}

auto PiecewiseDampingFunction::operator()(const double x) -> double
{
  for (const auto& [x0, x1, func] : m_pieces)
  {
    if ((x0 <= x) && (x < x1))
    {
      return (*func)(x);
    }
  }
  return 0.0;
}

SineWaveMultiplier::SineWaveMultiplier(const float freq,
                                       const float lwr,
                                       const float upr,
                                       const float x0) noexcept
  : m_rangeMapper{-1, +1},
    m_frequency{freq},
    m_lower{lwr},
    m_upper{upr},
    m_piStepFrac{1.0 / 16.0},
    m_x{x0}
{
}

auto SineWaveMultiplier::GetNext() -> float
{
  const auto val =
      static_cast<float>(m_rangeMapper(static_cast<double>(m_lower), static_cast<double>(m_upper),
                                       static_cast<double>(std::sin(m_frequency * m_x))));
  //logInfo("lower = {}, upper = {}, sin({}) = {}.", lower, upper, x, val);
  m_x += m_piStepFrac * m_pi;
  return val;
}

} // namespace GOOM::UTILS
