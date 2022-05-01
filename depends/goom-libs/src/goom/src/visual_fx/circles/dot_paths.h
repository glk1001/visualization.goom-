#pragma once

#include "point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/paths.h"
#include "utils/t_values.h"

#include <cstdint>
#include <vector>

namespace GOOM::VISUAL_FX::CIRCLES
{

class DotPaths
{
public:
  DotPaths(const UTILS::MATH::IGoomRand& goomRand,
           uint32_t numDots,
           std::vector<Point2dInt>&& dotStartingPositions,
           const Point2dInt& dotTarget,
           const UTILS::MATH::OscillatingPath::Params& dotPathParams) noexcept;

  auto Reset() -> void;
  auto SetTarget(const Point2dInt& target) -> void;
  auto SetPathParams(const UTILS::MATH::OscillatingPath::Params& params) -> void;

  [[nodiscard]] auto GetPositionTRef() const -> const UTILS::TValue&;

  [[nodiscard]] auto GetPositionTNumSteps() const -> uint32_t;
  auto SetPositionTNumSteps(uint32_t numSteps) -> void;

  [[nodiscard]] auto HasPositionTJustHitStartBoundary() const -> bool;
  [[nodiscard]] auto HasPositionTJustHitEndBoundary() const -> bool;

  [[nodiscard]] auto GetPositionT() const -> float;
  auto IncrementPositionT() -> void;

  [[nodiscard]] auto GetNextDotPositions() const -> std::vector<Point2dInt>;

private:
  const UTILS::MATH::IGoomRand& m_goomRand;
  const uint32_t m_numDots;
  const std::vector<Point2dInt> m_dotStartingPositions;
  Point2dInt m_target;

  static constexpr uint32_t DEFAULT_POSITION_STEPS = 100;
  static constexpr uint32_t DELAY_TIME_AT_CENTRE = 20;
  static constexpr uint32_t DELAY_TIME_AT_EDGE = 10;
  UTILS::MATH::OscillatingPath::Params m_pathParams;

  bool m_randomizePoints = false;
  std::vector<UTILS::MATH::OscillatingPath> m_dotPaths;
  [[nodiscard]] auto GetNewDotPaths(const std::vector<Point2dInt>& dotStartingPositions)
      -> std::vector<UTILS::MATH::OscillatingPath>;
  [[nodiscard]] auto GetSmallRandomOffset() const -> Vec2dInt;
};

inline auto DotPaths::SetPathParams(const UTILS::MATH::OscillatingPath::Params& params) -> void
{
  m_pathParams = params;
}

inline auto DotPaths::Reset() -> void
{
  std::for_each(begin(m_dotPaths), end(m_dotPaths),
                [](UTILS::MATH::IPath& path) { path.Reset(0.0F); });
}

inline auto DotPaths::SetPositionTNumSteps(const uint32_t numSteps) -> void
{
  std::for_each(begin(m_dotPaths), end(m_dotPaths),
                [&numSteps](UTILS::MATH::IPath& path) { path.SetNumSteps(numSteps); });
}

inline auto DotPaths::IncrementPositionT() -> void
{
  std::for_each(begin(m_dotPaths), end(m_dotPaths),
                [](UTILS::MATH::IPath& path) { path.IncrementT(); });
}

inline auto DotPaths::GetPositionTRef() const -> const UTILS::TValue&
{
  return m_dotPaths.at(0).GetPositionT();
}

inline auto DotPaths::GetPositionTNumSteps() const -> uint32_t
{
  return m_dotPaths.at(0).GetNumSteps();
}

inline auto DotPaths::HasPositionTJustHitStartBoundary() const -> bool
{
  return m_dotPaths.at(0).GetPositionT().HasJustHitStartBoundary();
}

inline auto DotPaths::HasPositionTJustHitEndBoundary() const -> bool
{
  return m_dotPaths.at(0).GetPositionT().HasJustHitEndBoundary();
}

inline auto DotPaths::GetPositionT() const -> float
{
  return m_dotPaths.at(0).GetCurrentT();
}

} // namespace GOOM::VISUAL_FX::CIRCLES
