#include "circles_tentacle_layout.h"

#include "point2d.h"
#include "utils/math/misc.h"

#include <cmath>
#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

using UTILS::MATH::HALF_PI;
using UTILS::MATH::TWO_PI;

static constexpr auto ANGLE_START  = HALF_PI;
static constexpr auto ANGLE_FINISH = ANGLE_START + TWO_PI;

CirclesTentacleLayout::CirclesTentacleLayout(const LayoutProperties& layoutProperties) noexcept
  : m_startPoints{GetCirclePoints({layoutProperties.startRadius, layoutProperties.numTentacles})},
    m_endPoints{GetCirclePoints({layoutProperties.endRadius, layoutProperties.numTentacles})},
    m_startRadius{layoutProperties.startRadius},
    m_endRadius{layoutProperties.endRadius}
{
}

auto CirclesTentacleLayout::GetCirclePoints(
    const CirclePointsProperties& circlePointsProperties) noexcept -> std::vector<Point2dFlt>
{
  auto circlePoints = std::vector<Point2dFlt>{};

  // TODO(glk) Use TValue
  const auto angleStep =
      (ANGLE_FINISH - ANGLE_START) / static_cast<float>(circlePointsProperties.numPoints);
  auto angle = ANGLE_START;

  for (auto i = 0U; i < circlePointsProperties.numPoints; ++i)
  {
    const auto x = circlePointsProperties.radius * std::cos(angle);
    const auto y = circlePointsProperties.radius * std::sin(angle);

    circlePoints.emplace_back(Point2dFlt{x, y});

    angle += angleStep;
  }

  return circlePoints;
}

} // namespace GOOM::VISUAL_FX::TENTACLES
