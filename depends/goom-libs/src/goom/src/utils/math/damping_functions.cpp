#include "damping_functions.h"

#include "goom_config.h"
#include "misc.h"

#include <cmath>
#include <memory>
#include <tuple>
#include <vector>

namespace GOOM::UTILS::MATH
{

using UTILS::MATH::PI;

ExpDampingFunction::ExpDampingFunction(const ExpProperties& expProperties)
  : m_amplitude{expProperties.amplitude}
{
  static constexpr auto MIN_AMP = 0.00001;
  USED_FOR_DEBUGGING(MIN_AMP);
  Expects(std::fabs(expProperties.amplitude) >= MIN_AMP);
  Expects(expProperties.yAtStartToRise > m_amplitude);
  Expects(expProperties.yAtXMax > m_amplitude);

  const auto y0    = (expProperties.yAtStartToRise / m_amplitude) - 1.0;
  const auto y1    = (expProperties.yAtXMax / m_amplitude) - 1.0;
  const auto logY0 = std::log(y0);
  const auto logY1 = std::log(y1);
  m_b = ((expProperties.xToStartRise * logY1) - (expProperties.xMax * logY0)) / (logY1 - logY0);
  m_k = logY1 / (expProperties.xMax - m_b);
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

LinearDampingFunction::LinearDampingFunction(const LinearProperties& linearProperties) noexcept
  : m_m{(linearProperties.y1 - linearProperties.y0) / (linearProperties.x1 - linearProperties.x0)},
    m_x1{linearProperties.x1},
    m_y1{linearProperties.y1}
{
}

auto LinearDampingFunction::operator()(const double x) -> double
{
  return (m_m * (x - m_x1)) + m_y1;
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

SineWaveMultiplier::SineWaveMultiplier(const SineProperties& sineProperties) noexcept
  : m_frequency{sineProperties.frequency},
    m_lower{sineProperties.lower},
    m_upper{sineProperties.upper},
    m_x{sineProperties.x0}
{
}

auto SineWaveMultiplier::Increment() noexcept -> void
{
  m_x += m_piStepFrac * PI;
}

auto SineWaveMultiplier::GetNext() const noexcept -> float
{
  const auto val =
      static_cast<float>(m_rangeMapper(static_cast<double>(m_lower),
                                       static_cast<double>(m_upper),
                                       static_cast<double>(std::sin(m_frequency * m_x))));
  return val;
}

} // namespace GOOM::UTILS::MATH
