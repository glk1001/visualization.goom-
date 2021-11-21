#include "tentacle_plotter.h"

#include "draw/goom_draw.h"
#include "utils/mathutils.h"
#include "v2d.h"

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

using DRAW::IGoomDraw;
using UTILS::IGoomRand;
using UTILS::m_half_pi;
using UTILS::m_pi;
using UTILS::SmallImageBitmaps;

TentaclePlotter::TentaclePlotter(IGoomDraw& draw,
                                 IGoomRand& goomRand,
                                 const SmallImageBitmaps& smallBitmaps) noexcept
  : m_draw{draw},
    m_goomRand{goomRand},
    m_halfScreenWidth{static_cast<int32_t>(m_draw.GetScreenWidth() / 2)},
    m_halfScreenHeight{static_cast<int32_t>(m_draw.GetScreenHeight() / 2)},
    m_dotDrawer{
        m_draw,
        m_goomRand,
        smallBitmaps,
          // clang-format off
          // min dot sizes
          {
              m_goomRand,
              {
                  {1, 100},
                  {3, 50},
                  {5, 5},
                  {7, 100},
              }
          },
          // normal dot sizes
          {
              m_goomRand,
              {
                  { 1, 50},
                  { 3, 20},
                  { 5, 10},
                  { 7, 10},
                  { 9, 10},
                  {11, 10},
                  {13,  1},
                  {15,  1},
              }
          }
          // clang-format on
    }
{
}

void TentaclePlotter::ChangeDotSizes()
{
  m_dotDrawer.ChangeDotSizes();
}

void TentaclePlotter::ChangeNumNodesBetweenDots()
{
  m_numNodesBetweenDots =
      m_goomRand.GetRandInRange(MIN_STEPS_BETWEEN_NODES, MAX_STEPS_BETWEEN_NODES + 1U);
}

constexpr int COORD_IGNORE_VAL = -666;

void TentaclePlotter::Plot3D(const Tentacle3D& tentacle)
{
  const float brightness = GetBrightness(tentacle);
  const std::vector<V2dInt> points2D = Get2DTentaclePoints(tentacle);

  for (size_t nodeNum = 0; nodeNum < (points2D.size() - 1); ++nodeNum)
  {
    const int32_t ix0 = points2D[nodeNum].x;
    const int32_t ix1 = points2D[nodeNum + 1].x;
    const int32_t iy0 = points2D[nodeNum].y;
    const int32_t iy1 = points2D[nodeNum + 1].y;

    if (((ix0 == COORD_IGNORE_VAL) && (iy0 == COORD_IGNORE_VAL)) ||
        ((ix1 == COORD_IGNORE_VAL) && (iy1 == COORD_IGNORE_VAL)))
    {
      continue;
    }
    if ((ix0 == ix1) && (iy0 == iy1))
    {
      continue;
    }

    DrawNode(tentacle, nodeNum, ix0, iy0, ix1, iy1, brightness);
  }
}

inline void TentaclePlotter::DrawNode(const Tentacle3D& tentacle,
                                      const size_t nodeNum,
                                      const int32_t x0,
                                      const int32_t y0,
                                      const int32_t x1,
                                      const int32_t y1,
                                      const float brightness)
{
  const std::vector<Pixel> colors = GetMixedColors(tentacle, brightness, nodeNum);
  DrawNodeLine(x0, y0, x1, y1, colors);
  DrawNodeDot(nodeNum, x1, y1, colors);
}

inline void TentaclePlotter::DrawNodeLine(const int32_t x0,
                                          const int32_t y0,
                                          const int32_t x1,
                                          const int32_t y1,
                                          const std::vector<Pixel>& colors)
{
  constexpr uint8_t THICKNESS = 1;
  m_draw.Line(x0, y0, x1, y1, colors, THICKNESS);
}

inline void TentaclePlotter::DrawNodeDot(const size_t nodeNum,
                                         const int32_t x,
                                         const int32_t y,
                                         const std::vector<Pixel>& colors)
{
  if ((nodeNum % m_numNodesBetweenDots) != 0)
  {
    return;
  }

  constexpr float DOT_BRIGHTNESS = 1.5F;
  m_dotDrawer.DrawDots({x, y}, colors, DOT_BRIGHTNESS);
}

inline auto TentaclePlotter::Get2DTentaclePoints(const Tentacle3D& tentacle) const
    -> std::vector<V2dInt>
{
  const std::vector<V3dFlt> vertices = tentacle.GetVertices();
  const float angleAboutY = GetTentacleAngleAboutY(tentacle);

  const std::vector<V3dFlt> points3D =
      GetTransformedPoints(vertices, m_cameraPosition, m_pi - angleAboutY);

  return GetPerspectiveProjection(points3D);
}

inline auto TentaclePlotter::GetTentacleAngleAboutY(const Tentacle3D& tentacle) const -> float
{
  constexpr float HEAD_CUTOFF = 10.0F;
  constexpr float ANGLE_ADJ_FRACTION = 0.05F;
  float angleAboutY = m_tentacleAngle;
  if ((-HEAD_CUTOFF < tentacle.GetHead().x) && (tentacle.GetHead().x < 0.0F))
  {
    angleAboutY -= ANGLE_ADJ_FRACTION * m_pi;
  }
  else if ((0.0F <= tentacle.GetHead().x) && (tentacle.GetHead().x < HEAD_CUTOFF))
  {
    angleAboutY += ANGLE_ADJ_FRACTION * m_pi;
  }

  return angleAboutY;
}

