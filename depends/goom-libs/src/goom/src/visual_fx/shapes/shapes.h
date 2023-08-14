#pragma once

#include "color/color_maps.h"
#include "color/random_color_maps.h"
#include "draw/goom_draw.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "goom_types.h"
#include "point2d.h"
#include "shape_parts.h"
#include "utils/math/goom_rand_base.h"
#include "utils/t_values.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace GOOM::VISUAL_FX::SHAPES
{

class Shape
{
public:
  struct Params
  {
    float minRadiusFraction{};
    float maxRadiusFraction{};
    int32_t minShapeDotRadius{};
    int32_t maxShapeDotRadius{};
    uint32_t maxNumShapePaths{};
    Point2dInt zoomMidpoint;
    uint32_t minNumShapePathSteps{};
    uint32_t maxNumShapePathSteps{};
  };

  Shape(DRAW::IGoomDraw& draw,
        const UTILS::MATH::IGoomRand& goomRand,
        const PluginInfo& goomInfo,
        const Params& params,
        PixelChannelType defaultAlpha) noexcept;

  auto SetWeightedMainColorMaps(const COLOR::WeightedRandomColorMaps& weightedMaps) noexcept
      -> void;
  auto SetWeightedLowColorMaps(const COLOR::WeightedRandomColorMaps& weightedMaps) noexcept -> void;
  auto SetWeightedInnerColorMaps(const COLOR::WeightedRandomColorMaps& weightedMaps) noexcept
      -> void;

  auto SetVaryDotRadius(bool val) -> void;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;
  auto SetShapePathsMinMaxNumSteps(const MinMaxValues<uint32_t>& minMaxShapePathsNumSteps) noexcept
      -> void;

  auto Start() noexcept -> void;

  auto Draw() noexcept -> void;
  auto Update() noexcept -> void;

  [[nodiscard]] auto GetTotalNumShapePaths() const noexcept -> uint32_t;
  [[nodiscard]] auto GetNumShapeParts() const noexcept -> size_t;
  [[nodiscard]] auto GetShapePart(size_t shapePartNum) const noexcept -> const ShapePart&;

  [[nodiscard]] auto HasFirstShapePathJustHitStartBoundary() const noexcept -> bool;
  [[nodiscard]] auto HasFirstShapePathJustHitEndBoundary() const noexcept -> bool;
  [[nodiscard]] auto FirstShapePathAtMeetingPoint() const noexcept -> bool;
  [[nodiscard]] auto FirstShapePathsCloseToMeeting() const noexcept -> bool;

  auto DoRandomChanges() noexcept -> void;
  auto SetShapeNumSteps() noexcept -> void;
  auto SetFixedShapeNumSteps() noexcept -> void;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;

  static constexpr uint32_t NUM_SHAPE_PARTS = 10;
  std::vector<ShapePart> m_shapeParts;
  [[nodiscard]] static auto GetInitialShapeParts(DRAW::IGoomDraw& draw,
                                                 const UTILS::MATH::IGoomRand& goomRand,
                                                 const PluginInfo& goomInfo,
                                                 const Params& params,
                                                 PixelChannelType defaultAlpha) noexcept
      -> std::vector<ShapePart>;
  [[nodiscard]] auto GetFirstShapePathPositionT() const noexcept -> float;

  bool m_varyDotRadius                                    = false;
  COLOR::ColorMapSharedPtr m_meetingPointMainColorMapPtr  = nullptr;
  COLOR::ColorMapSharedPtr m_meetingPointLowColorMapPtr   = nullptr;
  static constexpr uint32_t NUM_MEETING_POINT_COLOR_STEPS = 50;
  UTILS::TValue m_meetingPointColorsT{
      {UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE, NUM_MEETING_POINT_COLOR_STEPS}
  };
  [[nodiscard]] auto GetCurrentMeetingPointColors() const noexcept -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetBrightnessAttenuation() const noexcept -> float;

  static constexpr float STARTING_FIXED_T_MIN_MAX_LERP = 0.5F;
  float m_fixedTMinMaxLerp                             = STARTING_FIXED_T_MIN_MAX_LERP;
  auto SetRandomShapeNumSteps() noexcept -> void;
};

inline auto Shape::SetVaryDotRadius(const bool val) -> void
{
  m_varyDotRadius = val;
}

inline auto Shape::GetNumShapeParts() const noexcept -> size_t
{
  return m_shapeParts.size();
}

inline auto Shape::GetShapePart(const size_t shapePartNum) const noexcept -> const ShapePart&
{
  return m_shapeParts.at(shapePartNum);
}

inline auto Shape::HasFirstShapePathJustHitStartBoundary() const noexcept -> bool
{
  if (0 == m_shapeParts.front().GetNumShapePaths())
  {
    return false;
  }

  return m_shapeParts.front().GetShapePath(0).HasJustHitStartBoundary();
}

inline auto Shape::HasFirstShapePathJustHitEndBoundary() const noexcept -> bool
{
  if (0 == m_shapeParts.front().GetNumShapePaths())
  {
    return false;
  }

  return m_shapeParts.front().GetShapePath(0).HasJustHitEndBoundary();
}

inline auto Shape::FirstShapePathAtMeetingPoint() const noexcept -> bool
{
  return HasFirstShapePathJustHitStartBoundary() || HasFirstShapePathJustHitEndBoundary();
}

inline auto Shape::FirstShapePathsCloseToMeeting() const noexcept -> bool
{
  return m_shapeParts.front().AreShapePathsCloseToMeeting();
}

inline auto Shape::GetFirstShapePathPositionT() const noexcept -> float
{
  if (0 == m_shapeParts.front().GetNumShapePaths())
  {
    return 1.0F;
  }

  return m_shapeParts.front().GetFirstShapePathPositionT();
}

inline auto Shape::SetShapeNumSteps() noexcept -> void
{
  if (static constexpr auto PROB_FIXED_NUM_STEPS = 0.95F;
      m_goomRand->ProbabilityOf(PROB_FIXED_NUM_STEPS))
  {
    SetFixedShapeNumSteps();
  }
  else
  {
    SetRandomShapeNumSteps();
  }
}

} // namespace GOOM::VISUAL_FX::SHAPES
