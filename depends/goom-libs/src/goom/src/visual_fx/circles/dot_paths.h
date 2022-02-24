#pragma once

#include "goom_graphic.h"
#include "point2d.h"
#include "utils/math/paths.h"
#include "utils/t_values.h"

#include <cstdint>
#include <vector>

namespace GOOM::VISUAL_FX::CIRCLES
{

class DotPaths
{
public:
  DotPaths(uint32_t numDots,
           const Point2dInt& startTarget,
           float startRadius,
           const UTILS::MATH::PathParams& startPathParams) noexcept;

  void SetTarget(const Point2dInt& target);
  void SetPathParams(const UTILS::MATH::PathParams& params);

  [[nodiscard]] auto GetPositionTRef() const -> const UTILS::TValue&;

  [[nodiscard]] auto GetPositionTNumSteps() const -> uint32_t;
  void SetPositionTNumSteps(uint32_t numSteps);

  [[nodiscard]] auto HasPositionTJustHitStartBoundary() const -> bool;
  [[nodiscard]] auto HasPositionTJustHitEndBoundary() const -> bool;

  [[nodiscard]] auto GetPositionT() const -> float;
  void IncrementPositionT();

  [[nodiscard]] auto GetNextDotPositions() const -> std::vector<Point2dInt>;

private:
  const uint32_t m_numDots;
  const std::vector<Point2dInt> m_dotStartingPositions;
  [[nodiscard]] static auto GetDotStartingPositions(uint32_t numDots,
                                                    const Point2dInt& centre,
                                                    float radius) -> std::vector<Point2dInt>;
  Point2dInt m_target;

  static constexpr uint32_t DEFAULT_POSITION_STEPS = 100;
  static constexpr uint32_t DELAY_TIME_AT_CENTRE = 20;
  static constexpr uint32_t DELAY_TIME_AT_EDGE = 10;
  UTILS::TValue m_positionT{
      UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE,
      DEFAULT_POSITION_STEPS,
      {{0.0F, DELAY_TIME_AT_EDGE}, {1.0F, DELAY_TIME_AT_CENTRE}},
      0.0F
  };
  UTILS::MATH::PathParams m_pathParams;
  std::vector<UTILS::MATH::OscillatingPath> m_dotPaths;
  [[nodiscard]] auto GetNewDotPaths() -> std::vector<UTILS::MATH::OscillatingPath>;
};

inline void DotPaths::SetPathParams(const UTILS::MATH::PathParams& params)
{
  m_pathParams = params;
}

inline auto DotPaths::GetPositionTRef() const -> const UTILS::TValue&
{
  return m_positionT;
}

inline auto DotPaths::GetPositionTNumSteps() const -> uint32_t
{
  return m_positionT.GetNumSteps();
}

inline void DotPaths::SetPositionTNumSteps(const uint32_t numSteps)
{
  m_positionT.SetNumSteps(numSteps);
}

inline auto DotPaths::HasPositionTJustHitStartBoundary() const -> bool
{
  return m_positionT.HasJustHitStartBoundary();
}

inline auto DotPaths::HasPositionTJustHitEndBoundary() const -> bool
{
  return m_positionT.HasJustHitEndBoundary();
}

inline auto DotPaths::GetPositionT() const -> float
{
  return m_positionT();
}

inline void DotPaths::IncrementPositionT()
{
  m_positionT.Increment();
}

} // namespace GOOM::VISUAL_FX::CIRCLES
