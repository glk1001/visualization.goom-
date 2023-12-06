#pragma once

#include "../fx_helper.h"
#include "goom/goom_types.h"
#include "goom/math20.h"
#include "goom/point2d.h"
#include "goom_plugin_info.h"
#include "utils/math/paths.h"
#include "utils/t_values.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX::RAINDROPS
{

class RaindropPositions
{
public:
  struct Params
  {
    uint32_t numConcentricCircles{};
    float enclosingRadius{};
    Point2dInt sourceRectangleWeightPoint{};
    Point2dInt targetRectangleWeightPoint{};
  };

  RaindropPositions(const FxHelper& fxHelper, uint32_t numRaindrops, const Params& params) noexcept;

  auto SetTargetRectangleWeightPoint(const Point2dInt& targetRectangleWeightPoint) noexcept -> void;
  auto ResetPositions(uint32_t newNumConcentricCircles, uint32_t newNumRaindrops) noexcept -> void;
  [[nodiscard]] auto OkToChangeNumRaindrops() const noexcept -> bool;
  auto UpdatePositions() noexcept -> void;

  [[nodiscard]] auto GetEnclosingRadius() const noexcept -> float;
  [[nodiscard]] auto GetCurrentRectangleWeightPoint() const noexcept -> Point2dInt;

  [[nodiscard]] auto GetPosition(uint32_t dropNum) const noexcept -> Point2dInt;

private:
  const FxHelper* m_fxHelper;
  Params m_params;
  Point2dInt m_screenCentre = m_fxHelper->GetGoomInfo().GetDimensions().GetCentrePoint();
  static constexpr auto WEIGHT_POINT_CLOSE_TO_SCREEN_CENTRE_T = 0.2F;
  static constexpr auto MIN_WEIGHT_POINT_STEPS                = 10U;
  static constexpr auto MAX_WEIGHT_POINT_STEPS                = 20U;
  UTILS::TValue m_rectangleWeightPointT{
      {UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE, MIN_WEIGHT_POINT_STEPS}
  };

  static constexpr auto NUM_RAINDROP_POSITION_INCREMENTS = 100U;
  static constexpr auto RAINDROP_MOVEMENT_DELAY_AT_START = 10U;
  static constexpr auto RAINDROP_MOVEMENT_DELAY_AT_END   = 10U;
  std::vector<UTILS::TValue::DelayPoint> m_raindropDelayPoints{
      {0.0F, RAINDROP_MOVEMENT_DELAY_AT_START},
      {1.0F,   RAINDROP_MOVEMENT_DELAY_AT_END}
  };
  UTILS::TValue m_raindropPositionT{
      {UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE, NUM_RAINDROP_POSITION_INCREMENTS},
      m_raindropDelayPoints
  };
  static constexpr auto MIN_RAINDROP_POSITION_T = 0.00F;
  static constexpr auto MAX_RAINDROP_POSITION_T = 0.95F;
  std::vector<Point2dInt> m_raindropPositions;
  std::vector<std::unique_ptr<UTILS::MATH::IPath>> m_raindropPaths;
  [[nodiscard]] auto GetNewRaindropPaths() const noexcept
      -> std::vector<std::unique_ptr<UTILS::MATH::IPath>>;
  [[nodiscard]] auto GetNewRaindropPositions(uint32_t numRaindrops) const noexcept
      -> std::vector<Point2dInt>;
  [[nodiscard]] auto GetPositionsOnConcentricCircles(uint32_t numPositions,
                                                     const Point2dInt& centre,
                                                     uint32_t minRadius,
                                                     uint32_t maxRadius) const noexcept
      -> std::vector<Point2dInt>;
  [[nodiscard]] auto GetPositionsOnCircle(uint32_t num,
                                          const Point2dInt& centre,
                                          uint32_t radius) const noexcept
      -> std::vector<Point2dInt>;

  auto SetNewTargetRectangleWeightPoint(const Point2dInt& targetRectangleWeightPoint) noexcept
      -> void;
  auto GetAcceptableTargetRectangleWeightPoint(
      const Point2dInt& requestedTargetRectangleWeightPoint) const noexcept -> Point2dInt;
};

inline auto RaindropPositions::GetPosition(const uint32_t dropNum) const noexcept -> Point2dInt
{
  const auto t =
      STD20::lerp(MIN_RAINDROP_POSITION_T, MAX_RAINDROP_POSITION_T, m_raindropPositionT());

  return lerp(m_raindropPaths.at(dropNum)->GetNextPoint(), GetCurrentRectangleWeightPoint(), t);
}

inline auto RaindropPositions::GetEnclosingRadius() const noexcept -> float
{
  return m_params.enclosingRadius;
}

inline auto RaindropPositions::SetTargetRectangleWeightPoint(
    const Point2dInt& targetRectangleWeightPoint) noexcept -> void
{
  SetNewTargetRectangleWeightPoint(targetRectangleWeightPoint);
}

inline auto RaindropPositions::OkToChangeNumRaindrops() const noexcept -> bool
{
  return m_raindropPositionT.HasJustHitEndBoundary();
}

} // namespace GOOM::VISUAL_FX::RAINDROPS
