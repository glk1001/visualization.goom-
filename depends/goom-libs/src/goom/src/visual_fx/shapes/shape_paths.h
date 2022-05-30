#pragma once

#include "color/random_colormaps_manager.h"
#include "utils/math/paths.h"
#include "utils/propagate_const.h"

#include <memory>

namespace GOOM::VISUAL_FX::SHAPES
{

class ShapePart;

class ShapePath
{
public:
  struct ColorInfo
  {
    COLOR::RandomColorMapsManager::ColorMapId mainColorMapId{};
    COLOR::RandomColorMapsManager::ColorMapId lowColorMapId{};
    COLOR::RandomColorMapsManager::ColorMapId innerColorMapId{};
  };
  ShapePath(std::shared_ptr<UTILS::MATH::IPath> path, ColorInfo colorInfo) noexcept;

  auto UpdateMainColorInfo(ShapePart& parentShapePart) const noexcept -> void;
  auto UpdateLowColorInfo(ShapePart& parentShapePart) const noexcept -> void;
  auto UpdateInnerColorInfo(ShapePart& parentShapePart) const noexcept -> void;

  auto SetNumSteps(uint32_t val) noexcept -> void;
  auto IncrementT() noexcept -> void;
  auto ResetT(float val) noexcept -> void;
  [[nodiscard]] auto HasJustHitStartBoundary() const noexcept -> bool;
  [[nodiscard]] auto HasJustHitEndBoundary() const noexcept -> bool;
  [[nodiscard]] auto HasJustHitAnyBoundary() const noexcept -> bool;
  [[nodiscard]] auto GetNextPoint() const noexcept -> Point2dInt;
  [[nodiscard]] auto GetColorInfo() const noexcept -> const ColorInfo&;
  [[nodiscard]] auto GetCurrentT() const noexcept -> float;

  [[nodiscard]] auto GetIPath() const noexcept -> const UTILS::MATH::IPath&;
  [[nodiscard]] auto GetIPath() noexcept -> UTILS::MATH::IPath&;

private:
  std::experimental::propagate_const<std::shared_ptr<UTILS::MATH::IPath>> m_path;
  ColorInfo m_colorInfo;
};

inline ShapePath::ShapePath(const std::shared_ptr<UTILS::MATH::IPath> path,
                            const ColorInfo colorInfo) noexcept
  : m_path{path}, m_colorInfo{colorInfo}
{
}

inline auto ShapePath::SetNumSteps(const uint32_t val) noexcept -> void
{
  m_path->SetNumSteps(val);
}

inline auto ShapePath::IncrementT() noexcept -> void
{
  m_path->IncrementT();
}

inline auto ShapePath::ResetT(const float val) noexcept -> void
{
  m_path->Reset(val);
}

inline auto ShapePath::HasJustHitStartBoundary() const noexcept -> bool
{
  return m_path->GetPositionT().HasJustHitStartBoundary();
}

inline auto ShapePath::HasJustHitEndBoundary() const noexcept -> bool
{
  return m_path->GetPositionT().HasJustHitEndBoundary();
}

inline auto ShapePath::HasJustHitAnyBoundary() const noexcept -> bool
{
  return HasJustHitStartBoundary() || HasJustHitEndBoundary();
}

inline auto ShapePath::GetNextPoint() const noexcept -> Point2dInt
{
  return m_path->GetNextPoint();
}

inline auto ShapePath::GetCurrentT() const noexcept -> float
{
  return m_path->GetPositionT()();
}

inline auto ShapePath::GetIPath() const noexcept -> const UTILS::MATH::IPath&
{
  return *m_path;
}

inline auto ShapePath::GetIPath() noexcept -> UTILS::MATH::IPath&
{
  return *m_path;
}

inline auto ShapePath::GetColorInfo() const noexcept -> const ColorInfo&
{
  return m_colorInfo;
}

} // namespace GOOM::VISUAL_FX::SHAPES
