#pragma once

#include "tentacle3d.h"

#include <cstdint>
#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

class CirclesTentacleLayout
{
public:
  CirclesTentacleLayout(float startRadius, float endRadius, uint32_t numPoints) noexcept;

  [[nodiscard]] auto GetNumPoints() const noexcept -> uint32_t;

  [[nodiscard]] auto GetStartPoints() const noexcept -> const std::vector<V3dFlt>&;
  [[nodiscard]] auto GetEndPoints() const noexcept -> const std::vector<V3dFlt>&;

  [[nodiscard]] auto GetStartRadius() const noexcept -> float;
  [[nodiscard]] auto GetEndRadius() const noexcept -> float;

  [[nodiscard]] static auto GetCirclePoints(float radius, uint32_t numPoints) noexcept
      -> std::vector<V3dFlt>;

private:
  const std::vector<V3dFlt> m_startPoints;
  const std::vector<V3dFlt> m_endPoints;
  const float m_startRadius;
  const float m_endRadius;
};

inline auto CirclesTentacleLayout::GetNumPoints() const noexcept -> uint32_t
{
  return static_cast<uint32_t>(m_startPoints.size());
}

inline auto CirclesTentacleLayout::GetStartPoints() const noexcept -> const std::vector<V3dFlt>&
{
  return m_startPoints;
}

inline auto CirclesTentacleLayout::GetEndPoints() const noexcept -> const std::vector<V3dFlt>&
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
