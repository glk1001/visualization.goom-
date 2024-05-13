module;

#include "goom/goom_types.h"
#include "goom/point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/math/parametric_functions2d.h"
#include "utils/math/paths.h"
#include "utils/t_values.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

module Goom.VisualFx.CirclesFx:DotPaths;

namespace GOOM::VISUAL_FX::CIRCLES
{

class DotPaths
{
public:
  struct DotStartsToAndFrom
  {
    std::vector<Point2dInt> dotStartingPositionsFromTarget;
    std::vector<Point2dInt> dotStartingPositionsToTarget;
  };
  struct DotTargetsToAndFrom
  {
    Point2dInt dotTargetPositionToTarget;
    Point2dInt dotTargetPositionFromTarget;
  };
  struct DotPathParamsToAndFrom
  {
    UTILS::MATH::OscillatingFunction::Params dotPathParamsToTarget;
    UTILS::MATH::OscillatingFunction::Params dotPathParamsFromTarget;
  };

  DotPaths(const UTILS::MATH::IGoomRand& goomRand,
           uint32_t numDots,
           DotStartsToAndFrom&& dotStartsToAndFrom,
           const DotTargetsToAndFrom& dotTargetsToAndFrom,
           const DotPathParamsToAndFrom& dotPathParamsToAndFrom) noexcept;

  [[nodiscard]] auto GetNumDots() const noexcept -> uint32_t;
  enum class Direction : UnderlyingEnumType
  {
    TO_TARGET,
    FROM_TARGET,
  };
  [[nodiscard]] auto GetCurrentDirection() const noexcept -> Direction;
  auto ChangeDirectionWithToTargetFixup(Direction newDirection) noexcept -> void;
  auto ChangeDirection(Direction newDirection) noexcept -> void;
  auto SetPathParams(const DotPathParamsToAndFrom& dotPathParamsToAndFrom) noexcept -> void;

  [[nodiscard]] auto GetPositionTRef() const noexcept -> const UTILS::TValue&;
  [[nodiscard]] auto HasUpdatedDotPathsToAndFrom() const noexcept -> bool;
  auto ResetUpdatedDotPathsToAndFromFlag() noexcept -> void;

  [[nodiscard]] auto GetPositionTNumSteps() const noexcept -> uint32_t;
  auto SetPositionTNumSteps(uint32_t numSteps) noexcept -> void;

  [[nodiscard]] auto HasPositionTJustHitStartBoundary() const noexcept -> bool;
  [[nodiscard]] auto HasPositionTJustHitEndBoundary() const noexcept -> bool;
  [[nodiscard]] auto IsCloseToStartBoundary(float distance) const noexcept -> bool;
  [[nodiscard]] auto IsCloseToEndBoundary(float distance) const noexcept -> bool;

  [[nodiscard]] auto GetPositionT() const noexcept -> float;
  auto IncrementPositionT() noexcept -> void;

  [[nodiscard]] auto GetNextDotPositions() const noexcept -> std::vector<Point2dInt>;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  uint32_t m_numDots;
  DotStartsToAndFrom m_dotStartsToAndFrom;
  DotTargetsToAndFrom m_dotTargetsToAndFrom;

  static constexpr uint32_t DEFAULT_POSITION_STEPS = 100;
  DotPathParamsToAndFrom m_dotPathParamsToAndFrom;

