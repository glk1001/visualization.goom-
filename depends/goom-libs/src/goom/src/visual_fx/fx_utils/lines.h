#pragma once

#include "point2d.h"

#include <cassert>
#include <cstdint>
#include <vector>

namespace GOOM::VISUAL_FX::FX_UTILS
{

struct LinePoint
{
  Point2dFlt point;
  float angle;
};


[[nodiscard]] auto GetHorizontalLinePoints(uint32_t numPoints, uint32_t width, float yValue)
    -> std::vector<LinePoint>;
[[nodiscard]] auto GetVerticalLinePoints(uint32_t numPoints, uint32_t height, float xValue)
    -> std::vector<LinePoint>;
[[nodiscard]] auto GetCircularLinePoints(uint32_t numPoints,
                                         uint32_t width,
                                         uint32_t height,
                                         float radius) -> std::vector<LinePoint>;
template<typename T>
void SmoothTheCircleJoinAtEnds(std::vector<T>& circlePoints, uint32_t numPointsToSmooth);


template<typename T>
void SmoothTheCircleJoinAtEnds(std::vector<T>& circlePoints, uint32_t numPointsToSmooth)
{
  assert(numPointsToSmooth > 0);
  assert(numPointsToSmooth < circlePoints.size());

  const float tStep = 1.0F / static_cast<float>(numPointsToSmooth);

  const size_t lastPointIndex = circlePoints.size() - 1;
  const Point2dInt endDiff = circlePoints[0].point - Vec2dInt{circlePoints[lastPointIndex].point};
  if ((0 == endDiff.x) && (0 == endDiff.y))
  {
    return;
  }

  const size_t minIndexMinus1 = circlePoints.size() - numPointsToSmooth;
  Vec2dInt diff{endDiff};
  float t = 1.0F - tStep;
  for (size_t i = lastPointIndex; i > minIndexMinus1; --i)
  {
    circlePoints[i].point.Translate(diff);

    diff = {static_cast<int32_t>(std::round(t * static_cast<float>(endDiff.x))),
            static_cast<int32_t>(std::round(t * static_cast<float>(endDiff.y)))};

    t -= tStep;
  }

  assert(circlePoints[0].point.x == circlePoints[circlePoints.size() - 1].point.x);
  assert(circlePoints[0].point.y == circlePoints[circlePoints.size() - 1].point.y);
}

} // namespace GOOM::VISUAL_FX::FX_UTILS