#include "circles_tentacle_layout.h"

#include "tentacle3d.h"
#include "utils/math/misc.h"

#include <format>
#include <stdexcept>
#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

using STD20::pi;
using UTILS::MATH::HALF_PI;
using UTILS::MATH::IsOdd;
using UTILS::MATH::U_HALF;

CirclesTentacleLayout::CirclesTentacleLayout(const float radiusMin,
                                             const float radiusMax,
                                             const std::vector<uint32_t>& numCircleSamples,
                                             const float zConst)
{
  const size_t numCircles = numCircleSamples.size();
  if (constexpr size_t MIN_NUM_CIRCLES = 2; numCircles < MIN_NUM_CIRCLES)
  {
    throw std::logic_error(
        std20::format("There must be >= 2 circle sample numbers not {}.", numCircles));
  }
  for (const auto numSample : numCircleSamples)
  {
    if (IsOdd(numSample))
    {
      // Perspective looks bad with odd because of x=0 tentacle.
      throw std::logic_error(std20::format("Circle sample num must be even not {}.", numSample));
    }
  }

  const auto getSamplePoints = [&](const float radius, const size_t numSample,
                                   const float angleStart, const float angleFinish)
  {
    const float angleStep = (angleFinish - angleStart) / static_cast<float>(numSample - 1);
    float angle = angleStart;
    for (size_t i = 0; i < numSample; ++i)
    {
      const float x = radius * std::cos(angle);
      const float y = radius * std::sin(angle);
      const V3dFlt point = {x, y, zConst};
      m_points.push_back(point);
      angle += angleStep;
    }
  };

  const float angleLeftStart = +HALF_PI;
  const float angleLeftFinish = 1.5F * pi;
  const float angleRightStart = -HALF_PI;
  const float angleRightFinish = +HALF_PI;

  const float angleOffsetStart = 0.035F * pi;
  const float angleOffsetFinish = 0.035F * pi;
  const float offsetStep =
      (angleOffsetStart - angleOffsetFinish) / static_cast<float>(numCircles - 1);
  const float radiusStep = (radiusMax - radiusMin) / static_cast<float>(numCircles - 1);

  float radius = radiusMax;
  float angleOffset = angleOffsetStart;
  for (const auto numSample : numCircleSamples)
  {
    getSamplePoints(radius, U_HALF * numSample, angleLeftStart + angleOffset,
                    angleLeftFinish - angleOffset);
    getSamplePoints(radius, U_HALF * numSample, angleRightStart + angleOffset,
                    angleRightFinish - angleOffset);

    radius -= radiusStep;
    angleOffset -= offsetStep;
  }
}

auto CirclesTentacleLayout::GetNumPoints() const -> size_t
{
  return m_points.size();
}

auto CirclesTentacleLayout::GetPoints() const -> const std::vector<V3dFlt>&
{
  return m_points;
}

} // namespace GOOM::VISUAL_FX::TENTACLES
