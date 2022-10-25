#pragma once

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
namespace DRAW
{
class IGoomDraw;
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
  const UTILS::MATH::IGoomRand& m_goomRand;
  const Vec2dFlt m_screenMidPoint{
      MidpointFromOrigin({m_draw.GetScreenWidth(), m_draw.GetScreenHeight()}).ToFlt()};
  const UTILS::GRAPHICS::LineClipper m_lineClipper{
      {0.0F,
       0.0F, static_cast<float>(m_draw.GetScreenWidth() - 1),
       static_cast<float>(m_draw.GetScreenHeight() - 1)}
  };

  static constexpr auto MIN_CAMERA_X_OFFSET = -10.0F;
  static constexpr auto MAX_CAMERA_X_OFFSET = +10.0F;
  static constexpr auto MIN_CAMERA_Y_OFFSET = -10.0F;
  static constexpr auto MAX_CAMERA_Y_OFFSET = +10.0F;
  static constexpr auto CAMERA_Z_OFFSET     = -1.0F;
  V3dFlt m_cameraPosition{0.0F, 0.0F, CAMERA_Z_OFFSET};
  DRAW::MultiplePixels m_dominantColors{};
  uint8_t m_lineThickness{1U};

  auto DrawNode(const Tentacle3D& tentacle,
                size_t nodeNum,
                const Point2dInt& point1,
                const Point2dInt& point2,
                float brightness) -> void;

  auto PlotPoints(const Tentacle3D& tentacle, float brightness, const std::vector<V3dFlt>& points3D)
      -> void;
  [[nodiscard]] static auto GetBrightness(const Tentacle3D& tentacle) -> float;
  [[nodiscard]] static auto GetBrightnessCut(const Tentacle3D& tentacle) -> float;
  [[nodiscard]] auto GetPerspectiveProjection(const std::vector<V3dFlt>& points3D) const
      -> std::vector<Point2dFlt>;
};

inline auto TentaclePlotter::UpdateCameraPosition() noexcept -> void
{
  m_cameraPosition = {m_goomRand.GetRandInRange(MIN_CAMERA_X_OFFSET, MAX_CAMERA_X_OFFSET + 1U),
                      m_goomRand.GetRandInRange(MIN_CAMERA_Y_OFFSET, MAX_CAMERA_Y_OFFSET + 1U),
                      m_cameraPosition.z};
}

inline auto TentaclePlotter::SetDominantColors(const DRAW::MultiplePixels& dominantColors) noexcept
    -> void
{
  m_dominantColors = dominantColors;
}

inline auto TentaclePlotter::SetTentacleLineThickness(const uint8_t lineThickness) noexcept -> void
{
  m_lineThickness = lineThickness;
}

} // namespace VISUAL_FX::TENTACLES
} // namespace GOOM
