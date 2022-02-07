#pragma once

#include "color/colormaps.h"
#include "utils/goom_rand_base.h"
#include "utils/graphics/bezier_drawer.h"

#include <bezier/bezier.h>
#include <cstdint>
#include <vector>

namespace GOOM::VISUAL_FX::SHAPES
{

class PetalColoring
{
public:
  explicit PetalColoring(const UTILS::IGoomRand& goomRand) noexcept;

  void SetNumPetals(uint32_t numPetals);

  void StartColoringPetal(uint32_t petalNum);
  [[nodiscard]] auto GetCurrentLineColorMap() const -> const COLOR::IColorMap&;
  [[nodiscard]] auto GetCurrentDotColorMap() const -> const COLOR::IColorMap&;

private:
  const UTILS::IGoomRand& m_goomRand;
  uint32_t m_numPetals = 0;
  std::vector<const COLOR::IColorMap*> m_lineColorMaps{};
  std::vector<const COLOR::IColorMap*> m_dotColorMaps{};
  uint32_t m_currentPetalNum = 0;
};

class BezierFlower
{
public:
  BezierFlower(UTILS::GRAPHICS::BezierDrawer& bezierDrawer, PetalColoring& petalColoring) noexcept;

  [[nodiscard]] auto GetBezierDrawer() const -> const UTILS::GRAPHICS::BezierDrawer&;

  void SetRotation(float rotation);

  void Draw(const Point2dInt& origin, uint32_t numPetals, uint32_t petalOffset);

private:
  UTILS::GRAPHICS::BezierDrawer& m_bezierDrawer;
  PetalColoring& m_petalColoring;
  float m_rotation = 0.0F;
  [[nodiscard]] static auto GetBezierPetal(const Point2dInt& origin, uint32_t offset)
      -> Bezier::Bezier<3>;
};

inline PetalColoring::PetalColoring(const UTILS::IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand}
{
}

inline BezierFlower::BezierFlower(UTILS::GRAPHICS::BezierDrawer& bezierDrawer,
                                  PetalColoring& petalColoring) noexcept
  : m_bezierDrawer{bezierDrawer}, m_petalColoring{petalColoring}
{
}

inline auto BezierFlower::GetBezierDrawer() const -> const UTILS::GRAPHICS::BezierDrawer&
{
  return m_bezierDrawer;
}

inline void BezierFlower::SetRotation(const float rotation)
{
  m_rotation = rotation;
}

} // namespace GOOM::VISUAL_FX::SHAPES
