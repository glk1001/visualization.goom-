module;

#include "goom/goom_config.h"

#include <algorithm>
#include <cstdint>
#include <lsys/parsed_model.h>
#include <lsys/vector.h>
#include <vector>

module Goom.VisualFx.LSystemFx:LSysDraw;

import Goom.Draw.ShaperDrawers.LineDrawerWithEffects;
import Goom.Lib.Point2d;
import :LSysColors;
import :LSysGeom;

namespace GOOM::VISUAL_FX::L_SYSTEM
{

class LSysDraw
{
public:
  LSysDraw(const LSysGeometry& lSysGeometry,
           LSysColors& lSysColors,
           float lineWidthFactor) noexcept;

  auto SetNumLSysCopies(uint32_t numLSysCopies) noexcept -> void;
  auto SetLineDrawer(DRAW::SHAPE_DRAWERS::ILineDrawerWithEffects& lineDrawer) noexcept -> void;

  auto DrawLine(uint32_t lSysColor,
                const ::LSYS::Vector& point1,
                const ::LSYS::Vector& point2,
                float lineWidth) noexcept -> void;
  auto DrawPolygon(uint32_t lSysColor,
                   const std::vector<::LSYS::Vector>& polygon,
                   float lineWidth) noexcept -> void;

  [[nodiscard]] static auto GetPerspectivePoint(const ::LSYS::Point3dFlt& point3d) noexcept
      -> Point2dFlt;

private:
  const LSysGeometry* m_lSysGeometry;
  LSysColors* m_lSysColors;
  float m_lineWidthFactor;
  uint32_t m_numLSysCopies = 1U;
  DRAW::SHAPE_DRAWERS::ILineDrawerWithEffects* m_lineDrawer{};

  static constexpr auto MAX_LINE_WIDTH = 5.0F;
  auto DrawJoinedVertices(const std::vector<::LSYS::Point3dFlt>& vertices,
                          uint32_t lSysColor) noexcept -> void;
  [[nodiscard]] static auto GetPerspectiveProjection(
      const std::vector<::LSYS::Point3dFlt>& points3d) noexcept -> std::vector<Point2dFlt>;
  [[nodiscard]] static auto GetPolygon3dFlt(const std::vector<::LSYS::Vector>& polygon) noexcept
      -> std::vector<::LSYS::Point3dFlt>;
  [[nodiscard]] static auto GetPoint3dFlt(const ::LSYS::Vector& point) noexcept
      -> ::LSYS::Point3dFlt;
};

} // namespace GOOM::VISUAL_FX::L_SYSTEM

