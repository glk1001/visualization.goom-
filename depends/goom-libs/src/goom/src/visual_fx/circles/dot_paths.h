#pragma once

#include "point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/parametric_functions2d.h"
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
           const UTILS::MATH::OscillatingFunction::Params& dotPathParams) noexcept;

  auto Reset() noexcept -> void;
  auto SetDotStartingPositions(std::vector<Point2dInt>&& dotStartingPositions) noexcept -> void;
  auto SetTarget(const Point2dInt& target) noexcept -> void;
  auto SetPathParams(const UTILS::MATH::OscillatingFunction::Params& params) noexcept -> void;

  [[nodiscard]] auto GetPositionTRef() const noexcept -> const UTILS::TValue&;

  [[nodiscard]] auto GetPositionTNumSteps() const noexcept -> uint32_t;
  auto SetPositionTNumSteps(uint32_t numSteps) noexcept -> void;

  [[nodiscard]] auto HasPositionTJustHitStartBoundary() const noexcept -> bool;
  [[nodiscard]] auto HasPositionTJustHitEndBoundary() const noexcept -> bool;
  [[nodiscard]] auto IsDelayed() const noexcept -> bool;

  [[nodiscard]] auto GetPositionT() const noexcept -> float;
  auto IncrementPositionT() noexcept -> void;

  [[nodiscard]] auto GetNextDotPositions() const noexcept -> std::vector<Point2dInt>;

private:
  const UTILS::MATH::IGoomRand& m_goomRand;
  const uint32_t m_numDots;
  std::vector<Point2dInt> m_dotStartingPositions;
  Point2dInt m_target;

  static constexpr uint32_t DEFAULT_POSITION_STEPS = 100;
  UTILS::MATH::OscillatingFunction::Params m_pathParams;

  bool m_randomizePoints = false;
  std::vector<UTILS::MATH::OscillatingPath> m_dotPaths{GetNewDotPaths(m_dotStartingPositions)};
  [[nodiscard]] auto GetNewDotPaths(const std::vector<Point2dInt>& dotStartingPositions) noexcept
      -> std::vector<UTILS::MATH::OscillatingPath>;
  [[nodiscard]] auto GetSmallRandomOffset() const noexcept -> Vec2dInt;
};

inline auto DotPaths::SetPathParams(const UTILS::MATH::OscillatingFunction::Params& params) noexcept
    -> void
{
  m_pathParams = params;
}

inline auto DotPaths::Reset() noexcept -> void
{
  std::for_each(
      begin(m_dotPaths), end(m_dotPaths), [](UTILS::MATH::IPath& path) { path.Reset(0.0F); });
}

inline auto DotPaths::SetPositionTNumSteps(const uint32_t numSteps) noexcept -> void
{
  std::for_each(begin(m_dotPaths),
                end(m_dotPaths),
                [&numSteps](UTILS::MATH::IPath& path) { path.SetNumSteps(numSteps); });
}

inline auto DotPaths::IncrementPositionT() noexcept -> void
{
  std::for_each(
      begin(m_dotPaths), end(m_dotPaths), [](UTILS::MATH::IPath& path) { path.IncrementT(); });
}

inline auto DotPaths::GetPositionTRef() const noexcept -> const UTILS::TValue&
{
  return m_dotPaths.at(0).GetPositionT();
}

inline auto DotPaths::GetPositionTNumSteps() const noexcept -> uint32_t
{
  return m_dotPaths.at(0).GetNumSteps();
}

inline auto DotPaths::HasPositionTJustHitStartBoundary() const noexcept -> bool
{
  return m_dotPaths.at(0).GetPositionT().HasJustHitStartBoundary();
}

inline auto DotPaths::HasPositionTJustHitEndBoundary() const noexcept -> bool
{
  return m_dotPaths.at(0).GetPositionT().HasJustHitEndBoundary();
}

inline auto DotPaths::IsDelayed() const noexcept -> bool
{
  return m_dotPaths.at(0).GetPositionT().IsDelayed();
}

inline auto DotPaths::GetPositionT() const noexcept -> float
{
  return m_dotPaths.at(0).GetCurrentT();
}

} // namespace GOOM::VISUAL_FX::CIRCLES
