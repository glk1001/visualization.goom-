#pragma once

#include "utils/math/damping_functions.h"

#include <memory>
#include <tuple>
#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

class Tentacle2D
{
  using XAndYVectors = std::tuple<std::vector<double>&, std::vector<double>&>;

public:
  static constexpr uint32_t MIN_NUM_NODES = 10U;

  Tentacle2D(uint32_t id,
             uint32_t numNodes,
             double xMin,
             double xMax,
             double yMin,
             double yMax,
             double basePrevYWeight,
             double baseCurrentYWeight) noexcept;

  void StartIterating();

  void Iterate();

  [[nodiscard]] auto GetDampedXAndYVectors() const -> const XAndYVectors&;

  [[nodiscard]] auto GetLength() const -> double;
  [[nodiscard]] auto GetXMin() const -> double;
  [[nodiscard]] auto GetXMax() const -> double;
  void SetXDimensions(double x0, double x1);

  [[nodiscard]] auto GetNumNodes() const -> uint32_t;

  void SetIterZeroYVal(double val);
  void SetIterZeroLerpFactor(double val);
  void SetDoDamping(bool val);

private:
  const uint32_t m_numNodes;
  double m_xMin                                         = 0.0;
  double m_xMax                                         = 0.0;
  double m_yMin                                         = 0.0;
  double m_yMax                                         = 0.0;
  static constexpr double DEFAULT_BASE_PREV_Y_WEIGHT    = 0.770;
  double m_basePrevYWeight                              = DEFAULT_BASE_PREV_Y_WEIGHT;
  static constexpr double DEFAULT_BASE_CURRENT_Y_WEIGHT = 0.230;
  double m_baseCurrentYWeight                           = DEFAULT_BASE_CURRENT_Y_WEIGHT;
  static constexpr double DEFAULT_ITER_ZERO_Y_VAL       = 0.9;
  double m_iterZeroYVal                                 = DEFAULT_ITER_ZERO_Y_VAL;
  static constexpr double DEFAULT_ITER_ZERO_LERP_FACTOR = 0.8;
  double m_iterZeroLerpFactor                           = DEFAULT_ITER_ZERO_LERP_FACTOR;
  uint32_t m_iterNum                                    = 0;
  bool m_startedIterating                               = false;
  std::vector<double> m_xVec{};
  std::vector<double> m_yVec{};
  XAndYVectors m_xAndYVectors{std::make_tuple(std::ref(m_xVec), std::ref(m_yVec))};
  std::vector<double> m_dampedYVec{};
  std::vector<double> m_dampingCache{};
  XAndYVectors m_dampedVectors{std::make_tuple(std::ref(m_xVec), std::ref(m_dampedYVec))};
  std::unique_ptr<UTILS::MATH::IDampingFunction> m_dampingFunc;
  bool m_doDamping = true;

  [[nodiscard]] auto GetFirstY() -> float;
  [[nodiscard]] auto GetNextY(size_t nodeNum) -> float;
  [[nodiscard]] auto GetDamping(double x) const -> double;
  [[nodiscard]] auto GetDampedVal(size_t nodeNum, double val) const -> double;
  void UpdateDampedValues(const std::vector<double>& yValues);
  [[nodiscard]] auto Damp(size_t nodeNum) const -> double;
  void ValidateSettings() const;
  void ValidateXDimensions() const;
  void ValidateYDimensions() const;
  void ValidateNumNodes() const;
  void ValidatePrevYWeight() const;
  void ValidateCurrentYWeight() const;

  using DampingFuncPtr = std::unique_ptr<UTILS::MATH::IDampingFunction>;
  [[nodiscard]] static auto CreateDampingFunc(double prevYWeight, double xMin, double xMax)
      -> DampingFuncPtr;
  [[nodiscard]] static auto CreateExpDampingFunc(double xMin, double xMax) -> DampingFuncPtr;
  [[nodiscard]] static auto CreateLinearDampingFunc(double xMin, double xMax) -> DampingFuncPtr;
};

inline void Tentacle2D::SetIterZeroYVal(const double val)
{
  m_iterZeroYVal = val;
}

inline void Tentacle2D::SetIterZeroLerpFactor(const double val)
{
  m_iterZeroLerpFactor = val;
}

inline auto Tentacle2D::GetNumNodes() const -> uint32_t
{
  return m_numNodes;
}

inline auto Tentacle2D::GetLength() const -> double
{
  return m_xMax - m_xMin;
}

inline auto Tentacle2D::GetXMin() const -> double
{
  return m_xMin;
}
inline auto Tentacle2D::GetXMax() const -> double
{
  return m_xMax;
}

inline void Tentacle2D::SetDoDamping(const bool val)
{
  m_doDamping = val;
}

inline auto Tentacle2D::GetDamping(const double x) const -> double
{
  return (*m_dampingFunc)(x);
}

} // namespace GOOM::VISUAL_FX::TENTACLES
