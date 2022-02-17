#pragma once

#include "math20.h"

#include <cmath>
#include <memory>
#include <tuple>
#include <vector>

namespace GOOM::UTILS::MATH
{

constexpr float two_pi = 2.0F * STD20::pi;
constexpr float half_pi = 0.5F * STD20::pi;
constexpr float third_pi = STD20::pi / 3.0F;

constexpr float DEGREES_90 = 90.0F;
constexpr float DEGREES_180 = 180.0F;
constexpr float DEGREES_360 = 360.0F;

template<typename T>
[[nodiscard]] constexpr auto Sq(const T& x) -> T
{
  return x * x;
}

[[nodiscard]] constexpr auto SqDistance(const float x, const float y) -> float
{
  return Sq(x) + Sq(y);
}

constexpr float SMALL_FLOAT = 0.00001F;

[[nodiscard]] inline bool floats_equal(const float x,
                                       const float y,
                                       const float epsilon = SMALL_FLOAT)
{
  return std::fabs(x - y) < epsilon;
}

template<typename T>
[[nodiscard]] constexpr auto IsEven(const T& n) -> bool
{
  return 0 == (n % 2);
}

template<typename T>
[[nodiscard]] constexpr auto IsOdd(const T& n) -> bool
{
  return 0 != (n % 2);
}

template<typename T>
[[nodiscard]] constexpr auto GetHalf(const T& n) -> T
{
  return n / 2;
}

template<typename T>
[[nodiscard]] constexpr auto GetQuarter(const T& n) -> T
{
  return n / 4;
}

template<typename T>
[[nodiscard]] constexpr auto GetThreeQuarters(const T& n) -> T
{
  return (3 * n) / 4;
}

class RangeMapper
{
public:
  RangeMapper() noexcept = default;
  explicit RangeMapper(double x0, double x1) noexcept;
  auto operator()(double x0, double x1, double x) const -> double;
  [[nodiscard]] auto GetXMin() const -> double { return m_xMin; }
  [[nodiscard]] auto GetXMax() const -> double { return m_xMax; }

private:
  double m_xMin = 0;
  double m_xMax = 0;
  double m_xWidth = 0;
};

class IDampingFunction
{
public:
  IDampingFunction() noexcept = default;
  IDampingFunction(const IDampingFunction&) noexcept = delete;
  IDampingFunction(IDampingFunction&&) noexcept = delete;
  virtual ~IDampingFunction() noexcept = default;
  auto operator=(const IDampingFunction&) -> IDampingFunction& = delete;
  auto operator=(IDampingFunction&&) -> IDampingFunction& = delete;

  virtual auto operator()(double x) -> double = 0;
};

// Asymptotes to 'amplitude' in negative x direction,
//   and the rate depends on the input parameters.
class ExpDampingFunction : public IDampingFunction
{
public:
  ExpDampingFunction() noexcept = default;
  ExpDampingFunction(
      double amplitude, double xToStartRise, double yAtStartToRise, double xMax, double yAtXMax);
  auto operator()(double x) -> double override;
  [[nodiscard]] auto KVal() const -> double { return m_k; }
  [[nodiscard]] auto BVal() const -> double { return m_b; }

private:
  const double m_amplitude = 1;
  double m_k = 1;
  double m_b = 1;
};

class FlatDampingFunction : public IDampingFunction
{
public:
  FlatDampingFunction() noexcept = default;
  explicit FlatDampingFunction(double y) noexcept;
  auto operator()(double x) -> double override;

private:
  const double m_y = 0.0;
};

class LinearDampingFunction : public IDampingFunction
{
public:
  LinearDampingFunction() noexcept = default;
  explicit LinearDampingFunction(double x0, double y0, double x1, double y1) noexcept;
  auto operator()(double x) -> double override;

private:
  const double m_m = 1;
  const double m_x1 = 0;
  const double m_y1 = 1;
};

class PiecewiseDampingFunction : public IDampingFunction
{
public:
  PiecewiseDampingFunction() noexcept = default;
  explicit PiecewiseDampingFunction(
      std::vector<std::tuple<double, double, std::unique_ptr<IDampingFunction>>>& func) noexcept;
  auto operator()(double x) -> double override;

private:
  const std::vector<std::tuple<double, double, std::unique_ptr<IDampingFunction>>> m_pieces{};
};

class ISequenceFunction
{
public:
  ISequenceFunction() noexcept = default;
  ISequenceFunction(const ISequenceFunction&) noexcept = default;
  ISequenceFunction(ISequenceFunction&&) noexcept = delete;
  virtual ~ISequenceFunction() noexcept = default;
  auto operator=(const ISequenceFunction&) noexcept -> ISequenceFunction& = default;
  auto operator=(ISequenceFunction&&) noexcept -> ISequenceFunction& = delete;

  virtual auto GetNext() -> float = 0;
};

class SineWaveMultiplier : public ISequenceFunction
{
public:
  SineWaveMultiplier(float frequency, float lower, float upper, float x0) noexcept;

  auto GetNext() -> float override;

  void SetX0(const float x0) { m_x = x0; }
  [[nodiscard]] auto GetFrequency() const -> float { return m_frequency; }
  void SetFrequency(const float val) { m_frequency = val; }

  [[nodiscard]] auto GetLower() const -> float { return m_lower; }
  [[nodiscard]] auto GetUpper() const -> float { return m_upper; }
  void SetRange(const float lwr, const float upr)
  {
    m_lower = lwr;
    m_upper = upr;
  }

  void SetPiStepFrac(const float val) { m_piStepFrac = val; }

private:
  RangeMapper m_rangeMapper;
  float m_frequency;
  float m_lower;
  float m_upper;
  float m_piStepFrac;
  float m_x;
};

inline RangeMapper::RangeMapper(const double x0, const double x1) noexcept
  : m_xMin(x0), m_xMax(x1), m_xWidth(m_xMax - m_xMin)
{
}

inline auto RangeMapper::operator()(const double x0, const double x1, const double x) const
    -> double
{
  return STD20::lerp(x0, x1, (x - m_xMin) / m_xWidth);
}

} // namespace GOOM::UTILS::MATH
