module;

#include "draw/goom_draw.h"
#include "draw/shape_drawers/circle_drawer.h"
#include "draw/shape_drawers/line_drawer.h"
#include "goom/goom_config.h"
#include "goom/goom_types.h"
#include "goom/point2d.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <vector>

module Goom.VisualFx.TentaclesFx:TentaclePlotter;

import Goom.Utils.Graphics.LineClipper;
import Goom.Utils.Math.TValues;
import Goom.Utils.Math.GoomRandBase;
import :Tentacle3d;

namespace GOOM::VISUAL_FX::TENTACLES
{

class TentaclePlotter
{
public:
  TentaclePlotter() noexcept = delete;
  TentaclePlotter(DRAW::IGoomDraw& draw, const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto UpdateCameraPosition() noexcept -> void;
  auto SetTentacleLineThickness(uint8_t lineThickness) noexcept -> void;

  using GetColorsFunc = std::function<DRAW::MultiplePixels(float nodeT)>;
  auto SetGetColorsFunc(const GetColorsFunc& getColorsFunc) noexcept -> void;
  auto SetEndDotColors(const DRAW::MultiplePixels& endDotColors) noexcept -> void;
  auto SetNodeTOffset(float value) noexcept -> void;

  auto Plot3D(const Tentacle3D& tentacle) noexcept -> void;

private:
  DRAW::IGoomDraw* m_draw;
  const UTILS::MATH::IGoomRand* m_goomRand;
  Vec2dFlt m_screenCentre = ToVec2dFlt(m_draw->GetDimensions().GetCentrePoint());

  DRAW::SHAPE_DRAWERS::LineDrawerNoClippedEndPoints m_lineDrawer{*m_draw};
  DRAW::SHAPE_DRAWERS::CircleDrawer m_circleDrawer{*m_draw};

  using LineClipper = UTILS::GRAPHICS::LineClipper;
  LineClipper m_lineClipper{GetLineClipRectangle(1U)};
  [[nodiscard]] auto GetLineClipRectangle(uint8_t lineThickness) const noexcept -> Rectangle2dInt;

  GetColorsFunc m_getColors{};
  DRAW::MultiplePixels m_endDotColors{};
  float m_nodeTOffset = 0.0F;

  static constexpr auto PROJECTION_DISTANCE = 170.0F;
  static constexpr auto MIN_CAMERA_X_OFFSET = -10.0F;
  static constexpr auto MAX_CAMERA_X_OFFSET = +10.0F;
  static constexpr auto MIN_CAMERA_Y_OFFSET = -10.0F;
  static constexpr auto MAX_CAMERA_Y_OFFSET = +10.0F;
  static constexpr auto MIN_CAMERA_Z_OFFSET = +04.0F; // Don't make this any smaller
  static constexpr auto MAX_CAMERA_Z_OFFSET = +10.1F;
  static_assert(MIN_CAMERA_X_OFFSET < MAX_CAMERA_X_OFFSET);
  static_assert(MIN_CAMERA_Y_OFFSET < MAX_CAMERA_Y_OFFSET);
  static_assert(MIN_CAMERA_Z_OFFSET < MAX_CAMERA_Z_OFFSET);
  V3dFlt m_cameraPosition{0.0F, 0.0F, MIN_CAMERA_Z_OFFSET};

  auto PlotPoints(const std::vector<V3dFlt>& points3D) -> void;
  struct Line2DInt
  {
    Point2dInt point1;
    Point2dInt point2;
  };
  [[nodiscard]] auto GetPerspectiveProjection(const std::vector<V3dFlt>& points3D) const
      -> std::vector<Line2DInt>;
  [[nodiscard]] auto GetPerspectivePoint(const V3dFlt& point3D) const -> Point2dFlt;
  [[nodiscard]] static auto GetLine2D(const Point2dFlt& point1Flt,
                                      const Point2dFlt& point2Flt) noexcept -> Line2DInt;
  struct Line3DFlt
  {
    V3dFlt point1;
    V3dFlt point2;
  };
  [[nodiscard]] static auto GetLines3D(const std::vector<V3dFlt>& points3D)
      -> std::vector<Line3DFlt>;
};

inline auto TentaclePlotter::SetGetColorsFunc(const GetColorsFunc& getColorsFunc) noexcept -> void
{
  m_getColors = getColorsFunc;
}

inline auto TentaclePlotter::SetEndDotColors(const DRAW::MultiplePixels& endDotColors) noexcept
    -> void
{
  m_endDotColors = endDotColors;
}

inline auto TentaclePlotter::SetNodeTOffset(const float value) noexcept -> void
{
  m_nodeTOffset = value;
}

inline auto TentaclePlotter::UpdateCameraPosition() noexcept -> void
{
  m_cameraPosition = {m_goomRand->GetRandInRange(MIN_CAMERA_X_OFFSET, MAX_CAMERA_X_OFFSET),
                      m_goomRand->GetRandInRange(MIN_CAMERA_Y_OFFSET, MAX_CAMERA_Y_OFFSET),
                      m_goomRand->GetRandInRange(MIN_CAMERA_Z_OFFSET, MAX_CAMERA_Z_OFFSET)};
}

} // namespace GOOM::VISUAL_FX::TENTACLES

namespace GOOM::VISUAL_FX::TENTACLES
{

using DRAW::IGoomDraw;
using UTILS::MATH::TValue;
using UTILS::MATH::IGoomRand;

TentaclePlotter::TentaclePlotter(IGoomDraw& draw, const IGoomRand& goomRand) noexcept
  : m_draw{&draw}, m_goomRand{&goomRand}
{
}

auto TentaclePlotter::SetTentacleLineThickness(const uint8_t lineThickness) noexcept -> void
{
  m_lineDrawer.SetLineThickness(lineThickness);
  m_lineClipper.SetClipRectangle(GetLineClipRectangle(lineThickness));
}

inline auto TentaclePlotter::GetLineClipRectangle(const uint8_t lineThickness) const noexcept
    -> Rectangle2dInt
{
  const auto clipMargin  = static_cast<int32_t>(lineThickness + 1U);
  const auto topLeft     = Point2dInt{clipMargin, clipMargin};
  const auto bottomRight = Point2dInt{m_draw->GetDimensions().GetIntWidth() - clipMargin,
                                      m_draw->GetDimensions().GetIntHeight() - clipMargin};

  return {topLeft, bottomRight};
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

  auto nodeT = TValue{
      {TValue::StepType::CONTINUOUS_REVERSIBLE, numNodes, m_nodeTOffset}
  };
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

  return Point2dFlt{xProj, -yProj} + m_screenCentre;
}

inline auto TentaclePlotter::GetLines3D(const std::vector<V3dFlt>& points3D)
    -> std::vector<Line3DFlt>
{
  auto lines3D = std::vector<Line3DFlt>{};
  if (points3D.size() < 2)
  {
    return lines3D;
  }

  const auto numPointsMinus1 = points3D.size() - 1;
  for (auto i = 0U; i < numPointsMinus1; ++i)
  {
    lines3D.emplace_back(Line3DFlt{points3D[i], points3D[i + 1]});
  }

  return lines3D;
}

inline auto TentaclePlotter::GetLine2D(const Point2dFlt& point1Flt,
                                       const Point2dFlt& point2Flt) noexcept -> Line2DInt
{
  auto line2D = Line2DInt{ToPoint2dInt(point1Flt), ToPoint2dInt(point2Flt)};

  if (line2D.point1 == line2D.point2)
  {
    return line2D;
  }

  // TODO(glk) - What about the last line??
  // We are drawing joined lines and we don't want to re-plot the last
  // point of the previous line. So back up one pixel from 'point2'.
  const auto dx = std::clamp(line2D.point2.x - line2D.point1.x, -1, +1);
  const auto dy = std::clamp(line2D.point2.y - line2D.point1.y, -1, +1);
  line2D.point2.x -= dx;
  line2D.point2.y -= dy;

  return line2D;
}

} // namespace GOOM::VISUAL_FX::TENTACLES
