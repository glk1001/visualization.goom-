#pragma once

#include "draw/goom_draw.h"
#include "draw/shape_drawers/line_drawer.h"
#include "goom_graphic.h"
#include "point2d.h"
#include "tentacle3d.h"
#include "utils/graphics/line_clipper.h"

namespace GOOM
{

namespace UTILS::MATH
{
class IGoomRand;
}

namespace VISUAL_FX::TENTACLES
{

class TentaclePlotter
{
public:
  TentaclePlotter() noexcept = delete;
  TentaclePlotter(DRAW::IGoomDraw& draw, const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto UpdateCameraPosition() noexcept -> void;
  auto SetDominantColors(const DRAW::MultiplePixels& dominantColors) noexcept -> void;
  auto SetTentacleLineThickness(uint8_t lineThickness) noexcept -> void;

  auto Plot3D(const Tentacle3D& tentacle) noexcept -> void;

private:
  DRAW::IGoomDraw& m_draw;
  DRAW::SHAPE_DRAWERS::LineDrawer m_lineDrawer{m_draw};
  const UTILS::MATH::IGoomRand& m_goomRand;
  const Vec2dFlt m_screenMidPoint{
      MidpointFromOrigin({m_draw.GetDimensions().GetWidth(), m_draw.GetDimensions().GetHeight()})
          .ToFlt()};
  uint8_t m_lineThickness = 1U;

  using LineClipper = UTILS::GRAPHICS::LineClipper;
  LineClipper m_lineClipper{GetLineClipRectangle()};
  [[nodiscard]] auto GetLineClipRectangle() const noexcept -> LineClipper::ClipRectangle;

  static constexpr auto PROJECTION_DISTANCE = 170.0F;
  static constexpr auto MIN_CAMERA_X_OFFSET = -10.0F;
  static constexpr auto MAX_CAMERA_X_OFFSET = +10.0F;
  static constexpr auto MIN_CAMERA_Y_OFFSET = -10.0F;
  static constexpr auto MAX_CAMERA_Y_OFFSET = +10.0F;
  static constexpr auto MIN_CAMERA_Z_OFFSET = +01.0F; // Don't make this any smaller
  static constexpr auto MAX_CAMERA_Z_OFFSET = +10.1F;
  static_assert(MIN_CAMERA_X_OFFSET < MAX_CAMERA_X_OFFSET);
  static_assert(MIN_CAMERA_Y_OFFSET < MAX_CAMERA_Y_OFFSET);
  static_assert(MIN_CAMERA_Z_OFFSET < MAX_CAMERA_Z_OFFSET);
  V3dFlt m_cameraPosition{0.0F, 0.0F, MIN_CAMERA_Z_OFFSET};
  DRAW::MultiplePixels m_dominantColors{};

  auto PlotPoints(const Tentacle3D& tentacle, float brightness, const std::vector<V3dFlt>& points3D)
      -> void;
  [[nodiscard]] static auto GetBrightness(const Tentacle3D& tentacle) -> float;
  [[nodiscard]] static auto GetBrightnessCut(const Tentacle3D& tentacle) -> float;
  struct Line2DInt
  {
    Point2dInt point1;
    Point2dInt point2;
  };
  auto DrawNode(const Tentacle3D& tentacle, float t, const Line2DInt& line, float brightness)
      -> void;
  [[nodiscard]] auto GetPerspectiveProjection(const std::vector<V3dFlt>& points3D) const
      -> std::vector<Line2DInt>;
  [[nodiscard]] auto GetPerspectivePoint(const V3dFlt& point3D) const -> Point2dFlt;
};

inline auto TentaclePlotter::UpdateCameraPosition() noexcept -> void
{
  m_cameraPosition = {m_goomRand.GetRandInRange(MIN_CAMERA_X_OFFSET, MAX_CAMERA_X_OFFSET),
                      m_goomRand.GetRandInRange(MIN_CAMERA_Y_OFFSET, MAX_CAMERA_Y_OFFSET),
                      m_goomRand.GetRandInRange(MIN_CAMERA_Z_OFFSET, MAX_CAMERA_Z_OFFSET)};
}

inline auto TentaclePlotter::SetDominantColors(const DRAW::MultiplePixels& dominantColors) noexcept
    -> void
{
  m_dominantColors = dominantColors;
}

} // namespace VISUAL_FX::TENTACLES
} // namespace GOOM
