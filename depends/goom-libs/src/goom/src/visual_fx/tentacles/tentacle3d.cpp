#include "tentacle3d.h"

#include "color/colormaps.h"
#include "color/colorutils.h"
#include "tentacle2d.h"
#include "utils/mathutils.h"

#undef NDEBUG
#include <cassert>
#include <memory>
#include <tuple>
#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

using COLOR::GetBrighterColor;
using COLOR::GetIncreasedChroma;
using COLOR::IColorMap;
using UTILS::IGoomRand;

constexpr float HEAD_X_MAX = 10.0F;

Tentacle3D::Tentacle3D(std::unique_ptr<Tentacle2D> tentacle,
                       const Pixel& headColor,
                       const Pixel& headLowColor,
                       const V3dFlt& head,
                       const size_t numHeadNodes,
                       const IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand},
    m_tentacle{std::move(tentacle)},
    m_headColor{headColor},
    m_headLowColor{headLowColor},
    m_head{head},
    m_numHeadNodes{numHeadNodes}
{
}

void Tentacle3D::ColorMapsChanged()
{
  m_currentColorMap = m_colorMaps->GetRandomColorMapPtr(COLOR::RandomColorMaps::ALL);
  m_currentLowColorMap = m_colorMaps->GetRandomColorMapPtr(COLOR::RandomColorMaps::ALL);

  constexpr float PROB_CHROMA_INCREASE = 0.7F;
  m_useIncreasedChroma = m_goomRand.ProbabilityOf(PROB_CHROMA_INCREASE);
}

auto Tentacle3D::GetMixedColors(const size_t nodeNum,
                                const Pixel& color,
                                const Pixel& lowColor,
                                const float brightness) const -> std::pair<Pixel, Pixel>
{
  if (nodeNum < GetNumHeadNodes())
  {
    return GetMixedColors(nodeNum, color, lowColor);
  }

  const auto [mixedColor, mixedLowColor] = GetMixedColors(nodeNum, color, lowColor);
  const Pixel mixedColorPixel = mixedColor;
  const Pixel mixedLowColorPixel = mixedLowColor;
  return std::make_pair(GetBrighterColor(brightness, mixedColorPixel),
                        GetBrighterColor(brightness, mixedLowColorPixel));
}

inline auto Tentacle3D::GetMixedColors(const size_t nodeNum,
                                       const Pixel& color,
                                       const Pixel& lowColor) const -> std::pair<Pixel, Pixel>
{
  assert(m_currentColorMap != nullptr);
  assert(m_currentLowColorMap != nullptr);

  if (nodeNum < GetNumHeadNodes())
  {
    return GetMixedHeadColors(nodeNum, color, lowColor);
  }

  float t = static_cast<float>(nodeNum + 1) / static_cast<float>(Get2DTentacle().GetNumNodes());
  if (m_reverseColorMix)
  {
    t = 1 - t;
  }

  const Pixel segmentColor = m_currentColorMap->GetColor(t);
  const Pixel segmentLowColor = m_currentLowColorMap->GetColor(t);
  const Pixel mixedColor = GetFinalMixedColor(color, segmentColor, 0.8F);
  const Pixel mixedLowColor = GetFinalMixedColor(lowColor, segmentLowColor, 0.8F);

  if (std::abs(GetHead().x) < HEAD_X_MAX)
  {
    const float brightnessCut = t * t;
    return std::make_pair(GetBrighterColor(brightnessCut, mixedColor),
                          GetBrighterColor(brightnessCut, mixedLowColor));
  }

  return std::make_pair(mixedColor, mixedLowColor);
}

inline auto Tentacle3D::GetMixedHeadColors(const size_t nodeNum,
                                           const Pixel& color,
                                           const Pixel& lowColor) const -> std::pair<Pixel, Pixel>
{
  const float t =
      0.5F * (1.0F + (static_cast<float>(nodeNum + 1) / static_cast<float>(GetNumHeadNodes() + 1)));
  const Pixel mixedHeadColor = IColorMap::GetColorMix(m_headColor, color, t);
  const Pixel mixedHeadLowColor = IColorMap::GetColorMix(m_headLowColor, lowColor, t);
  return std::make_pair(mixedHeadColor, mixedHeadLowColor);
}

inline auto Tentacle3D::GetFinalMixedColor(const Pixel& color,
                                           const Pixel& segmentColor,
                                           const float t) const -> Pixel
{
  const Pixel finalColor = IColorMap::GetColorMix(color, segmentColor, t);
  if (!m_useIncreasedChroma)
  {
    return finalColor;
  }
  return GetIncreasedChroma(finalColor);
}

auto Tentacle3D::GetVertices() const -> std::vector<V3dFlt>
{
  const auto [xVec2D, yVec2D] = m_tentacle->GetDampedXAndYVectors();
  const size_t n = xVec2D.size();

  std::vector<V3dFlt> vec3d(n);
  const float x0 = m_head.x;
  const float y0 = m_head.y - static_cast<float>(yVec2D[0]);
  const float z0 = m_head.z - static_cast<float>(xVec2D[0]);
  float xStep = 0.0;
  if (std::abs(x0) < HEAD_X_MAX)
  {
    const float xn = 0.1F * x0;
    xStep = (x0 - xn) / static_cast<float>(n);
  }
  float x = x0;
  for (size_t i = 0; i < n; ++i)
  {
    vec3d[i].x = x;
    vec3d[i].z = z0 + static_cast<float>(xVec2D[i]);
    vec3d[i].y = y0 + static_cast<float>(yVec2D[i]);

    x -= xStep;
  }

  return vec3d;
}

} // namespace GOOM::VISUAL_FX::TENTACLES
