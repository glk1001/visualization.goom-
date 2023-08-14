#pragma once

#include "goom_config.h"
#include "goom_types.h"
#include "point2d.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace GOOM::VISUAL_FX::FX_UTILS
{

struct LinePoint
{
  Point2dFlt point;
  float angle{};
};


struct LineProperties
{
  uint32_t numPoints;
  uint32_t length;
};
[[nodiscard]] auto GetHorizontalLinePoints(const LineProperties& lineProperties, float yValue)
    -> std::vector<LinePoint>;
[[nodiscard]] auto GetVerticalLinePoints(const LineProperties& lineProperties, float xValue)
    -> std::vector<LinePoint>;
[[nodiscard]] auto GetCircularLinePoints(uint32_t numPoints,
                                         const Dimensions& dimensions,
                                         float radius) -> std::vector<LinePoint>;
template<typename T>
void SmoothTheCircleJoinAtEnds(std::vector<T>& circlePoints, uint32_t numPointsToSmooth);


template<typename T>
void SmoothTheCircleJoinAtEnds(std::vector<T>& circlePoints, const uint32_t numPointsToSmooth)
{
  Expects(numPointsToSmooth > 0);
  Expects(numPointsToSmooth < circlePoints.size());

  const auto tStep = 1.0F / static_cast<float>(numPointsToSmooth);

  const auto lastPointIndex = circlePoints.size() - 1;
  const auto endDiff = circlePoints[0].point - ToVec2dInt(circlePoints[lastPointIndex].point);
  if ((0 == endDiff.x) and (0 == endDiff.y))
  {
    return;
  }

  const auto minIndexMinus1 = circlePoints.size() - numPointsToSmooth;
  auto diff                 = ToVec2dInt(endDiff);
  auto t                    = 1.0F - tStep;
  for (size_t i = lastPointIndex; i > minIndexMinus1; --i)
  {
    circlePoints[i].point = Translate(circlePoints[i].point, diff);

    diff = {static_cast<int32_t>(std::round(t * static_cast<float>(endDiff.x))),
            static_cast<int32_t>(std::round(t * static_cast<float>(endDiff.y)))};

    t -= tStep;
  }

  Ensures(circlePoints[0].point.x == circlePoints[circlePoints.size() - 1].point.x);
  Ensures(circlePoints[0].point.y == circlePoints[circlePoints.size() - 1].point.y);
}

} // namespace GOOM::VISUAL_FX::FX_UTILS
