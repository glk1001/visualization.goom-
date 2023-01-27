//#undef NO_LOGGING

#include "tentacle_plotter.h"

#include "draw/goom_draw.h"
#include "goom_config.h"
#include "goom_logger.h"
#include "point2d.h"
#include "utils/debugging_logger.h"
#include "utils/graphics/line_clipper.h"
#include "utils/math/goom_rand_base.h"
#include "utils/t_values.h"

namespace GOOM::VISUAL_FX::TENTACLES
{

using DRAW::IGoomDraw;
using DRAW::MultiplePixels;
using UTILS::TValue;
using UTILS::MATH::IGoomRand;

TentaclePlotter::TentaclePlotter(IGoomDraw& draw, const IGoomRand& goomRand) noexcept
  : m_draw{draw}, m_goomRand{goomRand}
{
}

auto TentaclePlotter::SetTentacleLineThickness(const uint8_t lineThickness) noexcept -> void
{
  m_lineDrawer.SetLineThickness(lineThickness);
  m_lineClipper.SetClipRectangle(GetLineClipRectangle(lineThickness));
}

inline auto TentaclePlotter::GetLineClipRectangle(const uint8_t lineThickness) const noexcept
    -> LineClipper::ClipRectangle
{
  const auto clipMargin = lineThickness + 1U;
  return {clipMargin,
          clipMargin,
          m_draw.GetDimensions().GetWidth() - clipMargin,
          m_draw.GetDimensions().GetHeight() - clipMargin};
}

auto TentaclePlotter::Plot3D(const Tentacle3D& tentacle) noexcept -> void
{
  const auto points3D = tentacle.GetTentacleVertices(m_cameraPosition);

  PlotPoints(points3D);
}

inline auto TentaclePlotter::PlotPoints(const std::vector<V3dFlt>& points3D) -> void
{
  const auto lines2D = GetPerspectiveProjection(points3D);

  const auto numNodes = static_cast<uint32_t>(lines2D.size());
  if (0 == numNodes)
  {
    return;
  }

  auto nodeT = TValue{TValue::StepType::CONTINUOUS_REVERSIBLE, numNodes, m_nodeTOffset};
  for (const auto& line : lines2D)
  {
    const auto colors = m_getColors(nodeT());
    m_lineDrawer.DrawLine(line.point1, line.point2, colors);
    nodeT.Increment();
  }

  static constexpr auto END_DOT_RADIUS = 1;
  m_circleDrawer.DrawFilledCircle(lines2D.back().point2, END_DOT_RADIUS, m_endDotColors);
}

auto TentaclePlotter::GetPerspectiveProjection(const std::vector<V3dFlt>& points3D) const
    -> std::vector<Line2DInt>
{
  const auto lines3D = GetLines3D(points3D);

  auto lines2D = std::vector<Line2DInt>{};
  for (const auto& line3D : lines3D)
  {
    if (static constexpr auto MIN_Z = 2.0F;
        (line3D.point1.z <= MIN_Z) or (line3D.point2.z <= MIN_Z))
    {
      continue;
    }

    const auto pointFlt1 = GetPerspectivePoint(line3D.point1);
    const auto pointFlt2 = GetPerspectivePoint(line3D.point2);

    const auto clippedLine = m_lineClipper.GetClippedLine({pointFlt1, pointFlt2});
    if (clippedLine.clipResult == LineClipper::ClipResult::REJECTED)
    {
      continue;
    }

    const auto line2D = GetLine2D(clippedLine.line.point1, clippedLine.line.point2);

    lines2D.emplace_back(line2D);
  }

  return lines2D;
}

inline auto TentaclePlotter::GetPerspectivePoint(const V3dFlt& point3D) const -> Point2dFlt
{
  const auto perspectiveFactor = PROJECTION_DISTANCE / point3D.z;
  const auto xProj             = perspectiveFactor * point3D.x;
  const auto yProj             = perspectiveFactor * point3D.y;

  return Point2dFlt{xProj, -yProj} + m_screenMidPoint;
}

inline auto TentaclePlotter::GetLines3D(const std::vector<V3dFlt>& points3D)
    -> std::vector<Line3DFlt>
{
  const auto numPoints = points3D.size();

  auto lines3D     = std::vector<Line3DFlt>{};
  auto point3DFlt1 = points3D[0];
  for (auto i = 1U; i < numPoints; ++i)
  {
    const auto point3DFlt2 = points3D[i];
    lines3D.emplace_back(Line3DFlt{point3DFlt1, point3DFlt2});
    point3DFlt1 = point3DFlt2;
  }

  return lines3D;
}

inline auto TentaclePlotter::GetLine2D(const Point2dFlt& point1Flt,
                                       const Point2dFlt& point2Flt) noexcept -> Line2DInt
{
  auto line2D = Line2DInt{point1Flt.ToInt(), point2Flt.ToInt()};

  if (line2D.point1 == line2D.point2)
  {
    return line2D;
  }

  //TODO(glk) - What about the last line??
  // We are drawing joined lines and we don't want to re-plot the last
  // point of the previous line. So back up one pixel from 'point2'.
  const auto dx = std::clamp(line2D.point2.x - line2D.point1.x, -1, +1);
  const auto dy = std::clamp(line2D.point2.y - line2D.point1.y, -1, +1);
  line2D.point2.x -= dx;
  line2D.point2.y -= dy;

  return line2D;
}

} // namespace GOOM::VISUAL_FX::TENTACLES
