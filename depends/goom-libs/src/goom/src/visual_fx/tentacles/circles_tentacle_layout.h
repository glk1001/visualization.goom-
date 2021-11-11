#pragma once

#include "tentacle3d.h"
#include "tentacle_layout.h"

#include <cstdint>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
namespace TENTACLES
{
#else
namespace GOOM::VISUAL_FX::TENTACLES
{
#endif

class CirclesTentacleLayout : public ITentacleLayout
{
public:
  CirclesTentacleLayout(float radiusMin,
                        float radiusMax,
                        const std::vector<size_t>& numCircleSamples,
                        float zConst);
  // Order of points is outer circle to inner.
  [[nodiscard]] auto GetNumPoints() const -> size_t override;
  [[nodiscard]] auto GetPoints() const -> const std::vector<V3dFlt>& override;

private:
  std::vector<V3dFlt> m_points{};
};

#if __cplusplus <= 201402L
} // namespace TENTACLES
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::TENTACLES
#endif