  bool m_randomizePoints = false;
  struct DotPathsToAndFrom
  {
    std::vector<UTILS::MATH::OscillatingPath> dotPathToTarget;
    std::vector<UTILS::MATH::OscillatingPath> dotPathFromTarget;
  };
  DotPathsToAndFrom m_dotPathsToAndFrom;
  bool m_updatedDotPathsToAndFrom = false;
  [[nodiscard]] auto GetNewDotPaths(const DotStartsToAndFrom& dotStartsToAndFrom) noexcept
      -> DotPathsToAndFrom;
  auto MakeToDotPathsSameAsFromDotPaths() noexcept -> void;
  [[nodiscard]] auto GetNextDotPositions(const std::vector<UTILS::MATH::OscillatingPath>& dotPath)
      const noexcept -> std::vector<Point2dInt>;
  Direction m_direction         = Direction::TO_TARGET;
  bool m_changeDirectionPending = false;
  static auto IncrementPositionT(std::vector<UTILS::MATH::OscillatingPath>& paths) noexcept -> void;
  [[nodiscard]] auto GetStepSize() const noexcept -> float;
  auto CheckReverse(Direction currentDirection,
                    const std::vector<UTILS::MATH::OscillatingPath>& paths) noexcept -> void;
  [[nodiscard]] static auto GetOppositeDirection(Direction direction) noexcept -> Direction;
  [[nodiscard]] auto GetSmallRandomOffset() const noexcept -> Vec2dInt;
};

inline auto DotPaths::SetPathParams(const DotPathParamsToAndFrom& dotPathParamsToAndFrom) noexcept
    -> void
{
  m_dotPathParamsToAndFrom = dotPathParamsToAndFrom;
}

inline auto DotPaths::SetPositionTNumSteps(const uint32_t numSteps) noexcept -> void
{
  std::for_each(begin(m_dotPathsToAndFrom.dotPathToTarget),
                end(m_dotPathsToAndFrom.dotPathToTarget),
                [&numSteps](UTILS::MATH::IPath& path) { path.SetNumSteps(numSteps); });

  std::for_each(begin(m_dotPathsToAndFrom.dotPathFromTarget),
                end(m_dotPathsToAndFrom.dotPathFromTarget),
                [&numSteps](UTILS::MATH::IPath& path) { path.SetNumSteps(numSteps); });
}

inline auto DotPaths::IncrementPositionT() noexcept -> void
{
  if (m_direction == Direction::TO_TARGET)
  {
    IncrementPositionT(m_dotPathsToAndFrom.dotPathToTarget);
    CheckReverse(Direction::TO_TARGET, m_dotPathsToAndFrom.dotPathToTarget);
  }
  else
  {
    IncrementPositionT(m_dotPathsToAndFrom.dotPathFromTarget);
    CheckReverse(Direction::FROM_TARGET, m_dotPathsToAndFrom.dotPathFromTarget);
  }
}

inline auto DotPaths::IncrementPositionT(std::vector<UTILS::MATH::OscillatingPath>& paths) noexcept
    -> void
{
  if (paths.front().IsStopped())
  {
    return;
  }
  std::for_each(begin(paths), end(paths), [](UTILS::MATH::IPath& path) { path.IncrementT(); });
}

inline auto DotPaths::CheckReverse(const Direction currentDirection,
                                   const std::vector<UTILS::MATH::OscillatingPath>& paths) noexcept
    -> void
{
  if (not paths.front().IsStopped())
  {
    return;
  }
  if (not m_changeDirectionPending)
  {
    m_changeDirectionPending = true;
    return;
  }

  ChangeDirectionWithToTargetFixup(GetOppositeDirection(currentDirection));

  m_changeDirectionPending = false;
}

inline auto DotPaths::GetNumDots() const noexcept -> uint32_t
{
  return m_numDots;
}

inline auto DotPaths::GetCurrentDirection() const noexcept -> Direction
{
  return m_direction;
}

inline auto DotPaths::ChangeDirectionWithToTargetFixup(const Direction newDirection) noexcept
    -> void
{
  if (newDirection == Direction::TO_TARGET)
  {
    MakeToDotPathsSameAsFromDotPaths();
  }
  ChangeDirection(newDirection);
}

inline auto DotPaths::ChangeDirection(const Direction newDirection) noexcept -> void
{
  m_direction = newDirection;

  std::for_each(begin(m_dotPathsToAndFrom.dotPathToTarget),
                end(m_dotPathsToAndFrom.dotPathToTarget),
                [](UTILS::MATH::IPath& path)
                {
                  path.Reset(0.0F);
                  path.IncrementT();
                });

  std::for_each(begin(m_dotPathsToAndFrom.dotPathFromTarget),
                end(m_dotPathsToAndFrom.dotPathFromTarget),
                [](UTILS::MATH::IPath& path)
                {
                  path.Reset(0.0F);
                  path.IncrementT();
                });
}

inline auto DotPaths::GetOppositeDirection(const Direction direction) noexcept -> Direction
{
  if (direction == Direction::TO_TARGET)
  {
    return Direction::FROM_TARGET;
  }

  return Direction::TO_TARGET;
}

inline auto DotPaths::GetPositionTRef() const noexcept -> const UTILS::TValue&
{
  if (m_direction == Direction::TO_TARGET)
  {
    return m_dotPathsToAndFrom.dotPathToTarget.front().GetPositionT();
  }

  return m_dotPathsToAndFrom.dotPathFromTarget.front().GetPositionT();
}

inline auto DotPaths::HasUpdatedDotPathsToAndFrom() const noexcept -> bool
{
  return m_updatedDotPathsToAndFrom;
}

inline auto DotPaths::ResetUpdatedDotPathsToAndFromFlag() noexcept -> void
{
  m_updatedDotPathsToAndFrom = false;
}

inline auto DotPaths::GetPositionTNumSteps() const noexcept -> uint32_t
{
  if (m_direction == Direction::TO_TARGET)
  {
    return m_dotPathsToAndFrom.dotPathToTarget.front().GetNumSteps();
  }

  return m_dotPathsToAndFrom.dotPathFromTarget.front().GetNumSteps();
}

inline auto DotPaths::HasPositionTJustHitStartBoundary() const noexcept -> bool
{
  return IsCloseToStartBoundary(GetStepSize() - UTILS::MATH::SMALL_FLOAT);
}

inline auto DotPaths::HasPositionTJustHitEndBoundary() const noexcept -> bool
{
  return IsCloseToEndBoundary(GetStepSize() - UTILS::MATH::SMALL_FLOAT);
}

inline auto DotPaths::GetStepSize() const noexcept -> float
{
  if (m_direction == Direction::TO_TARGET)
  {
    return m_dotPathsToAndFrom.dotPathToTarget.front().GetStepSize();
  }

  return m_dotPathsToAndFrom.dotPathFromTarget.front().GetStepSize();
}

inline auto DotPaths::IsCloseToStartBoundary(const float distance) const noexcept -> bool
{
  if (m_direction == Direction::TO_TARGET)
  {
    return m_dotPathsToAndFrom.dotPathToTarget.front().GetPositionT()() < distance;
  }

  return m_dotPathsToAndFrom.dotPathFromTarget.front().GetPositionT()() > (1.0F - distance);
}

inline auto DotPaths::IsCloseToEndBoundary(const float distance) const noexcept -> bool
{
  if (m_direction == Direction::TO_TARGET)
  {
    return m_dotPathsToAndFrom.dotPathToTarget.front().GetPositionT()() > (1.0F - distance);
  }

  return m_dotPathsToAndFrom.dotPathFromTarget.front().GetPositionT()() < distance;
}

inline auto DotPaths::GetPositionT() const noexcept -> float
{
  if (m_direction == Direction::TO_TARGET)
  {
    return m_dotPathsToAndFrom.dotPathToTarget.front().GetPositionT()();
  }

  return m_dotPathsToAndFrom.dotPathFromTarget.front().GetPositionT()();
}

} // namespace GOOM::VISUAL_FX::CIRCLES

