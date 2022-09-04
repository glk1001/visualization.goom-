#pragma once

#include "../fx_utils/dot_drawer.h"
#include "goom_graphic.h"
#include "point2d.h"
#include "tentacle3d.h"
#include "utils/math/misc.h"

namespace GOOM
{

namespace DRAW
{
class IGoomDraw;
}

namespace UTILS::MATH
{
class IGoomRand;
}

namespace UTILS::GRAPHICS
{
class SmallImageBitmaps;
}

namespace VISUAL_FX::TENTACLES
{

class TentaclePlotter
{
public:
  TentaclePlotter() noexcept = delete;
  TentaclePlotter(DRAW::IGoomDraw& draw,
                  const UTILS::MATH::IGoomRand& goomRand,
                  const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps) noexcept;

  auto ChangeNumNodesBetweenDots() -> void;
  auto ChangeDotSizes() -> void;

  auto SetDominantColors(const DRAW::MultiplePixels& dominantColors, const Pixel& dominantDotColor)
      -> void;

  auto Plot3D(const Tentacle3D& tentacle) -> void;

private:
  DRAW::IGoomDraw& m_draw;
  const UTILS::MATH::IGoomRand& m_goomRand;
  const Vec2dInt m_screenMidPoint{
      MidpointFromOrigin({m_draw.GetScreenWidth(), m_draw.GetScreenHeight()})};

  FX_UTILS::DotDrawer m_dotDrawer;
  static constexpr uint32_t MIN_STEPS_BETWEEN_NODES = 2;
  static constexpr uint32_t MAX_STEPS_BETWEEN_NODES = 11;
  uint32_t m_numNodesBetweenDots = (MIN_STEPS_BETWEEN_NODES + MAX_STEPS_BETWEEN_NODES) / 2;

  static constexpr auto PROJECTION_DISTANCE = 170.0F;
  static constexpr auto TENTACLE_ANGLE      = -1.0F * STD20::pi;
  static constexpr auto CAMERA_X_OFFSET     = 0.0F;
  static constexpr auto CAMERA_Y_OFFSET     = 0.0F;
  static constexpr auto CAMERA_Z_OFFSET     = 20.0F;
  const V3dFlt m_cameraPosition{CAMERA_X_OFFSET, CAMERA_Y_OFFSET, CAMERA_Z_OFFSET};
  DRAW::MultiplePixels m_dominantColors{};
  Pixel m_dominantDotColor{};

  auto DrawNode(const Tentacle3D& tentacle,
                size_t nodeNum,
                Point2dInt point1,
                Point2dInt point2,
                float brightness) -> void;
  auto DrawNodeLine(Point2dInt point1, Point2dInt point2, const DRAW::MultiplePixels& colors)
      -> void;
  auto DrawNodeDot(size_t nodeNum, Point2dInt point, const DRAW::MultiplePixels& colors) -> void;

  auto PlotPoints(const Tentacle3D& tentacle, float brightness, const std::vector<V3dFlt>& points3D)
      -> void;
  [[nodiscard]] static auto GetBrightness(const Tentacle3D& tentacle) -> float;
  [[nodiscard]] static auto GetBrightnessCut(const Tentacle3D& tentacle) -> float;
  [[nodiscard]] auto Get2DProjectedTentaclePoints(const Tentacle3D& tentacle,
                                                  const std::vector<V3dFlt>& points3D) const
      -> std::vector<Point2dInt>;
  [[nodiscard]] static auto GetTentacleAngleAboutY(const Tentacle3D& tentacle) -> float;
  [[nodiscard]] static auto GetTransformedPoints(const std::vector<V3dFlt>& points3D,
                                                 const V3dFlt& translate,
                                                 float angle) -> std::vector<V3dFlt>;
  [[nodiscard]] auto GetPerspectiveProjection(const std::vector<V3dFlt>& points3D) const
      -> std::vector<Point2dInt>;
  static auto RotateAboutYAxis(float sinAngle,
                               float cosAngle,
                               const V3dFlt& srcPoint,
                               V3dFlt& destPoint) -> void;
  static auto Translate(const V3dFlt& vAdd, V3dFlt& vInOut) -> void;
};

inline auto TentaclePlotter::SetDominantColors(const DRAW::MultiplePixels& dominantColors,
                                               const Pixel& dominantDotColor) -> void
{
  m_dominantColors   = dominantColors;
  m_dominantDotColor = dominantDotColor;
}

} // namespace VISUAL_FX::TENTACLES
} // namespace GOOM
