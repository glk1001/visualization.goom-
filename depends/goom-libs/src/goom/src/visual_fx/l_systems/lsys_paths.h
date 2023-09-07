#pragma once

#include "goom/point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/parametric_functions2d.h"
#include "utils/math/paths.h"

#include <cstdint>
#include <memory>

namespace GOOM::VISUAL_FX::L_SYSTEM
{

class LSysPath
{
public:
  explicit LSysPath(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  [[nodiscard]] auto GetPathStart() const noexcept -> const Point2dInt&;
  auto SetPathStart(const Point2dInt& pathStart) noexcept -> void;
  [[nodiscard]] auto GetPathTarget() const noexcept -> const Point2dInt&;
  auto SetPathTarget(const Point2dInt& pathTarget) noexcept -> void;

  auto SetPathNumSteps(uint32_t numSteps) noexcept -> void;

  auto Init() noexcept -> void;
  auto IncrementPositionT() noexcept -> void;
  [[nodiscard]] auto GetNextPathPosition() const noexcept -> Point2dInt;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;

  static constexpr auto DEFAULT_PATH_NUM_STEPS = 100U;
  uint32_t m_pathNumSteps                      = DEFAULT_PATH_NUM_STEPS;
  Point2dInt m_lSysPathStart{};
  Point2dInt m_lSysPathTarget{};
  bool m_newLSysPathStart  = false;
  bool m_newLSysPathTarget = false;
  struct PathsToAndFrom
  {
    UTILS::MATH::OscillatingPath pathToTarget;
    UTILS::MATH::OscillatingPath pathFromTarget;
  };
  std::unique_ptr<PathsToAndFrom> m_pathsToAndFrom = GetPathsToAndFrom();
  [[nodiscard]] auto GetPathsToAndFrom() const noexcept -> std::unique_ptr<PathsToAndFrom>;
  [[nodiscard]] auto GetPathParams() const noexcept -> UTILS::MATH::OscillatingFunction::Params;
  enum class Direction
  {
    TO_TARGET,
    FROM_TARGET,
  };
  Direction m_direction = Direction::TO_TARGET;
  auto UpdatePaths() noexcept -> void;
  auto ChangeToFromPath() noexcept -> void;
  auto ChangeToToPath() noexcept -> void;
};

inline auto LSysPath::GetPathStart() const noexcept -> const Point2dInt&
{
  return m_lSysPathStart;
}

inline auto LSysPath::GetPathTarget() const noexcept -> const Point2dInt&
{
  return m_lSysPathTarget;
}

} // namespace GOOM::VISUAL_FX::L_SYSTEM
