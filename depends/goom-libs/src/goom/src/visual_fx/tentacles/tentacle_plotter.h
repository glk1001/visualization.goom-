#pragma once

#include "draw/goom_draw.h"
#include "draw/shape_drawers/circle_drawer.h"
#include "draw/shape_drawers/line_drawer.h"
#include "goom_types.h"
#include "point2d.h"
#include "tentacle3d.h"
#include "utils/graphics/line_clipper.h"
#include "utils/math/goom_rand_base.h"

#include <cstdint>
#include <functional>
#include <vector>

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
