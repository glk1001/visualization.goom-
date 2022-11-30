#include "lsys_draw.h"

//#undef NO_LOGGING

#include "draw/goom_draw.h"
#include "goom_logger.h"
#include "lsys_colors.h"
#include "lsys_geom.h"
#include "point2d.h"
#include "utils/t_values.h"

namespace GOOM::VISUAL_FX::L_SYSTEM
{

using DRAW::MultiplePixels;
using UTILS::IncrementedValue;
using UTILS::TValue;

using ::LSYS::Point3dFlt;

LSysDraw::LSysDraw(DRAW::IGoomDraw& draw,
                   const LSysGeometry& lSysGeometry,
                   LSysColors& lSysColors,
                   const float lineWidthFactor) noexcept
  : m_draw{draw},
    m_lSysGeometry{lSysGeometry},
    m_lSysColors{lSysColors},
    m_lineWidthFactor{lineWidthFactor}
{
}

auto LSysDraw::SetNumLSysCopies(const uint32_t numLSysCopies) noexcept -> void
{
  Expects(numLSysCopies > 0U);
  m_numLSysCopies = numLSysCopies;
}

auto LSysDraw::DrawLine(const ::LSYS::Vector& point1,
                        const ::LSYS::Vector& point2,
                        const uint32_t lSysColor,
                        const float lineWidth) noexcept -> void
{
  const auto iLineWidth =
      static_cast<uint8_t>(std::clamp(m_lineWidthFactor * lineWidth, 1.0F, MAX_LINE_WIDTH));

  DrawJoinedVertices({GetPoint3dFlt(point1), GetPoint3dFlt(point2)}, lSysColor, iLineWidth);
}

auto LSysDraw::DrawPolygon(const std::vector<::LSYS::Vector>& polygon,
                           const uint32_t lSysColor,
                           const float lineWidth) noexcept -> void
{
  const auto iLineWidth =
      static_cast<uint8_t>(std::clamp(m_lineWidthFactor * lineWidth, 1.0F, MAX_LINE_WIDTH));

  DrawJoinedVertices(GetPolygon3dFlt(polygon), lSysColor, iLineWidth);
}

auto LSysDraw::DrawJoinedVertices(const std::vector<Point3dFlt>& vertices,
                                  const uint32_t lSysColor,
                                  const uint8_t lineWidth) noexcept -> void
{
  const auto numVertices = vertices.size();
  Expects(numVertices > 1);

  const auto points2d = GetPerspectiveProjection(vertices);

  //LogInfo("numVertices = {}", numVertices);
  auto point2d0 = points2d.at(0);
  for (auto i = 1U; i < numVertices; ++i)
  {
    const auto point0 = point2d0;
    const auto point1 = points2d.at(i);

    //LogInfo("point0 = ({}, {}), point1 = ({}, {})", point0.x, point0.y, point1.x, point1.y);

    for (auto copyNum = 0U; copyNum < m_numLSysCopies; ++copyNum)
    {
      const auto tPoint0 = m_lSysGeometry.GetTransformedPoint(point0, copyNum);
      const auto tPoint1 = m_lSysGeometry.GetTransformedPoint(point1, copyNum);

      /**
      LogInfo("copyNum = {}, tPoint0 = ({}, {}), tPoint1 = ({}, {})",
              copyNum,
              tPoint0.x,
              tPoint0.y,
              tPoint1.x,
              tPoint1.y);
              **/

      // TODO(Glk) put in transform
      /**
      tPoint0.x = static_cast<float>(m_goomInfo.GetScreenWidth() - 1) - tPoint0.x;
      tPoint1.x = static_cast<float>(m_goomInfo.GetScreenWidth() - 1) - tPoint1.x;
      tPoint0.y = static_cast<float>(m_goomInfo.GetScreenHeight() - 1) - tPoint0.y;
      tPoint1.y = static_cast<float>(m_goomInfo.GetScreenHeight() - 1) - tPoint1.y;
       **/

      DrawChunkedLine(tPoint0, tPoint1, copyNum, lSysColor, lineWidth);

      //LogInfo("m_currentColorTs.at({}) = {}", copyNum, m_currentColorTs.at(copyNum)());
    }

    m_lSysColors.IncrementColorTs();

    point2d0 = points2d.at(i);
  }
}

inline auto LSysDraw::DrawChunkedLine(const Point2dFlt& point1,
                                      const Point2dFlt& point2,
                                      const uint32_t copyNum,
                                      const uint32_t lSysColor,
                                      const uint8_t lineWidth) noexcept -> void
{
  if constexpr (1U == NUM_LINE_CHUNKS)
  {
    m_draw.Line(point1.ToInt(),
                point2.ToInt(),
                m_lSysColors.GetColors(copyNum, lSysColor, lineWidth),
                lineWidth);
    return;
  }

  auto pointOnLine =
      IncrementedValue{point1, point2, TValue::StepType::SINGLE_CYCLE, NUM_LINE_CHUNKS};

  for (auto i = 0U; i < NUM_LINE_CHUNKS; ++i)
  {
    const auto tempPoint1 = pointOnLine();
    pointOnLine.Increment();
    const auto tempPoint2 = pointOnLine();

    m_draw.Line(tempPoint1.ToInt(),
                tempPoint2.ToInt(),
                m_lSysColors.GetColors(copyNum, lSysColor, lineWidth),
                lineWidth);
  }
}

auto LSysDraw::GetPerspectiveProjection(const std::vector<Point3dFlt>& points3d) noexcept
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
  //TODO(glk) Why does this look better?
  return {point3d.x, point3d.y};

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
