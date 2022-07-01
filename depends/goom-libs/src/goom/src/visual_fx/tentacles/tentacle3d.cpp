#include "tentacle3d.h"

#include "color/color_maps.h"
#include "color/color_utils.h"
#include "tentacle2d.h"
#include "utils/math/misc.h"

#include <memory>
#include <tuple>
#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

using COLOR::GetBrighterColor;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using COLOR::COLOR_DATA::ColorMapName;
using UTILS::MATH::IGoomRand;

static constexpr float HEAD_X_MAX = 10.0F;

Tentacle3D::Tentacle3D(std::unique_ptr<Tentacle2D> tentacle,
                       const Pixel& headMainColor,
                       const Pixel& headLowColor,
                       const V3dFlt& head,
                       const size_t numHeadNodes,
                       const IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand},
    m_tentacle{std::move(tentacle)},
    m_headMainColor{headMainColor},
    m_headLowColor{headLowColor},
    m_head{head},
    m_numHeadNodes{numHeadNodes}
{
}

Tentacle3D::~Tentacle3D() noexcept = default;

void Tentacle3D::SetWeightedColorMaps(
    const std::shared_ptr<const COLOR::RandomColorMaps>& weightedMaps)
{
  m_colorMaps = weightedMaps;

  m_colorMapsManager.UpdateColorMapInfo(m_mainColorMapID,
                                        {m_colorMaps, RandomColorMaps::ALL_COLOR_MAP_TYPES});

  m_colorMapsManager.UpdateColorMapInfo(m_lowColorMapID,
                                        {m_colorMaps, RandomColorMaps::ALL_COLOR_MAP_TYPES});

  ColorMapsChanged();
}

void Tentacle3D::ColorMapsChanged()
{
  m_colorMapsManager.ChangeColorMapNow(m_mainColorMapID);
  m_colorMapsManager.ChangeColorMapNow(m_lowColorMapID);

  m_mainColorSegmentMixT =
      m_goomRand.GetRandInRange(MIN_COLOR_SEGMENT_MIX_T, MAX_COLOR_SEGMENT_MIX_T);

  static constexpr float PROB_LOW_MIX_SAME = 0.5F;
  m_lowColorSegmentMixT =
      m_goomRand.ProbabilityOf(PROB_LOW_MIX_SAME)
          ? m_mainColorSegmentMixT
          : m_goomRand.GetRandInRange(MIN_COLOR_SEGMENT_MIX_T, MAX_COLOR_SEGMENT_MIX_T);
}

auto Tentacle3D::GetMixedColors(const size_t nodeNum,
                                const Pixel& mainColor,
                                const Pixel& lowColor,
                                const float brightness) const -> std::pair<Pixel, Pixel>
{
  if (nodeNum < GetNumHeadNodes())
  {
    return GetMixedColors(nodeNum, mainColor, lowColor);
  }

  const auto [mixedMainColor, mixedLowColor] = GetMixedColors(nodeNum, mainColor, lowColor);
  const Pixel mixedMainColorPixel = mixedMainColor;
  const Pixel mixedLowColorPixel = mixedLowColor;
  static constexpr float LOW_BRIGHTNESS_FACTOR = 1.8F;
  return std::make_pair(
      m_colorAdjust.GetAdjustment(brightness, mixedMainColorPixel),
      m_colorAdjust.GetAdjustment(LOW_BRIGHTNESS_FACTOR * brightness, mixedLowColorPixel));
}

auto Tentacle3D::GetMixedColors(const size_t nodeNum,
                                       const Pixel& mainColor,
                                       const Pixel& lowColor) const -> std::pair<Pixel, Pixel>
{
  if (nodeNum < GetNumHeadNodes())
  {
    return GetMixedHeadColors(nodeNum, mainColor, lowColor);
  }

  float t = static_cast<float>(nodeNum + 1) / static_cast<float>(Get2DTentacle().GetNumNodes());
  if (m_reverseColorMix)
  {
    t = 1 - t;
  }

  const Pixel segmentMainColor = m_colorMapsManager.GetColorMap(m_mainColorMapID).GetColor(t);
  const Pixel segmentLowColor = m_colorMapsManager.GetColorMap(m_lowColorMapID).GetColor(t);
  const Pixel mixedMainColor =
      GetFinalMixedColor(mainColor, segmentMainColor, m_mainColorSegmentMixT);
  const Pixel mixedLowColor = GetFinalMixedColor(lowColor, segmentLowColor, m_lowColorSegmentMixT);

  if (std::abs(GetHead().x) < HEAD_X_MAX)
  {
    const float brightnessCut = t * t;
    return std::make_pair(GetBrighterColor(brightnessCut, mixedMainColor),
                          GetBrighterColor(brightnessCut, mixedLowColor));
  }

  return std::make_pair(mixedMainColor, mixedLowColor);
}

inline auto Tentacle3D::GetMixedHeadColors(const size_t nodeNum,
                                           const Pixel& mainColor,
                                           const Pixel& lowColor) const -> std::pair<Pixel, Pixel>
{
  const float t =
      0.5F * (1.0F + (static_cast<float>(nodeNum + 1) / static_cast<float>(GetNumHeadNodes() + 1)));
  const Pixel mixedHeadMainColor = IColorMap::GetColorMix(m_headMainColor, mainColor, t);
  const Pixel mixedHeadLowColor = IColorMap::GetColorMix(m_headLowColor, lowColor, t);
  return std::make_pair(mixedHeadMainColor, mixedHeadLowColor);
}

inline auto Tentacle3D::GetFinalMixedColor(const Pixel& color,
                                           const Pixel& segmentColor,
                                           const float t) const -> Pixel
{
  return IColorMap::GetColorMix(color, segmentColor, t);
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
