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
  const auto numCircles = numCircleSamples.size();
  if (static constexpr auto MIN_NUM_CIRCLES = 2; numCircles < MIN_NUM_CIRCLES)
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

  const auto getSamplePoints = [&](const float radius,
                                   const size_t numSample,
                                   const float angleStart,
                                   const float angleFinish)
  {
    const auto angleStep = (angleFinish - angleStart) / static_cast<float>(numSample - 1);
    auto angle           = angleStart;
    for (auto i = 0U; i < numSample; ++i)
    {
      const auto x     = radius * std::cos(angle);
      const auto y     = radius * std::sin(angle);
      const auto point = V3dFlt{x, y, zConst};
      m_points.push_back(point);
      angle += angleStep;
    }
  };

  const auto angleLeftStart   = +HALF_PI;
  const auto angleLeftFinish  = 1.5F * pi;
  const auto angleRightStart  = -HALF_PI;
  const auto angleRightFinish = +HALF_PI;

  const auto angleOffsetStart  = 0.035F * pi;
  const auto angleOffsetFinish = 0.035F * pi;
  const auto offsetStep =
      (angleOffsetStart - angleOffsetFinish) / static_cast<float>(numCircles - 1);
  const auto radiusStep = (radiusMax - radiusMin) / static_cast<float>(numCircles - 1);

  auto radius      = radiusMax;
  auto angleOffset = angleOffsetStart;
  for (const auto numSample : numCircleSamples)
  {
    getSamplePoints(
        radius, U_HALF * numSample, angleLeftStart + angleOffset, angleLeftFinish - angleOffset);
    getSamplePoints(
        radius, U_HALF * numSample, angleRightStart + angleOffset, angleRightFinish - angleOffset);

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
