module;

#include "goom/point2d.h"
#include "utils/math/misc.h"

#include <cmath>
#include <cstdint>
#include <vector>

module Goom.VisualFx.TentaclesFx:CirclesTentacleLayout;

namespace GOOM::VISUAL_FX::TENTACLES
{

class CirclesTentacleLayout
{
public:
  struct LayoutProperties
  {
    float startRadius;
    float endRadius;
    uint32_t numTentacles;
  };

  explicit CirclesTentacleLayout(const LayoutProperties& layoutProperties) noexcept;

  [[nodiscard]] auto GetNumTentacles() const noexcept -> uint32_t;

  [[nodiscard]] auto GetStartPoints() const noexcept -> const std::vector<Point2dFlt>&;
  [[nodiscard]] auto GetEndPoints() const noexcept -> const std::vector<Point2dFlt>&;

  [[nodiscard]] auto GetStartRadius() const noexcept -> float;
  [[nodiscard]] auto GetEndRadius() const noexcept -> float;

  struct CirclePointsProperties
  {
    float radius;
    uint32_t numPoints;
  };
  [[nodiscard]] static auto GetCirclePoints(
      const CirclePointsProperties& circlePointsProperties) noexcept -> std::vector<Point2dFlt>;

private:
  std::vector<Point2dFlt> m_startPoints;
  std::vector<Point2dFlt> m_endPoints;
  float m_startRadius;
  float m_endRadius;
};

inline auto CirclesTentacleLayout::GetNumTentacles() const noexcept -> uint32_t
{
  return static_cast<uint32_t>(m_startPoints.size());
}

inline auto CirclesTentacleLayout::GetStartPoints() const noexcept -> const std::vector<Point2dFlt>&
{
  return m_startPoints;
}

inline auto CirclesTentacleLayout::GetEndPoints() const noexcept -> const std::vector<Point2dFlt>&
{
  return m_endPoints;
}

inline auto CirclesTentacleLayout::GetStartRadius() const noexcept -> float
{
  return m_startRadius;
}

inline auto CirclesTentacleLayout::GetEndRadius() const noexcept -> float
{
  return m_endRadius;
}

} // namespace GOOM::VISUAL_FX::TENTACLES

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
