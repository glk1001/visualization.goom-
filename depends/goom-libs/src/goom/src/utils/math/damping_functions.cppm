module;

#include <cmath>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

export module Goom.Utils.Math.DampingFunctions;

import Goom.Utils.Math.Misc;
import Goom.Lib.AssertUtils;

export namespace GOOM::UTILS::MATH
{

class IDampingFunction
{
public:
  IDampingFunction() noexcept                                  = default;
  IDampingFunction(const IDampingFunction&) noexcept           = delete;
  IDampingFunction(IDampingFunction&&) noexcept                = delete;
  virtual ~IDampingFunction() noexcept                         = default;
  auto operator=(const IDampingFunction&) -> IDampingFunction& = delete;
  auto operator=(IDampingFunction&&) -> IDampingFunction&      = delete;

  virtual auto operator()(double x) -> double = 0;
};

// Asymptotes to 'amplitude' in negative x direction,
//   and the rate depends on the input parameters.
class ExpDampingFunction : public IDampingFunction
{
public:
  struct ExpProperties
  {
    double amplitude;
    double xToStartRise;
    double yAtStartToRise;
    double xMax;
    double yAtXMax;
  };

  ExpDampingFunction() noexcept = default;
  explicit ExpDampingFunction(const ExpProperties& expProperties);
  auto operator()(double x) -> double override;
  [[nodiscard]] auto KVal() const -> double { return m_k; }
  [[nodiscard]] auto BVal() const -> double { return m_b; }

private:
  double m_amplitude = 1;
  double m_k         = 1;
  double m_b         = 1;
};

class FlatDampingFunction : public IDampingFunction
{
public:
  FlatDampingFunction() noexcept = default;
  explicit FlatDampingFunction(double y) noexcept;
  auto operator()(double x) -> double override;

private:
  double m_y = 0.0;
};

class LinearDampingFunction : public IDampingFunction
{
public:
  struct LinearProperties
  {
    double x0;
    double y0;
    double x1;
    double y1;
  };

  LinearDampingFunction() noexcept = default;
  explicit LinearDampingFunction(const LinearProperties& linearProperties) noexcept;
  auto operator()(double x) -> double override;

private:
  double m_m  = 1.0;
  double m_x1 = 0.0;
  double m_y1 = 1.0;
};

class PiecewiseDampingFunction : public IDampingFunction
{
public:
  PiecewiseDampingFunction() noexcept = default;
  explicit PiecewiseDampingFunction(
      std::vector<std::tuple<double, double, std::unique_ptr<IDampingFunction>>>& func) noexcept;
  auto operator()(double x) -> double override;

private:
  std::vector<std::tuple<double, double, std::unique_ptr<IDampingFunction>>> m_pieces;
};

class ISequenceFunction
{
public:
  ISequenceFunction() noexcept                                            = default;
  ISequenceFunction(const ISequenceFunction&) noexcept                    = default;
  ISequenceFunction(ISequenceFunction&&) noexcept                         = delete;
  virtual ~ISequenceFunction() noexcept                                   = default;
  auto operator=(const ISequenceFunction&) noexcept -> ISequenceFunction& = default;
  auto operator=(ISequenceFunction&&) noexcept -> ISequenceFunction&      = delete;

  virtual auto Increment() noexcept -> void                    = 0;
  [[nodiscard]] virtual auto GetNext() const noexcept -> float = 0;
};

class SineWaveMultiplier : public ISequenceFunction
{
public:
  struct SineProperties
  {
    float frequency;
    float lower;
    float upper;
    float x0;
  };

  explicit SineWaveMultiplier(const SineProperties& sineProperties) noexcept;

  auto Increment() noexcept -> void override;
  [[nodiscard]] auto GetNext() const noexcept -> float override;

  auto SetX0(float x0) noexcept -> void;
  [[nodiscard]] auto GetFrequency() const noexcept -> float;
  auto SetFrequency(float val) noexcept -> void;

  [[nodiscard]] auto GetLower() const noexcept -> float;
  [[nodiscard]] auto GetUpper() const noexcept -> float;
  struct SineRange
  {
    float lower;
    float upper;
  };
  auto SetRange(const SineRange& sineRange) noexcept -> void;

  auto SetPiStepFrac(float val) noexcept -> void;

private:
  RangeMapper m_rangeMapper{-1.0, +1.0};
  float m_frequency;
  float m_lower;
  float m_upper;
  float m_x;
  static constexpr auto DEFAULT_PI_STEP_FRAC = 1.0 / 16.0;
  float m_piStepFrac                         = DEFAULT_PI_STEP_FRAC;
};

} // namespace GOOM::UTILS::MATH

namespace GOOM::UTILS::MATH
{

inline auto SineWaveMultiplier::SetX0(const float x0) noexcept -> void
{
  m_x = x0;
}

inline auto SineWaveMultiplier::GetFrequency() const noexcept -> float
{
  return m_frequency;
}

inline auto SineWaveMultiplier::SetFrequency(const float val) noexcept -> void
{
  m_frequency = val;
}

inline auto SineWaveMultiplier::GetLower() const noexcept -> float
{
  return m_lower;
}

inline auto SineWaveMultiplier::GetUpper() const noexcept -> float
{
  return m_upper;
}

inline auto SineWaveMultiplier::SetRange(const SineRange& sineRange) noexcept -> void
{
  m_lower = sineRange.lower;
  m_upper = sineRange.upper;
}

inline auto SineWaveMultiplier::SetPiStepFrac(const float val) noexcept -> void
{
  m_piStepFrac = val;
}

} // namespace GOOM::UTILS::MATH

module :private;

namespace GOOM::UTILS::MATH
{

using UTILS::MATH::PI;

ExpDampingFunction::ExpDampingFunction(const ExpProperties& expProperties)
  : m_amplitude{expProperties.amplitude}
{
  static constexpr auto MIN_AMP = 0.00001;
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
