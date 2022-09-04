#pragma once

#include "tentacle3d.h"

#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

class CirclesTentacleLayout
{
public:
  CirclesTentacleLayout(float startRadius, float endRadius, uint32_t numPoints) noexcept;

  [[nodiscard]] auto GetNumPoints() const noexcept -> size_t;
  [[nodiscard]] auto GetStartPoints() const noexcept -> const std::vector<V3dFlt>&;
  [[nodiscard]] auto GetEndPoints() const noexcept -> const std::vector<V3dFlt>&;

private:
  const std::vector<V3dFlt> m_startPoints;
  const std::vector<V3dFlt> m_endPoints;

  [[nodiscard]] static auto GetSamplePoints(float radius, uint32_t numPoints) noexcept
      -> std::vector<V3dFlt>;
};

inline auto CirclesTentacleLayout::GetNumPoints() const noexcept -> size_t
{
  return m_startPoints.size();
}

inline auto CirclesTentacleLayout::GetStartPoints() const noexcept -> const std::vector<V3dFlt>&
{
  return m_startPoints;
}

inline auto CirclesTentacleLayout::GetEndPoints() const noexcept -> const std::vector<V3dFlt>&
{
  return m_endPoints;
}

} // namespace GOOM::VISUAL_FX::TENTACLES
