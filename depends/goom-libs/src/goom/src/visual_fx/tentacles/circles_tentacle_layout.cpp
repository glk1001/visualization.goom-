#include "circles_tentacle_layout.h"

#include "tentacle3d.h"
#include "utils/math/misc.h"

#include <format>
#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

using STD20::pi;
using UTILS::MATH::HALF_PI;
using UTILS::MATH::IsOdd;
using UTILS::MATH::U_HALF;

static constexpr auto MIN_NUM_CIRCLES = 2;
static constexpr auto LAYOUT_Z_VALUE  = 0.0F;

static constexpr auto ANGLE_LEFT_START   = +HALF_PI;
static constexpr auto ANGLE_LEFT_FINISH  = 1.5F * pi;
static constexpr auto ANGLE_RIGHT_START  = -HALF_PI;
static constexpr auto ANGLE_RIGHT_FINISH = +HALF_PI;
static constexpr auto ANGLE_OFFSET0      = 0.035F * pi;

CirclesTentacleLayout::CirclesTentacleLayout(const RadiusMinMax& startRadii,
                                             const RadiusMinMax& endRadii,
                                             const std::vector<uint32_t>& numCircleSamples) noexcept
  : m_startPoints{GetSamplePoints(startRadii, numCircleSamples)},
    m_endPoints{GetSamplePoints(endRadii, numCircleSamples)}
{
  Expects(numCircleSamples.size() >= MIN_NUM_CIRCLES);
  UNUSED_FOR_NDEBUG(MIN_NUM_CIRCLES);

  std::for_each(cbegin(numCircleSamples),
                cend(numCircleSamples),
                [](const auto& numSamples) { Expects(not IsOdd(numSamples)); });
}

auto CirclesTentacleLayout::GetSamplePoints(const RadiusMinMax& radii,
                                            const std::vector<uint32_t>& numCircleSamples) noexcept
    -> std::vector<V3dFlt>
{
  auto pointsVec = std::vector<V3dFlt>{};

  const auto numCircles      = numCircleSamples.size();
  const auto offsetStep      = ANGLE_OFFSET0 / static_cast<float>(numCircles - 1);
  const auto startRadiusStep = (radii.max - radii.min) / static_cast<float>(numCircles - 1);

  auto startRadius = radii.max;
  auto angleOffset = ANGLE_OFFSET0;
  for (const auto numSample : numCircleSamples)
  {
    AddSamplePoints(pointsVec,
                    startRadius,
                    U_HALF * numSample,
                    ANGLE_LEFT_START + angleOffset,
                    ANGLE_LEFT_FINISH - angleOffset);
    AddSamplePoints(pointsVec,
                    startRadius,
                    U_HALF * numSample,
                    ANGLE_RIGHT_START + angleOffset,
                    ANGLE_RIGHT_FINISH - angleOffset);

    startRadius -= startRadiusStep;
    angleOffset -= offsetStep;
  }

  return pointsVec;
}

auto CirclesTentacleLayout::AddSamplePoints(std::vector<V3dFlt>& pointsVec,
                                            const float radius,
                                            const size_t numSample,
                                            const float angleStart,
                                            const float angleFinish) noexcept -> void
{
  const auto angleStep = (angleFinish - angleStart) / static_cast<float>(numSample - 1);
  auto angle           = angleStart;

  for (auto i = 0U; i < numSample; ++i)
  {
    const auto x     = radius * std::cos(angle);
    const auto y     = radius * std::sin(angle);
    const auto point = V3dFlt{x, y, LAYOUT_Z_VALUE};

    pointsVec.emplace_back(point);

    angle += angleStep;
  }
}

} // namespace GOOM::VISUAL_FX::TENTACLES
