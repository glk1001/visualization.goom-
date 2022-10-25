#include "tentacle_plotter.h"

//#undef NO_LOGGING

#include "draw/goom_draw.h"
#include "logging.h"
#include "point2d.h"
#include "utils/graphics/line_clipper.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"

namespace GOOM::VISUAL_FX::TENTACLES
{

using DRAW::IGoomDraw;
using DRAW::MultiplePixels;
using UTILS::Logging; // NOLINT(misc-unused-using-decls)
using UTILS::GRAPHICS::LineClipper;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::SMALL_FLOAT;

static constexpr auto BRIGHTNESS                   = 3.0F;
static constexpr auto NORMAL_BRIGHTNESS_CUT        = 1.0F;
static constexpr auto AT_START_HEAD_BRIGHTNESS_CUT = 0.2F;

static constexpr auto COORD_IGNORE_VAL    = -1000.0F;
static constexpr auto PROJECTION_DISTANCE = 170.0F;

TentaclePlotter::TentaclePlotter(IGoomDraw& draw, const IGoomRand& goomRand) noexcept
  : m_draw{draw}, m_goomRand{goomRand}
{
}

auto TentaclePlotter::Plot3D(const Tentacle3D& tentacle) noexcept -> void
{
  const auto brightness = GetBrightness(tentacle);
  const auto points3D   = tentacle.GetTentacleVertices(m_cameraPosition);

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
    const auto pointFlt1 = points2D[nodeNum];
    const auto pointFlt2 = points2D[nodeNum + 1];

    if ((pointFlt1.x <= COORD_IGNORE_VAL) or (pointFlt2.x <= COORD_IGNORE_VAL))
    {
      continue;
    }

    const auto clippedLine = m_lineClipper.GetClippedLine({pointFlt1, pointFlt2});
    if (clippedLine.clipResult == LineClipper::ClipResult::REJECTED)
    {
      continue;
    }

    const auto point1 = clippedLine.line.point1.ToInt();
    const auto point2 = clippedLine.line.point2.ToInt();
    if ((point1.x == point2.x) and (point1.y == point2.y))
    {
      continue;
    }

    DrawNode(tentacle, nodeNum, point1, point2, brightness);
  }
}

inline auto TentaclePlotter::DrawNode(const Tentacle3D& tentacle,
                                      const size_t nodeNum,
                                      const Point2dInt& point1,
                                      const Point2dInt& point2,
                                      const float brightness) -> void
{
  const auto colors = tentacle.GetMixedColors(nodeNum, m_dominantColors, brightness);

  m_draw.Line(point1, point2, colors, m_lineThickness);
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
    -> std::vector<Point2dFlt>
{
  static constexpr auto MIN_Z = 2.0F;

  const auto numPoints = points3D.size();
  auto points2D        = std::vector<Point2dFlt>(numPoints);

  for (auto i = 0U; i < numPoints; ++i)
  {
    if (points3D[i].z <= MIN_Z)
    {
      points2D[i].x = COORD_IGNORE_VAL - SMALL_FLOAT;
      points2D[i].y = COORD_IGNORE_VAL - SMALL_FLOAT;
    }
    else
    {
      const auto perspectiveFactor = PROJECTION_DISTANCE / points3D[i].z;

      const auto xProj = perspectiveFactor * points3D[i].x;
      const auto yProj = perspectiveFactor * points3D[i].y;

      points2D[i] = Point2dFlt{xProj, -yProj} + m_screenMidPoint;
    }
  }

  return points2D;
}

} // namespace GOOM::VISUAL_FX::TENTACLES
