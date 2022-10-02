#include "circles_tentacle_layout.h"

#include "tentacle3d.h"
#include "utils/math/misc.h"

#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

using UTILS::MATH::HALF_PI;
using UTILS::MATH::TWO_PI;

static constexpr auto LAYOUT_Z_VALUE = -10.0F;
static constexpr auto ANGLE_START    = HALF_PI;
static constexpr auto ANGLE_FINISH   = ANGLE_START + TWO_PI;

CirclesTentacleLayout::CirclesTentacleLayout(const float startRadius,
                                             const float endRadius,
                                             const uint32_t numPoints) noexcept
  : m_startPoints{GetCirclePoints(startRadius, numPoints)},
    m_endPoints{GetCirclePoints(endRadius, numPoints)},
    m_startRadius{startRadius},
    m_endRadius{endRadius}
{
}

auto CirclesTentacleLayout::GetCirclePoints(float radius, uint32_t numPoints) noexcept
    -> std::vector<V3dFlt>
{
  auto pointsVec = std::vector<V3dFlt>{};

  const auto angleStep = (ANGLE_FINISH - ANGLE_START) / static_cast<float>(numPoints);
  auto angle           = ANGLE_START;

  for (auto i = 0U; i < numPoints; ++i)
  {
    const auto x = radius * std::cos(angle);
    const auto y = radius * std::sin(angle);
    const auto z = LAYOUT_Z_VALUE;

    pointsVec.emplace_back(V3dFlt{x, y, z});

    angle += angleStep;
  }

  return pointsVec;
}

} // namespace GOOM::VISUAL_FX::TENTACLES