void TentaclePlotter::SetCameraPosition(const float cameraDistance, const float tentacleAngle)
{
  m_tentacleAngle = tentacleAngle;
  m_cameraDistance = cameraDistance;

  m_cameraPosition = {0.0F, 0.0F, -3.0F}; // TODO ????????????????????????????????
  m_cameraPosition.z += m_cameraDistance;
  m_cameraPosition.y += 2.0F * std::sin(-(m_tentacleAngle - m_half_pi) / 4.3F);
}

inline auto TentaclePlotter::GetTransformedPoints(const std::vector<V3dFlt>& points,
                                                  const V3dFlt& translate,
                                                  const float angle) -> std::vector<V3dFlt>
{
  const float sinAngle = std::sin(angle);
  const float cosAngle = std::cos(angle);

  std::vector<V3dFlt> transformedPoints{points};

  for (auto& transformedPoint : transformedPoints)
  {
    RotateAboutYAxis(sinAngle, cosAngle, transformedPoint, transformedPoint);
    Translate(translate, transformedPoint);
  }

  return transformedPoints;
}

inline auto TentaclePlotter::GetMixedColors(const Tentacle3D& tentacle,
                                            const float brightness,
                                            const size_t nodeNum) const -> std::vector<Pixel>
{
  const std::pair<Pixel, Pixel> colors =
      tentacle.GetMixedColors(nodeNum, m_dominantColor, m_dominantLowColor, brightness);

  return {colors.first, colors.second};
}

inline auto TentaclePlotter::GetBrightness(const Tentacle3D& tentacle) const -> float
{
  // Faraway tentacles get smaller and draw_line adds them together making them look
  // very white and over-exposed. If we reduce the brightness, then all the combined
  // tentacles look less bright and white and more colors show through.

  constexpr float BRIGHTNESS = 2.0F;
  constexpr float BRIGHTNESS_DISTANCE_CUTOFF = 30.F;
  const float brightnessCut = GetBrightnessCut(tentacle);

  if (m_cameraDistance <= BRIGHTNESS_DISTANCE_CUTOFF)
  {
    return brightnessCut * BRIGHTNESS;
  }

  constexpr float FAR_AWAY_DISTANCE = 50.0F;
  const float farAwayBrightness = FAR_AWAY_DISTANCE / m_cameraDistance;
  constexpr float MIN_RAND_BRIGHTNESS = 0.1F;
  constexpr float MAX_RAND_BRIGHTNESS = 0.3F;
  const float randBrightness = m_goomRand.GetRandInRange(MIN_RAND_BRIGHTNESS, MAX_RAND_BRIGHTNESS);
  return brightnessCut * std::max(randBrightness, farAwayBrightness);
}

inline auto TentaclePlotter::GetBrightnessCut(const Tentacle3D& tentacle) const -> float
{
  if (std::abs(tentacle.GetHead().x) < 10)
  {
    if (m_cameraDistance < 8)
    {
      return 0.5F;
    }
    return 0.2F;
  }
  return 1.0F;
}

auto TentaclePlotter::GetPerspectiveProjection(const std::vector<V3dFlt>& points3D) const
    -> std::vector<V2dInt>
{
  std::vector<V2dInt> points2D(points3D.size());

  const int32_t xProj0 =
      points3D[0].ignore || (points3D[0].z <= 2)
          ? 1
          : static_cast<int32_t>((m_projectionDistance * points3D[0].x) / points3D[0].z);
  const int32_t xProjN =
      points3D[points3D.size() - 1].ignore || (points3D[points3D.size() - 1].z <= 2)
          ? 1
          : static_cast<int32_t>((m_projectionDistance * points3D[points3D.size() - 1].x) /
                                 points3D[points3D.size() - 1].z);

  const float xSpread = std::min(1.0F, std::abs(static_cast<float>(xProj0 - xProjN)) / 10.0F);

  for (size_t i = 0; i < points3D.size(); ++i)
  {
    if ((!points3D[i].ignore) && (points3D[i].z > 2))
    {
      const auto xProj =
          static_cast<int32_t>((xSpread * m_projectionDistance * points3D[i].x) / points3D[i].z);
      const auto yProj =
          static_cast<int32_t>((xSpread * m_projectionDistance * points3D[i].y) / points3D[i].z);
      points2D[i].x = xProj + m_halfScreenWidth;
      points2D[i].y = -yProj + m_halfScreenHeight;
    }
    else
    {
      points2D[i].x = COORD_IGNORE_VAL;
      points2D[i].y = COORD_IGNORE_VAL;
    }
  }

  return points2D;
}

inline void TentaclePlotter::RotateAboutYAxis(const float sinAngle,
                                              const float cosAngle,
                                              const V3dFlt& vSrc,
                                              V3dFlt& vDest)
{
  const float vi_x = vSrc.x;
  const float vi_z = vSrc.z;
  vDest.x = (vi_x * cosAngle) - (vi_z * sinAngle);
  vDest.z = (vi_x * sinAngle) + (vi_z * cosAngle);
  vDest.y = vSrc.y;
}

inline void TentaclePlotter::Translate(const V3dFlt& vAdd, V3dFlt& vInOut)
{
  vInOut.x += vAdd.x;
  vInOut.y += vAdd.y;
  vInOut.z += vAdd.z;
}

#if __cplusplus <= 201402L
} // namespace TENTACLES
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::TENTACLES
#endif
