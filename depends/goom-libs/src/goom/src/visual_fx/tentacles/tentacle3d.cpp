//#undef NO_LOGGING

#include "tentacle3d.h"

#include "debugging_logger.h"
#include "goom_config.h"
#include "goom_logger.h"
#include "tentacle2d.h"

#include <memory>
#include <utility>
#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

Tentacle3D::Tentacle3D(std::unique_ptr<Tentacle2D> tentacle) noexcept
  : m_tentacle{std::move(tentacle)}
{
}

Tentacle3D::~Tentacle3D() noexcept = default;

auto Tentacle3D::SetEndPosOffset(const V3dFlt& endPosOffset) noexcept -> void
{
  m_endPosOffset = endPosOffset;
}

auto Tentacle3D::GetTentacleVertices(const V3dFlt& startPosOffset) const -> std::vector<V3dFlt>
{
  const auto& [xVec2D, yVec2D] = m_tentacle->GetDampedXAndYVectors();

  const auto numPoints = xVec2D.size();
  auto vec3d           = std::vector<V3dFlt>(numPoints);

  const auto x0 = m_startPos.x + startPosOffset.x;
  const auto xn = m_endPos.x + m_endPosOffset.x;
  const auto y0 = m_startPos.y + startPosOffset.y + static_cast<float>(yVec2D[0]);
  const auto yn = m_endPos.y + m_endPosOffset.y + static_cast<float>(yVec2D[0]);
  const auto z0 = startPosOffset.z + static_cast<float>(xVec2D[0]);

  const auto xStep = (xn - x0) / static_cast<float>(numPoints - 1);
  const auto yStep = (yn - y0) / static_cast<float>(numPoints - 1);

  auto x = x0;
  auto y = y0;
  for (auto i = 0U; i < numPoints; ++i)
  {
    vec3d[i].x = x;
    vec3d[i].y = y + static_cast<float>(yVec2D[i]);
    vec3d[i].z = z0 + static_cast<float>(xVec2D[i]);

    x += xStep;
    y += yStep;
  }

  return vec3d;
}

} // namespace GOOM::VISUAL_FX::TENTACLES
