#pragma once

#include "tentacle3d.h"
#include "tentacle_layout.h"

#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

class CirclesTentacleLayout : public ITentacleLayout
{
public:
  struct RadiusMinMax
  {
    float min;
    float max;
  };

  CirclesTentacleLayout(const RadiusMinMax& startRadii,
                        const RadiusMinMax& endRadii,
                        const std::vector<uint32_t>& numCircleSamples) noexcept;

  // Order of points is outer circle to inner.
  [[nodiscard]] auto GetNumPoints() const noexcept -> size_t override;
  [[nodiscard]] auto GetStartPoints() const noexcept -> const std::vector<V3dFlt>& override;
  [[nodiscard]] auto GetEndPoints() const noexcept -> const std::vector<V3dFlt>& override;

private:
  const std::vector<V3dFlt> m_startPoints;
  const std::vector<V3dFlt> m_endPoints;

  [[nodiscard]] static auto GetSamplePoints(const RadiusMinMax& radii,
                                            const std::vector<uint32_t>& numCircleSamples) noexcept
      -> std::vector<V3dFlt>;
  static auto AddSamplePoints(std::vector<V3dFlt>& pointsVec,
                              float radius,
                              size_t numSample,
                              float angleStart,
                              float angleFinish) noexcept -> void;
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
