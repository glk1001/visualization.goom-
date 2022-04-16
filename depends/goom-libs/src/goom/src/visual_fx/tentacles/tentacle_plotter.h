#pragma once

#include "../fx_utils/dot_drawer.h"
#include "goom_graphic.h"
#include "tentacle3d.h"

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
  auto SetDominantColors(const Pixel& dominantColor, const Pixel& dominantLowColor) -> void;

  auto Plot3D(const Tentacle3D& tentacle) -> void;

private:
  DRAW::IGoomDraw& m_draw;
  const UTILS::MATH::IGoomRand& m_goomRand;
  const int32_t m_halfScreenWidth;
  const int32_t m_halfScreenHeight;

  FX_UTILS::DotDrawer m_dotDrawer;
  static constexpr uint32_t MIN_STEPS_BETWEEN_NODES = 1;
  static constexpr uint32_t MAX_STEPS_BETWEEN_NODES = 6;
  uint32_t m_numNodesBetweenDots = (MIN_STEPS_BETWEEN_NODES + MAX_STEPS_BETWEEN_NODES) / 2;

  float m_projectionDistance = 0.0F;
  float m_tentacleAngle = 0.0F;
  float m_cameraDistance = 0.0F;
  V3dFlt m_cameraPosition{};
  Pixel m_dominantColor{};
  Pixel m_dominantLowColor{};

  auto DrawNode(const Tentacle3D& tentacle,
                size_t nodeNum,
                int32_t x0,
                int32_t y0,
                int32_t x1,
                int32_t y1,
                float brightness) -> void;
  auto DrawNodeLine(
      int32_t x0, int32_t y0, int32_t x1, int32_t y1, const std::vector<Pixel>& colors) -> void;
  auto DrawNodeDot(size_t nodeNum, int32_t x, int32_t y, const std::vector<Pixel>& colors) -> void;

  [[nodiscard]] auto GetMixedColors(const Tentacle3D& tentacle,
                                    float brightness,
                                    size_t nodeNum) const -> std::vector<Pixel>;
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

inline auto TentaclePlotter::SetDominantColors(const Pixel& dominantColor,
                                               const Pixel& dominantLowColor) -> void
{
  m_dominantColor = dominantColor;
  m_dominantLowColor = dominantLowColor;
}

} // namespace VISUAL_FX::TENTACLES
} // namespace GOOM