namespace GOOM::VISUAL_FX::L_SYSTEM
{

inline auto LSysDraw::SetLineDrawer(
    DRAW::SHAPE_DRAWERS::ILineDrawerWithEffects& lineDrawer) noexcept -> void
{
  m_lineDrawer = &lineDrawer;
}

using ::LSYS::Point3dFlt;

LSysDraw::LSysDraw(const LSysGeometry& lSysGeometry,
                   LSysColors& lSysColors,
                   const float lineWidthFactor) noexcept
  : m_lSysGeometry{&lSysGeometry}, m_lSysColors{&lSysColors}, m_lineWidthFactor{lineWidthFactor}
{
}

auto LSysDraw::SetNumLSysCopies(const uint32_t numLSysCopies) noexcept -> void
{
  Expects(numLSysCopies > 0U);
  m_numLSysCopies = numLSysCopies;
}

auto LSysDraw::DrawLine(const uint32_t lSysColor,
                        const ::LSYS::Vector& point1,
                        const ::LSYS::Vector& point2,
                        const float lineWidth) noexcept -> void
{
  Expects(m_lineDrawer != nullptr);

  const auto iLineWidth =
      static_cast<uint8_t>(std::clamp(m_lineWidthFactor * lineWidth, 1.0F, MAX_LINE_WIDTH));
  m_lineDrawer->SetLineThickness(iLineWidth);
  m_lSysColors->SetLineWidth(iLineWidth);

  DrawJoinedVertices({GetPoint3dFlt(point1), GetPoint3dFlt(point2)}, lSysColor);
}

auto LSysDraw::DrawPolygon(const uint32_t lSysColor,
                           const std::vector<::LSYS::Vector>& polygon,
                           const float lineWidth) noexcept -> void
{
  Expects(m_lineDrawer != nullptr);

  const auto iLineWidth =
      static_cast<uint8_t>(std::clamp(m_lineWidthFactor * lineWidth, 1.0F, MAX_LINE_WIDTH));
  m_lineDrawer->SetLineThickness(iLineWidth);
  m_lSysColors->SetLineWidth(iLineWidth);

  DrawJoinedVertices(GetPolygon3dFlt(polygon), lSysColor);
}

inline auto LSysDraw::DrawJoinedVertices(const std::vector<Point3dFlt>& vertices,
                                         const uint32_t lSysColor) noexcept -> void
{
  const auto numVertices                 = vertices.size();
  static constexpr auto MIN_NUM_VERTICES = 2U;
  USED_FOR_DEBUGGING(MIN_NUM_VERTICES);
  Expects(numVertices >= MIN_NUM_VERTICES);

  const auto points2d = GetPerspectiveProjection(vertices);
  Expects(points2d.size() == numVertices);

  const auto numPointsMinus1 = numVertices - 1;
  for (auto i = 0U; i < numPointsMinus1; ++i)
  {
    const auto point1 = points2d[i];
    const auto point2 = points2d[i + 1];

    for (auto copyNum = 0U; copyNum < m_numLSysCopies; ++copyNum)
    {
      const auto tPoint1 = m_lSysGeometry->GetTransformedPoint(point1, copyNum);
      const auto tPoint2 = m_lSysGeometry->GetTransformedPoint(point2, copyNum);

      m_lineDrawer->DrawLine(ToPoint2dInt(tPoint1),
                             ToPoint2dInt(tPoint2),
                             m_lSysColors->GetColors(copyNum, lSysColor));
    }

    m_lSysColors->IncrementColorTs();
  }
}

inline auto LSysDraw::GetPerspectiveProjection(const std::vector<Point3dFlt>& points3d) noexcept
    -> std::vector<Point2dFlt>
{
  const auto numPoints = points3d.size();
  auto points2d        = std::vector<Point2dFlt>(numPoints);

  for (auto i = 0U; i < numPoints; ++i)
  {
    points2d[i] = GetPerspectivePoint(points3d[i]);
  }

  return points2d;
}

auto LSysDraw::GetPerspectivePoint(const Point3dFlt& point3d) noexcept -> Point2dFlt
{
  // TODO(glk) Why does this look better?
  return {point3d.x, point3d.y};

  /**
  const auto point = Point3dFlt{point3d.x, point3d.y, point3d.z};

  const auto zOffset = point.z + 10.0F;
  if (std::fabs(zOffset) < UTILS::MATH::SMALL_FLOAT)
  {
    return {0.0F, 0.0F};
  }
  static constexpr auto PROJECTION_DISTANCE = 10.0F;
  const auto perspectiveFactor              = PROJECTION_DISTANCE / zOffset;
  const auto xProj                          = perspectiveFactor * point.x;
  const auto yProj                          = perspectiveFactor * point.y;

  return Point2dFlt{xProj, yProj};
   **/
}

inline auto LSysDraw::GetPolygon3dFlt(const std::vector<::LSYS::Vector>& polygon) noexcept
    -> std::vector<Point3dFlt>
{
  const auto numVertices = polygon.size();
  auto polygon3dFlt      = std::vector<Point3dFlt>(numVertices);

  for (auto i = 0U; i < numVertices; ++i)
  {
    polygon3dFlt[i] = GetPoint3dFlt(polygon[i]);
  }

  return polygon3dFlt;
}

inline auto LSysDraw::GetPoint3dFlt(const ::LSYS::Vector& point) noexcept -> Point3dFlt
{
  return {point(0), point(1), point(2)};
}

} // namespace GOOM::VISUAL_FX::L_SYSTEM
