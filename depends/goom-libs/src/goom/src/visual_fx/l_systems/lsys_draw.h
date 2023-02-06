#pragma once

#include "draw/shape_drawers/line_drawer_with_effects.h"
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
  LSysDraw(const LSysGeometry& lSysGeometry,
           LSysColors& lSysColors,
           float lineWidthFactor) noexcept;

  auto SetNumLSysCopies(uint32_t numLSysCopies) noexcept -> void;
  auto SetLineDrawer(DRAW::SHAPE_DRAWERS::ILineDrawerWithEffects& lineDrawer) noexcept -> void;

  auto DrawLine(const uint32_t lSysColor,
                const ::LSYS::Vector& point1,
                const ::LSYS::Vector& point2,
                const float lineWidth) noexcept -> void;
  auto DrawPolygon(const uint32_t lSysColor,
                   const std::vector<::LSYS::Vector>& polygon,
                   const float lineWidth) noexcept -> void;

  [[nodiscard]] static auto GetPerspectivePoint(const ::LSYS::Point3dFlt& point3d) noexcept
      -> Point2dFlt;

private:
  const LSysGeometry& m_lSysGeometry;
  LSysColors& m_lSysColors;
  const float m_lineWidthFactor;
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

inline auto LSysDraw::SetLineDrawer(
    DRAW::SHAPE_DRAWERS::ILineDrawerWithEffects& lineDrawer) noexcept -> void
{
  m_lineDrawer = &lineDrawer;
}

} // namespace GOOM::VISUAL_FX::L_SYSTEM
