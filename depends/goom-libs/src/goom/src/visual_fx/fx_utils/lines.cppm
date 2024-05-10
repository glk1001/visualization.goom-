module;

#include "goom/goom_config.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

export module Goom.VisualFx.FxUtils:Lines;

import Goom.Utils.Math.GoomRandBase;
import Goom.Utils.Math.Misc;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

export namespace GOOM::VISUAL_FX::FX_UTILS
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

namespace GOOM::VISUAL_FX::FX_UTILS
{

using UTILS::MATH::HALF_PI;
using UTILS::MATH::TWO_PI;

auto GetHorizontalLinePoints(const LineProperties& lineProperties, const float yValue)
    -> std::vector<LinePoint>
{
  Expects(lineProperties.numPoints > 1);

  auto linePoints = std::vector<LinePoint>(lineProperties.numPoints);

  const auto xStep = static_cast<float>(lineProperties.length - 1) /
                     static_cast<float>(lineProperties.numPoints - 1);
  auto x = 0.0F;
  for (auto& pt : linePoints)
  {
    pt.angle   = HALF_PI;
    pt.point.x = x;
    pt.point.y = yValue;

    x += xStep;
  }

  return linePoints;
}

auto GetVerticalLinePoints(const LineProperties& lineProperties, const float xValue)
    -> std::vector<LinePoint>
{
  Expects(lineProperties.numPoints > 1);

  auto linePoints = std::vector<LinePoint>(lineProperties.numPoints);

  const auto yStep = static_cast<float>(lineProperties.length - 1) /
                     static_cast<float>(lineProperties.numPoints - 1);
  auto y = 0.0F;
  for (auto& pt : linePoints)
  {
    pt.angle   = 0.0F;
    pt.point.x = xValue;
    pt.point.y = y;

    y += yStep;
  }

  return linePoints;
}

namespace
{

inline auto RoundToTwoDecimals(const double x) -> double
{
  static constexpr auto TEN_TO_THE_TWO = 100.0;
  return std::round(x * TEN_TO_THE_TWO) / TEN_TO_THE_TWO;
}

} // namespace

auto GetCircularLinePoints(const uint32_t numPoints,
                           const Dimensions& dimensions,
                           const float radius) -> std::vector<LinePoint>
{
  Expects(numPoints > 1);

  auto linePoint = std::vector<LinePoint>(numPoints);

  const auto xCentre = 0.5 * static_cast<double>(dimensions.GetWidth());
  const auto yCentre = 0.5 * static_cast<double>(dimensions.GetHeight());

  const auto angleStep = static_cast<double>(TWO_PI) / static_cast<double>(numPoints - 1);
  auto angle           = 0.0;
  for (auto i = 0U; i < numPoints; ++i)
  {
    linePoint[i].angle   = static_cast<float>(RoundToTwoDecimals(angle));
    linePoint[i].point.x = static_cast<float>(
        RoundToTwoDecimals(xCentre + (static_cast<double>(radius) * std::cos(angle))));
    linePoint[i].point.y = static_cast<float>(
        RoundToTwoDecimals(yCentre + (static_cast<double>(radius) * std::sin(angle))));
    angle += angleStep;
  }

  Ensures(UTILS::MATH::FloatsEqual(linePoint[0].point.x, linePoint[linePoint.size() - 1].point.x));
  Ensures(UTILS::MATH::FloatsEqual(linePoint[0].point.y, linePoint[linePoint.size() - 1].point.y));

  return linePoint;
}

} // namespace GOOM::VISUAL_FX::FX_UTILS
