#pragma once

#include "draw/goom_draw.h"
#include "lsys_colors.h"
#include "lsys_geom.h"
#include "point2d.h"

#include <cstdint>
#include <lsys/parsed_model.h>
#include <lsys/vector.h>
#include <vector>

namespace GOOM::VISUAL_FX::L_SYSTEM
{

class LSysDraw
{
public:
  LSysDraw(DRAW::IGoomDraw& draw,
           const LSysGeometry& lSysGeometry,
           LSysColors& lSysColors,
           float lineWidthFactor) noexcept;

  auto SetNumLSysCopies(uint32_t numLSysCopies) noexcept -> void;

  auto DrawLine(const ::LSYS::Vector& point1,
                const ::LSYS::Vector& point2,
                uint32_t lSysColor,
                float lineWidth) noexcept -> void;
  auto DrawPolygon(const std::vector<::LSYS::Vector>& polygon,
                   uint32_t lSysColor,
                   float lineWidth) noexcept -> void;

  [[nodiscard]] static auto GetPerspectivePoint(const ::LSYS::Point3dFlt& point3d) noexcept
      -> Point2dFlt;

private:
  DRAW::IGoomDraw& m_draw;
  const LSysGeometry& m_lSysGeometry;
  LSysColors& m_lSysColors;
  const float m_lineWidthFactor;
  uint32_t m_numLSysCopies = 1U;

  static constexpr auto MAX_LINE_WIDTH = 5.0F;
  auto DrawJoinedVertices(const std::vector<::LSYS::Point3dFlt>& vertices,
                          uint32_t lSysColor,
                          uint8_t lineWidth) noexcept -> void;
  static constexpr auto NUM_LINE_CHUNKS = 1U;
  auto DrawChunkedLine(const Point2dFlt& point1,
                       const Point2dFlt& point2,
                       uint32_t copyNum,
                       uint32_t lSysColor,
                       uint8_t lineWidth) noexcept -> void;
  [[nodiscard]] static auto GetPerspectiveProjection(
      const std::vector<::LSYS::Point3dFlt>& points3d) noexcept -> std::vector<Point2dFlt>;
  [[nodiscard]] static auto GetPolygon3dFlt(const std::vector<::LSYS::Vector>& polygon) noexcept
      -> std::vector<::LSYS::Point3dFlt>;
  [[nodiscard]] static auto GetPoint3dFlt(const ::LSYS::Vector& point) noexcept
      -> ::LSYS::Point3dFlt;
};

} // namespace GOOM::VISUAL_FX::L_SYSTEM
