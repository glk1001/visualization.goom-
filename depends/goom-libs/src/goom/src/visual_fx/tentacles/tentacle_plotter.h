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

namespace UTILS
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

  auto SetProjectionDistance(float value) -> void;
  auto SetCameraPosition(float cameraDistance, float tentacleAngle) -> void;
  auto SetDominantColors(const Pixel& dominantMainColor,
                         const Pixel& dominantLowColor,
                         const Pixel& dominantDotColor) -> void;

  auto Plot3D(const Tentacle3D& tentacle) -> void;

private:
  DRAW::IGoomDraw& m_draw;
  const UTILS::MATH::IGoomRand& m_goomRand;
  const int32_t m_halfScreenWidth{
      static_cast<int32_t>(UTILS::MATH::U_HALF * m_draw.GetScreenWidth())};
  const int32_t m_halfScreenHeight{
      static_cast<int32_t>(UTILS::MATH::U_HALF * m_draw.GetScreenHeight())};

  FX_UTILS::DotDrawer m_dotDrawer;
  static constexpr uint32_t MIN_STEPS_BETWEEN_NODES = 2;
  static constexpr uint32_t MAX_STEPS_BETWEEN_NODES = 11;
  uint32_t m_numNodesBetweenDots = (MIN_STEPS_BETWEEN_NODES + MAX_STEPS_BETWEEN_NODES) / 2;

  float m_projectionDistance = 0.0F;
  float m_tentacleAngle = 0.0F;
  float m_cameraDistance = 0.0F;
  V3dFlt m_cameraPosition{};
  Pixel m_dominantMainColor{};
  Pixel m_dominantLowColor{};
  Pixel m_dominantDotColor{};

  auto DrawNode(const Tentacle3D& tentacle,
                size_t nodeNum,
                Point2dInt point1,
                Point2dInt point2,
                float brightness) -> void;
  auto DrawNodeLine(Point2dInt point1, Point2dInt point2, const DRAW::MultiplePixels& colors)
      -> void;
  auto DrawNodeDot(size_t nodeNum, Point2dInt point, const DRAW::MultiplePixels& colors) -> void;

  [[nodiscard]] auto GetMixedColors(const Tentacle3D& tentacle,
                                    float brightness,
                                    size_t nodeNum) const -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetBrightness(const Tentacle3D& tentacle) const -> float;
  [[nodiscard]] auto GetBrightnessCut(const Tentacle3D& tentacle) const -> float;
  [[nodiscard]] auto Get2DTentaclePoints(const Tentacle3D& tentacle) const
      -> std::vector<Point2dInt>;
  [[nodiscard]] auto GetTentacleAngleAboutY(const Tentacle3D& tentacle) const -> float;
  [[nodiscard]] static auto GetTransformedPoints(const std::vector<V3dFlt>& points,
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

inline auto TentaclePlotter::SetProjectionDistance(const float value) -> void
{
  m_projectionDistance = value;
}

inline auto TentaclePlotter::SetDominantColors(const Pixel& dominantMainColor,
                                               const Pixel& dominantLowColor,
                                               const Pixel& dominantDotColor) -> void
{
  m_dominantMainColor = dominantMainColor;
  m_dominantLowColor = dominantLowColor;
  m_dominantDotColor = dominantDotColor;
}

} // namespace VISUAL_FX::TENTACLES
} // namespace GOOM