namespace GOOM::VISUAL_FX::CIRCLES
{

using UTILS::TValue;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::OscillatingPath;
using UTILS::MATH::StartAndEndPos;

DotPaths::DotPaths(const IGoomRand& goomRand,
                   const uint32_t numDots,
                   DotStartsToAndFrom&& dotStartsToAndFrom,
                   const DotTargetsToAndFrom& dotTargetsToAndFrom,
                   const DotPathParamsToAndFrom& dotPathParamsToAndFrom) noexcept
  : m_goomRand{&goomRand},
    m_numDots{numDots},
    m_dotStartsToAndFrom{std::move(dotStartsToAndFrom)},
    m_dotTargetsToAndFrom{dotTargetsToAndFrom},
    m_dotPathParamsToAndFrom{dotPathParamsToAndFrom},
    m_dotPathsToAndFrom{GetNewDotPaths(m_dotStartsToAndFrom)}
{
}

auto DotPaths::GetNewDotPaths(const DotStartsToAndFrom& dotStartsToAndFrom) noexcept
    -> DotPathsToAndFrom
{
  auto dotPathsToAndFrom = DotPathsToAndFrom{};

  for (auto i = 0U; i < m_numDots; ++i)
  {
    dotPathsToAndFrom.dotPathToTarget.emplace_back(
        std::make_unique<TValue>(
            TValue::NumStepsProperties{TValue::StepType::SINGLE_CYCLE, DEFAULT_POSITION_STEPS}),
        StartAndEndPos{ToPoint2dFlt(dotStartsToAndFrom.dotStartingPositionsToTarget.at(i)),
                       ToPoint2dFlt(m_dotTargetsToAndFrom.dotTargetPositionToTarget)},
        m_dotPathParamsToAndFrom.dotPathParamsToTarget);
    dotPathsToAndFrom.dotPathFromTarget.emplace_back(
        std::make_unique<TValue>(
            TValue::NumStepsProperties{TValue::StepType::SINGLE_CYCLE, DEFAULT_POSITION_STEPS}),
        StartAndEndPos{ToPoint2dFlt(m_dotTargetsToAndFrom.dotTargetPositionFromTarget),
                       ToPoint2dFlt(dotStartsToAndFrom.dotStartingPositionsFromTarget.at(i))},
        m_dotPathParamsToAndFrom.dotPathParamsFromTarget);
  }

  return dotPathsToAndFrom;
}

auto DotPaths::MakeToDotPathsSameAsFromDotPaths() noexcept -> void
{
  m_dotPathsToAndFrom.dotPathToTarget.clear();
  for (auto i = 0U; i < m_numDots; ++i)
  {
    m_dotPathsToAndFrom.dotPathToTarget.emplace_back(
        std::make_unique<TValue>(
            TValue::NumStepsProperties{TValue::StepType::SINGLE_CYCLE,
                                       m_dotPathsToAndFrom.dotPathFromTarget.at(i).GetNumSteps()}),
        StartAndEndPos{ToPoint2dFlt(m_dotPathsToAndFrom.dotPathFromTarget.at(i).GetNextPoint()),
                       ToPoint2dFlt(m_dotTargetsToAndFrom.dotTargetPositionFromTarget)},
        m_dotPathParamsToAndFrom.dotPathParamsToTarget);
  }

  m_updatedDotPathsToAndFrom = true;
}

auto DotPaths::GetNextDotPositions() const noexcept -> std::vector<Point2dInt>
{
  if (m_direction == Direction::TO_TARGET)
  {
    return GetNextDotPositions(m_dotPathsToAndFrom.dotPathToTarget);
  }

  return GetNextDotPositions(m_dotPathsToAndFrom.dotPathFromTarget);
}

auto DotPaths::GetNextDotPositions(const std::vector<UTILS::MATH::OscillatingPath>& dotPath)
    const noexcept -> std::vector<Point2dInt>
{
  auto nextDotPositions = std::vector<Point2dInt>(m_numDots);
  for (auto i = 0U; i < m_numDots; ++i)
  {
    nextDotPositions.at(i) = dotPath.at(i).GetNextPoint();
    if (m_randomizePoints)
    {
      nextDotPositions.at(i) = Translate(nextDotPositions.at(i), GetSmallRandomOffset());
    }
  }
  return nextDotPositions;
}

inline auto DotPaths::GetSmallRandomOffset() const noexcept -> Vec2dInt
{
  static constexpr auto MIN_VARIATION = -5;
  static constexpr auto MAX_VARIATION = +5;
  return {m_goomRand->GetRandInRange(MIN_VARIATION, MAX_VARIATION + 1),
          m_goomRand->GetRandInRange(MIN_VARIATION, MAX_VARIATION + 1)};
}

} // namespace GOOM::VISUAL_FX::CIRCLES
