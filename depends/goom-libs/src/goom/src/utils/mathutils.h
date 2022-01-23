#pragma once

#include <cmath>
#include <cstdlib>
#include <memory>
#if __cplusplus > 201703L
#include <numbers>
#endif
#include <tuple>
#include <vector>


namespace stdnew
{
#if __cplusplus <= 201703L
template<typename _Fp>
[[nodiscard]] constexpr _Fp __lerp(_Fp __a, _Fp __b, _Fp __t) noexcept
{
  if ((__a <= 0 && __b >= 0) || (__a >= 0 && __b <= 0))
  {
    return __t * __b + (1 - __t) * __a;
  }

  if (1 == __t)
  {
    return __b; // exact
  }

  // Exact at __t=0, monotonic except near __t=1,
  // bounded, determinate, and consistent:
  const _Fp __x = __a + __t * (__b - __a);
  return (__t > 1) == (__b > __a) ? (__b < __x ? __x : __b)
                                  : (__b > __x ? __x : __b); // monotonic near __t=1
}

[[nodiscard]] constexpr float lerp(const float __a, const float __b, const float __t) noexcept
{
  return __lerp(__a, __b, __t);
}
[[nodiscard]] constexpr double lerp(const double __a, const double __b, const double __t) noexcept
{
  return __lerp(__a, __b, __t);
}
[[nodiscard]] inline auto lerp(const uint32_t a, const uint32_t b, const float t) noexcept
    -> uint32_t
{
  return static_cast<uint32_t>(
      std::round(stdnew::lerp(static_cast<float>(a), static_cast<float>(b), t)));
}
[[nodiscard]] inline auto lerp(const int32_t a, const int32_t b, const float t) noexcept -> int32_t
{
  return static_cast<int32_t>(
      std::round(stdnew::lerp(static_cast<float>(a), static_cast<float>(b), t)));
}
#else
constexpr float lerp(float __a, float __b, float __t) noexcept
{
  return std::lerp(__a, __b, __t);
}
constexpr double lerp(double __a, double __b, double __t) noexcept
{
  return std::lerp(__a, __b, __t);
}
#endif
} // namespace stdnew


namespace GOOM::UTILS
{

#if __cplusplus <= 201703L
constexpr float m_pi = 3.14159265358979323846264F;
#else
constexpr float m_pi = std::numbers::pi;
#endif
constexpr float m_two_pi = 2.0F * m_pi;
constexpr float m_half_pi = 0.5F * m_pi;
constexpr float m_third_pi = m_pi / 3.0F;

constexpr float DEGREES_90 = 90.0F;
constexpr float DEGREES_180 = 180.0F;
constexpr float DEGREES_360 = 360.0F;

template<typename T>
constexpr auto Sq(const T& x) -> T
{
  return x * x;
}

[[nodiscard]] constexpr auto SqDistance(const float x, const float y) -> float
{
  return Sq(x) + Sq(y);
}

constexpr float SMALL_FLOAT = 0.00001F;

inline bool floats_equal(const float x, const float y, const float epsilon = SMALL_FLOAT)
{
  return std::fabs(x - y) < epsilon;
}

template<typename T>
constexpr auto IsOdd(const T& n) -> bool
{
  return (n % 2) != 0;
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
  virtual ~IDampingFunction() noexcept = default;
  IDampingFunction(const IDampingFunction&) noexcept = delete;
  IDampingFunction(IDampingFunction&&) noexcept = delete;
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
  virtual ~ISequenceFunction() noexcept = default;
  ISequenceFunction(const ISequenceFunction&) noexcept = default;
  ISequenceFunction(ISequenceFunction&&) noexcept = delete;
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
  return stdnew::lerp(x0, x1, (x - m_xMin) / m_xWidth);
}

} // namespace GOOM::UTILS
