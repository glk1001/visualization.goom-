#include "tentacle_plotter.h"

//#undef NO_LOGGING

#include "draw/goom_draw.h"
#include "logging.h"
#include "point2d.h"
#include "utils/math/misc.h"

namespace GOOM::VISUAL_FX::TENTACLES
{

using COLOR::IColorMap;
using DRAW::IGoomDraw;
using DRAW::MultiplePixels;
using FX_UTILS::DotSizes;
using UTILS::Logging; // NOLINT(misc-unused-using-decls)
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::IGoomRand;

static constexpr auto BRIGHTNESS                   = 3.0F;
static constexpr auto DOT_BRIGHTNESS               = 1.5F;
static constexpr auto NORMAL_BRIGHTNESS_CUT        = 1.0F;
static constexpr auto AT_START_HEAD_BRIGHTNESS_CUT = 0.2F;

static constexpr auto COORD_IGNORE_VAL    = -666;
static constexpr auto PROJECTION_DISTANCE = 170.0F;

static constexpr auto MIN_DOT_SIZE01_WEIGHT    = 150.0F;
static constexpr auto MIN_DOT_SIZE02_WEIGHT    = 050.0F;
static constexpr auto NORMAL_DOT_SIZE01_WEIGHT = 80.0F;
static constexpr auto NORMAL_DOT_SIZE02_WEIGHT = 20.0F;
static constexpr auto NORMAL_DOT_SIZE03_WEIGHT = 10.0F;

TentaclePlotter::TentaclePlotter(IGoomDraw& draw,
                                 const IGoomRand& goomRand,
                                 const SmallImageBitmaps& smallBitmaps) noexcept
  : m_draw{draw},
    m_goomRand{goomRand},
    m_dotDrawer{
        m_draw,
        m_goomRand,
        smallBitmaps,
          {
              m_goomRand,
              {
                  {DotSizes::DOT_SIZE01, MIN_DOT_SIZE01_WEIGHT},
                  {DotSizes::DOT_SIZE02, MIN_DOT_SIZE02_WEIGHT},
              }
          },
          {
              m_goomRand,
              {
                  {DotSizes::DOT_SIZE01, NORMAL_DOT_SIZE01_WEIGHT},
                  {DotSizes::DOT_SIZE02, NORMAL_DOT_SIZE02_WEIGHT},
                  {DotSizes::DOT_SIZE03, NORMAL_DOT_SIZE03_WEIGHT},
              }
          }
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

auto TentaclePlotter::Plot3D(const Tentacle3D& tentacle) -> void
{
  const auto brightness = GetBrightness(tentacle);
  const auto points3D   = tentacle.GetTentacleVertices();

  PlotPoints(tentacle, brightness, points3D);
}

inline auto TentaclePlotter::PlotPoints(const Tentacle3D& tentacle,
                                        const float brightness,
                                        const std::vector<V3dFlt>& points3D) -> void
{
  const auto points2D  = GetPerspectiveProjection(points3D);
  const auto numPoints = points2D.size();

  for (auto nodeNum = 0U; nodeNum < (numPoints - 1); ++nodeNum)
  {
    const auto point1 = points2D[nodeNum];
    const auto point2 = points2D[nodeNum + 1];

    if (((point1.x == COORD_IGNORE_VAL) and (point1.y == COORD_IGNORE_VAL)) or
        ((point2.x == COORD_IGNORE_VAL) and (point2.y == COORD_IGNORE_VAL)))
    {
      continue;
    }
    if ((point1.x == point2.x) and (point1.y == point2.y))
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
  const auto colors = tentacle.GetMixedColors(nodeNum, m_dominantColors, brightness);

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

  m_dotDrawer.DrawDot(point, dotColors, DOT_BRIGHTNESS);
}

inline auto TentaclePlotter::GetBrightness(const Tentacle3D& tentacle) -> float
{
  return BRIGHTNESS * GetBrightnessCut(tentacle);
}

inline auto TentaclePlotter::GetBrightnessCut(const Tentacle3D& tentacle) -> float
{
  return NORMAL_BRIGHTNESS_CUT;
  if (std::abs(tentacle.GetStartPos().x) < Tentacle3D::START_SMALL_X)
  {
    return AT_START_HEAD_BRIGHTNESS_CUT;
  }
  return NORMAL_BRIGHTNESS_CUT;
}

auto TentaclePlotter::GetPerspectiveProjection(const std::vector<V3dFlt>& points3D) const
    -> std::vector<Point2dInt>
{
  static constexpr auto MIN_Z = 2.0F;

  const auto numPoints = points3D.size();
  auto points2D        = std::vector<Point2dInt>(numPoints);

  for (auto i = 0U; i < numPoints; ++i)
  {
    if (points3D[i].z <= MIN_Z)
    {
      points2D[i].x = COORD_IGNORE_VAL;
      points2D[i].y = COORD_IGNORE_VAL;
    }
    else
    {
      const auto perspectiveFactor = PROJECTION_DISTANCE / points3D[i].z;

      const auto xProj = static_cast<int32_t>(perspectiveFactor * points3D[i].x);
      const auto yProj = static_cast<int32_t>(perspectiveFactor * points3D[i].y);

      points2D[i] = Point2dInt{xProj, -yProj} + m_screenMidPoint;
    }
  }

  return points2D;
}

} // namespace GOOM::VISUAL_FX::TENTACLES
