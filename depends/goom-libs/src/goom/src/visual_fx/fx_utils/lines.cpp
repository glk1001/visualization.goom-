#include "lines.h"

#include "utils/mathutils.h"

#undef NDEBUG
#include <cassert>
#include <format>
#include <stdexcept>

namespace GOOM::VISUAL_FX::FX_UTILS
{

using UTILS::floats_equal;
using UTILS::m_half_pi;
using UTILS::m_two_pi;

auto GetHorizontalLinePoints(const uint32_t numPoints, const uint32_t width, const float yValue)
    -> std::vector<LinePoint>
{
  assert(numPoints > 1);

  std::vector<LinePoint> linePoints(numPoints);

  const float xStep = static_cast<float>(width - 1) / static_cast<float>(numPoints - 1);
  float x = 0.0F;
  for (auto& pt : linePoints)
  {
    pt.angle = m_half_pi;
    pt.point.x = x;
    pt.point.y = yValue;

    x += xStep;
  }

  return linePoints;
}

auto GetVerticalLinePoints(const uint32_t numPoints, const uint32_t height, const float xValue)
    -> std::vector<LinePoint>
{
  assert(numPoints > 1);

  std::vector<LinePoint> linePoints(numPoints);

  const float yStep = static_cast<float>(height - 1) / static_cast<float>(numPoints - 1);
  float y = 0.0F;
  for (auto& pt : linePoints)
  {
    pt.angle = 0.0F;
    pt.point.x = xValue;
    pt.point.y = y;

    y += yStep;
  }

  return linePoints;
}

inline auto RoundToTwoDecimals(const double x) -> double
{
  constexpr double TEN_TO_THE_TWO = 100.0;
  return std::round(x * TEN_TO_THE_TWO) / TEN_TO_THE_TWO;
}

auto GetCircularLinePoints(const uint32_t numPoints,
                           const uint32_t width,
                           const uint32_t height,
                           const float radius) -> std::vector<LinePoint>
{
  assert(numPoints > 1);

  std::vector<LinePoint> linePoint(numPoints);

  const double xCentre = 0.5 * static_cast<double>(width);
  const double yCentre = 0.5 * static_cast<double>(height);

  const double angleStep = static_cast<double>(m_two_pi) / static_cast<double>(numPoints - 1);
  double angle = 0.0;
  for (size_t i = 0; i < numPoints; ++i)
  {
    linePoint[i].angle = static_cast<float>(RoundToTwoDecimals(angle));
    linePoint[i].point.x = static_cast<float>(
        RoundToTwoDecimals(xCentre + (static_cast<double>(radius) * std::cos(angle))));
    linePoint[i].point.y = static_cast<float>(
        RoundToTwoDecimals(yCentre + (static_cast<double>(radius) * std::sin(angle))));
    angle += angleStep;
  }

  assert(floats_equal(linePoint[0].point.x, linePoint[linePoint.size() - 1].point.x));
  assert(floats_equal(linePoint[0].point.y, linePoint[linePoint.size() - 1].point.y));

  return linePoint;
}

} // namespace GOOM::VISUAL_FX::FX_UTILS
