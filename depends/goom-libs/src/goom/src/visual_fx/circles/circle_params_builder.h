#pragma once

#include "circle.h"
#include "goom/goom_types.h"
#include "goom/point2d.h"
#include "visual_fx/fx_helper.h"

#include <cstdint>
#include <vector>

namespace GOOM::VISUAL_FX::CIRCLES
{

class CircleParamsBuilder
{
public:
  CircleParamsBuilder(uint32_t numCircles, const FxHelper& fxHelper) noexcept;

  enum class CircleStartModes : UnderlyingEnumType
  {
    SAME_RADIUS,
    REDUCING_RADIUS,
    FOUR_CORNERED_IN_MAIN,
  };
  auto SetCircleStartMode(CircleStartModes mode) noexcept -> void;

  enum class CircleTargetModes : UnderlyingEnumType
  {
    FOUR_CORNERS,
    SIMILAR_TARGETS,
  };
  auto SetCircleTargetMode(CircleTargetModes mode) noexcept -> void;

  auto SetMainCircleStartCentre(const Point2dInt& circleCentreStart) noexcept -> void;
  auto SetMainCircleCentreTarget(const Point2dInt& circleCentreTarget) noexcept -> void;
  [[nodiscard]] auto GetMainCircleCentreTarget() const noexcept -> const Point2dInt&;

  [[nodiscard]] auto GetCircleParams() const noexcept -> std::vector<Circle::Params>;
  [[nodiscard]] auto GetCircleParamsTargetsOnly(const std::vector<Circle::Params>& circleParams)
      const noexcept -> std::vector<Circle::Params>;

private:
  uint32_t m_numCircles;
  const FxHelper* m_fxHelper;
  uint32_t m_screenWidth         = m_fxHelper->GetDimensions().GetWidth();
  uint32_t m_screenHeight        = m_fxHelper->GetDimensions().GetHeight();
  Point2dInt m_screenCentre      = MidpointFromOrigin(GetPoint2dInt(m_screenWidth, m_screenHeight));
  Point2dInt m_topLeftCorner     = GetPoint2dInt(0U, 0U);
  Point2dInt m_topRightCorner    = GetPoint2dInt(m_screenWidth - 1, 0U);
  Point2dInt m_bottomLeftCorner  = GetPoint2dInt(0U, m_screenHeight - 1);
  Point2dInt m_bottomRightCorner = GetPoint2dInt(m_screenWidth - 1, m_screenHeight - 1);
  Point2dInt m_mainCircleCentreStart   = m_screenCentre;
  Point2dInt m_mainCircleCentreTarget  = m_screenCentre;
  CircleStartModes m_circleStartMode   = CircleStartModes::SAME_RADIUS;
  CircleTargetModes m_circleTargetMode = CircleTargetModes::FOUR_CORNERS;

  auto SetCircleCentreStarts(std::vector<Circle::Params>& circleParams) const noexcept -> void;
  [[nodiscard]] auto GetCircleCentreStarts(float mainCircleRadius) const noexcept
      -> std::vector<Point2dInt>;
  [[nodiscard]] auto GetAllSameCircleCentreStarts(float mainCircleRadius) const noexcept
      -> std::vector<Point2dInt>;
  [[nodiscard]] auto GetReducingRadiusCircleCentreStarts(float mainCircleRadius) const noexcept
      -> std::vector<Point2dInt>;
  [[nodiscard]] auto GetFourCornersCircleCentreStarts(float mainCircleRadius) const noexcept
      -> std::vector<Point2dInt>;
  [[nodiscard]] auto GetCircleRadii(float mainCircleRadius) const noexcept -> std::vector<float>;
  [[nodiscard]] auto GetSameCircleRadii(float mainCircleRadius) const noexcept
      -> std::vector<float>;
  [[nodiscard]] auto GetReducingCircleRadii(float mainCircleRadius) const noexcept
      -> std::vector<float>;
  [[nodiscard]] auto GetFourCornersCircleRadii(float mainCircleRadius) const noexcept
      -> std::vector<float>;
  [[nodiscard]] auto GetMainCircleRadius() const noexcept -> float;
  [[nodiscard]] auto GetCircleRadiusReducer() const noexcept -> float;

  auto SetCircleCentreTargets(std::vector<Circle::Params>& circleParams) const noexcept -> void;
  [[nodiscard]] auto GetCircleCentreTargets(const Point2dInt& target) const noexcept
      -> std::vector<Point2dInt>;
  [[nodiscard]] auto GetSimilarCircleCentreTargets(const Point2dInt& target) const noexcept
      -> std::vector<Point2dInt>;
  [[nodiscard]] auto GetFourCornersCircleCentreTargets(const Point2dInt& target) const noexcept
      -> std::vector<Point2dInt>;
};

inline auto CircleParamsBuilder::SetCircleStartMode(const CircleStartModes mode) noexcept -> void
{
  m_circleStartMode = mode;
}

inline auto CircleParamsBuilder::SetCircleTargetMode(const CircleTargetModes mode) noexcept -> void
{
  m_circleTargetMode = mode;
}

inline auto CircleParamsBuilder::SetMainCircleStartCentre(
    const Point2dInt& circleCentreStart) noexcept -> void
{
  m_mainCircleCentreStart = circleCentreStart;
}

inline auto CircleParamsBuilder::GetMainCircleCentreTarget() const noexcept -> const Point2dInt&
{
  return m_mainCircleCentreTarget;
}

inline auto CircleParamsBuilder::SetMainCircleCentreTarget(
    const Point2dInt& circleCentreTarget) noexcept -> void
{
  m_mainCircleCentreTarget = circleCentreTarget;
}

} // namespace GOOM::VISUAL_FX::CIRCLES
