#include "tentacle_plotter.h"

#include "draw/goom_draw.h"
#include "point2d.h"

namespace GOOM::VISUAL_FX::TENTACLES
{

using COLOR::IColorMap;
using DRAW::IGoomDraw;
using DRAW::MultiplePixels;
using FX_UTILS::DotSizes;
using STD20::pi;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::HALF_PI;
using UTILS::MATH::IGoomRand;

// clang-format off
static constexpr auto MIN_DOT_SIZE01_WEIGHT    = 150.0F;
static constexpr auto MIN_DOT_SIZE02_WEIGHT    =  50.0F;

static constexpr auto NORMAL_DOT_SIZE01_WEIGHT =  80.0F;
static constexpr auto NORMAL_DOT_SIZE02_WEIGHT =  20.0F;
static constexpr auto NORMAL_DOT_SIZE03_WEIGHT =  10.0F;
// clang-format on

TentaclePlotter::TentaclePlotter(IGoomDraw& draw,
                                 const IGoomRand& goomRand,
                                 const SmallImageBitmaps& smallBitmaps) noexcept
  : m_draw{draw},
    m_goomRand{goomRand},
    m_dotDrawer{
        m_draw,
        m_goomRand,
        smallBitmaps,
          // clang-format off
          // min dot sizes
          {
              m_goomRand,
              {
                  {DotSizes::DOT_SIZE01, MIN_DOT_SIZE01_WEIGHT},
                  {DotSizes::DOT_SIZE02, MIN_DOT_SIZE02_WEIGHT},
              }
          },
          // normal dot sizes
          {
              m_goomRand,
              {
                  {DotSizes::DOT_SIZE01, NORMAL_DOT_SIZE01_WEIGHT},
                  {DotSizes::DOT_SIZE02, NORMAL_DOT_SIZE02_WEIGHT},
                  {DotSizes::DOT_SIZE03, NORMAL_DOT_SIZE03_WEIGHT},
              }
          }
          // clang-format on
    }
{
  m_dotDrawer.ChangeDotSizes();
}

auto TentaclePlotter::ChangeDotSizes() -> void
{
  m_dotDrawer.ChangeDotSizes();
}

auto TentaclePlotter::ChangeNumNodesBetweenDots() -> void
{
  m_numNodesBetweenDots =
      m_goomRand.GetRandInRange(MIN_STEPS_BETWEEN_NODES, MAX_STEPS_BETWEEN_NODES + 1U);
}

static constexpr auto COORD_IGNORE_VAL = -666;

auto TentaclePlotter::Plot3D(const Tentacle3D& tentacle) -> void
{
  const auto brightness = GetBrightness(tentacle);
  const auto points2D = Get2DTentaclePoints(tentacle);

  for (auto nodeNum = 0U; nodeNum < (points2D.size() - 1); ++nodeNum)
  {
    const auto point1 = points2D[nodeNum];
    const auto point2 = points2D[nodeNum + 1];

    if (((point1.x == COORD_IGNORE_VAL) && (point1.y == COORD_IGNORE_VAL)) ||
        ((point2.x == COORD_IGNORE_VAL) && (point2.y == COORD_IGNORE_VAL)))
    {
      continue;
    }
    if ((point1.x == point2.x) && (point1.y == point2.y))
    {
      continue;
    }

    DrawNode(tentacle, nodeNum, point1, point2, brightness);
  }
}

inline auto TentaclePlotter::DrawNode(const Tentacle3D& tentacle,
                                      const size_t nodeNum,
                                      const Point2dInt point1,
                                      const Point2dInt point2,
                                      const float brightness) -> void
{
  const auto colors = GetMixedColors(tentacle, brightness, nodeNum);
  DrawNodeLine(point1, point2, colors);
  DrawNodeDot(nodeNum, point2, colors);
}

inline auto TentaclePlotter::DrawNodeLine(const Point2dInt point1,
                                          const Point2dInt point2,
                                          const MultiplePixels& colors) -> void
{
  static constexpr auto THICKNESS = 1U;
  m_draw.Line(point1, point2, colors, THICKNESS);
}

inline auto TentaclePlotter::DrawNodeDot(const size_t nodeNum,
                                         const Point2dInt point,
                                         const MultiplePixels& colors) -> void
{
  if ((nodeNum % m_numNodesBetweenDots) != 0)
  {
    return;
  }

  const auto dotColors = MultiplePixels{
      IColorMap::GetColorMix(colors[0], m_dominantDotColor, 0.5F),
      IColorMap::GetColorMix(colors.at(1), m_dominantDotColor, 0.5F),
  };
  static constexpr auto DOT_BRIGHTNESS = 1.5F;
  m_dotDrawer.DrawDot(point, dotColors, DOT_BRIGHTNESS);
}

auto TentaclePlotter::Get2DTentaclePoints(const Tentacle3D& tentacle) const
    -> std::vector<Point2dInt>
{
  const auto vertices = tentacle.GetVertices();
  const auto angleAboutY = GetTentacleAngleAboutY(tentacle);

  const auto points3D = GetTransformedPoints(vertices, m_cameraPosition, pi - angleAboutY);

  return GetPerspectiveProjection(points3D);
}

inline auto TentaclePlotter::GetTentacleAngleAboutY(const Tentacle3D& tentacle) const -> float
{
  static constexpr auto HEAD_CUTOFF = 10.0F;
  static constexpr auto ANGLE_ADJ_FRACTION = 0.05F;
  auto angleAboutY = m_tentacleAngle;
  if ((-HEAD_CUTOFF < tentacle.GetHead().x) && (tentacle.GetHead().x < 0.0F))
  {
    angleAboutY -= ANGLE_ADJ_FRACTION * pi;
  }
  else if ((0.0F <= tentacle.GetHead().x) && (tentacle.GetHead().x < HEAD_CUTOFF))
  {
    angleAboutY += ANGLE_ADJ_FRACTION * pi;
  }

  return angleAboutY;
}

auto TentaclePlotter::SetCameraPosition(const float cameraDistance, const float tentacleAngle)
    -> void
{
  m_tentacleAngle = tentacleAngle;
  m_cameraDistance = cameraDistance;

  static constexpr auto CAMERA_Z_OFFSET = -3.0F;
  m_cameraPosition = {0.0F, 0.0F, CAMERA_Z_OFFSET}; // TODO ????????????????????????????????
  m_cameraPosition.z += m_cameraDistance;
  static constexpr auto ANGLE_FACTOR = 1.0F / 4.3F;
  static constexpr auto CAMERA_POS_FACTOR = 2.0F;
  m_cameraPosition.y += CAMERA_POS_FACTOR * std::sin(-ANGLE_FACTOR * (m_tentacleAngle - HALF_PI));
}

inline auto TentaclePlotter::GetTransformedPoints(const std::vector<V3dFlt>& points,
                                                  const V3dFlt& translate,
                                                  const float angle) -> std::vector<V3dFlt>
{
  const auto sinAngle = std::sin(angle);
  const auto cosAngle = std::cos(angle);

  auto transformedPoints = points;

  for (auto& transformedPoint : transformedPoints)
  {
    RotateAboutYAxis(sinAngle, cosAngle, transformedPoint, transformedPoint);
    Translate(translate, transformedPoint);
  }

  return transformedPoints;
}

inline auto TentaclePlotter::GetMixedColors(const Tentacle3D& tentacle,
                                            const float brightness,
                                            const size_t nodeNum) const -> MultiplePixels
{
  const auto [mainColor, lowColor] =
      tentacle.GetMixedColors(nodeNum, m_dominantMainColor, m_dominantLowColor, brightness);

  return {mainColor, lowColor};
}

inline auto TentaclePlotter::GetBrightness(const Tentacle3D& tentacle) const -> float
{
  // Faraway tentacles get smaller and draw_line adds them together making them look
  // very white and over-exposed. If we reduce the brightness, then all the combined
  // tentacles look less bright and white and more colors show through.

  static constexpr auto BRIGHTNESS = 3.0F;
  static constexpr auto BRIGHTNESS_DISTANCE_CUTOFF = 30.F;
  const auto brightnessCut = GetBrightnessCut(tentacle);

  if (m_cameraDistance <= BRIGHTNESS_DISTANCE_CUTOFF)
  {
    return brightnessCut * BRIGHTNESS;
  }

  static constexpr auto FAR_AWAY_DISTANCE = 50.0F;
  const auto farAwayBrightness = FAR_AWAY_DISTANCE / m_cameraDistance;
  static constexpr auto MIN_RAND_BRIGHTNESS = 0.1F;
  static constexpr auto MAX_RAND_BRIGHTNESS = 0.3F;
  const auto randBrightness = m_goomRand.GetRandInRange(MIN_RAND_BRIGHTNESS, MAX_RAND_BRIGHTNESS);
  return brightnessCut * std::max(randBrightness, farAwayBrightness);
}

inline auto TentaclePlotter::GetBrightnessCut(const Tentacle3D& tentacle) const -> float
{
  static constexpr auto IN_HEAD_CLOSE_CAMERA_BRIGHTNESS_CUT = 0.5F;
  static constexpr auto IN_HEAD_BRIGHTNESS_CUT = 0.2F;
  static constexpr auto NORMAL_BRIGHTNESS_CUT = 1.0F;

  if (std::abs(tentacle.GetHead().x) < Tentacle3D::HEAD_SMALL_X)
  {
    if (static constexpr auto CLOSE_CAMERA_DISTANCE = 8.0F;
        m_cameraDistance < CLOSE_CAMERA_DISTANCE)
    {
      return IN_HEAD_CLOSE_CAMERA_BRIGHTNESS_CUT;
    }
    return IN_HEAD_BRIGHTNESS_CUT;
  }
  return NORMAL_BRIGHTNESS_CUT;
}

auto TentaclePlotter::GetPerspectiveProjection(const std::vector<V3dFlt>& points3D) const
    -> std::vector<Point2dInt>
{
  auto points2D = std::vector<Point2dInt>(points3D.size());

  const auto xProj0 =
      points3D[0].ignore || (points3D[0].z <= 2)
          ? 1
          : static_cast<int32_t>((m_projectionDistance * points3D[0].x) / points3D[0].z);
  const auto xProjN =
      points3D[points3D.size() - 1].ignore || (points3D[points3D.size() - 1].z <= 2)
          ? 1
          : static_cast<int32_t>((m_projectionDistance * points3D[points3D.size() - 1].x) /
                                 points3D[points3D.size() - 1].z);

  const auto xSpread = std::min(1.0F, std::abs(static_cast<float>(xProj0 - xProjN)) / 10.0F);

  for (auto i = 0U; i < points3D.size(); ++i)
  {
    static constexpr auto MIN_Z = 2.0F;
    if ((!points3D[i].ignore) && (points3D[i].z > MIN_Z))
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

inline auto TentaclePlotter::RotateAboutYAxis(const float sinAngle,
                                              const float cosAngle,
                                              const V3dFlt& srcPoint,
                                              V3dFlt& destPoint) -> void
{
  const auto srcX = srcPoint.x;
  const auto srcZ = srcPoint.z;
  destPoint.x = (srcX * cosAngle) - (srcZ * sinAngle);
  destPoint.z = (srcX * sinAngle) + (srcZ * cosAngle);
  destPoint.y = srcPoint.y;
}

inline auto TentaclePlotter::Translate(const V3dFlt& vAdd, V3dFlt& vInOut) -> void
{
  vInOut.x += vAdd.x;
  vInOut.y += vAdd.y;
  vInOut.z += vAdd.z;
}

} // namespace GOOM::VISUAL_FX::TENTACLES
