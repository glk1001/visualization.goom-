#pragma once

#include "goom_graphic.h"
#include "point2d.h"
#include "tentacle3d.h"

namespace GOOM
{

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
  explicit TentaclePlotter(DRAW::IGoomDraw& draw) noexcept;

  [[nodiscard]] auto GetCameraPosition() const noexcept -> V3dFlt;

  auto SetDominantColors(const DRAW::MultiplePixels& dominantColors) -> void;

  auto Plot3D(const Tentacle3D& tentacle) -> void;

private:
  DRAW::IGoomDraw& m_draw;
  const Vec2dInt m_screenMidPoint{
      MidpointFromOrigin({m_draw.GetScreenWidth(), m_draw.GetScreenHeight()})};

  static constexpr auto CAMERA_X_OFFSET     = 0.0F;
  static constexpr auto CAMERA_Y_OFFSET     = 0.0F;
  static constexpr auto CAMERA_Z_OFFSET     = 20.0F;
  const V3dFlt m_cameraPosition{CAMERA_X_OFFSET, CAMERA_Y_OFFSET, CAMERA_Z_OFFSET};
  DRAW::MultiplePixels m_dominantColors{};

  auto DrawNode(const Tentacle3D& tentacle,
                size_t nodeNum,
                Point2dInt point1,
                Point2dInt point2,
                float brightness) -> void;
  auto DrawNodeLine(Point2dInt point1, Point2dInt point2, const DRAW::MultiplePixels& colors)
      -> void;

  auto PlotPoints(const Tentacle3D& tentacle, float brightness, const std::vector<V3dFlt>& points3D)
      -> void;
  [[nodiscard]] static auto GetBrightness(const Tentacle3D& tentacle) -> float;
  [[nodiscard]] static auto GetBrightnessCut(const Tentacle3D& tentacle) -> float;
  [[nodiscard]] auto GetPerspectiveProjection(const std::vector<V3dFlt>& points3D) const
      -> std::vector<Point2dInt>;
};

inline auto TentaclePlotter::GetCameraPosition() const noexcept -> V3dFlt
{
  return m_cameraPosition;
}

inline auto TentaclePlotter::SetDominantColors(const DRAW::MultiplePixels& dominantColors) -> void
{
  m_dominantColors = dominantColors;
}

} // namespace VISUAL_FX::TENTACLES
} // namespace GOOM
