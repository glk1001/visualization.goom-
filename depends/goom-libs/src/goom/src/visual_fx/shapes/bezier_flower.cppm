module;

#include "bezier/bezier.h"

#include <cstdint>
#include <vector>

module Goom.VisualFx.ShapesFx:BezierFlower;

import Goom.Color.ColorMapBase;
import Goom.Color.ColorMaps;
import Goom.Color.RandomColorMaps;
import Goom.Utils.Graphics.BezierDrawer;
import Goom.Utils.Math.GoomRandBase;
import Goom.Utils.Math.Misc;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomGraphic;
import Goom.Lib.Point2d;

namespace GOOM::VISUAL_FX::SHAPES
{

class PetalColoring
{
public:
  explicit PetalColoring(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  void SetNumPetals(uint32_t numPetals);

  void StartColoringPetal(uint32_t petalNum);
  [[nodiscard]] auto GetCurrentLineColorMap() const -> const COLOR::IColorMap&;
  [[nodiscard]] auto GetCurrentDotColorMap() const -> const COLOR::IColorMap&;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  uint32_t m_numPetals = 0;
  std::vector<COLOR::ColorMapPtrWrapper> m_lineColorMaps;
  std::vector<COLOR::ColorMapPtrWrapper> m_dotColorMaps;
  uint32_t m_currentPetalNum = 0;
};

class BezierFlower
{
public:
  BezierFlower(UTILS::GRAPHICS::BezierDrawer& bezierDrawer, PetalColoring& petalColoring) noexcept;

  [[nodiscard]] auto GetBezierDrawer() const -> const UTILS::GRAPHICS::BezierDrawer&;

  auto SetRotation(float rotation) -> void;

  auto Draw(uint32_t numPetals, const Point2dInt& origin, uint32_t petalOffset) -> void;

private:
  UTILS::GRAPHICS::BezierDrawer* m_bezierDrawer;
  PetalColoring* m_petalColoring;
  float m_rotation = 0.0F;
  [[nodiscard]] static auto GetBezierPetal(const Point2dInt& origin,
                                           uint32_t offset) -> Bezier::Bezier<3>;
};

} // namespace GOOM::VISUAL_FX::SHAPES

namespace GOOM::VISUAL_FX::SHAPES
{

inline PetalColoring::PetalColoring(const UTILS::MATH::IGoomRand& goomRand) noexcept
  : m_goomRand{&goomRand}
{
}

inline BezierFlower::BezierFlower(UTILS::GRAPHICS::BezierDrawer& bezierDrawer,
                                  PetalColoring& petalColoring) noexcept
  : m_bezierDrawer{&bezierDrawer}, m_petalColoring{&petalColoring}
{
}

inline auto BezierFlower::GetBezierDrawer() const -> const UTILS::GRAPHICS::BezierDrawer&
{
  return *m_bezierDrawer;
}

inline auto BezierFlower::SetRotation(const float rotation) -> void
{
  m_rotation = rotation;
}

using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using UTILS::MATH::TWO_PI;

auto BezierFlower::Draw(const uint32_t numPetals,
                        const Point2dInt& origin,
                        const uint32_t petalOffset) -> void
{
  auto petal         = GetBezierPetal(origin, petalOffset);
  const auto originX = static_cast<float>(origin.x);
  const auto originY = static_cast<float>(origin.y);

  m_petalColoring->SetNumPetals(numPetals);
  m_bezierDrawer->SetLineColorFunc(
      [this](const float t) { return m_petalColoring->GetCurrentLineColorMap().GetColor(t); });
  m_bezierDrawer->SetDotColorFunc([this](const float t)
                                  { return m_petalColoring->GetCurrentDotColorMap().GetColor(t); });

  const auto angleStep = 1.0F / static_cast<float>(numPetals);
  for (auto i = 0U; i < numPetals; ++i)
  {
    m_petalColoring->StartColoringPetal(i);
    const auto petalRotation = m_rotation + (angleStep * TWO_PI);
    petal.rotate(petalRotation, {originX, originY});
    m_bezierDrawer->Draw(petal, 0.0F, 1.0F);
  }
}

inline auto BezierFlower::GetBezierPetal(const Point2dInt& origin,
                                         const uint32_t offset) -> Bezier::Bezier<3>
{
  const auto originX   = static_cast<float>(origin.x);
  const auto originY   = static_cast<float>(origin.y);
  const auto fltOffset = static_cast<float>(offset);

  const auto petal = Bezier::Bezier<3>{
      {
       {originX, originY},
       {originX - fltOffset, originY - fltOffset},
       {originX - fltOffset, originY + fltOffset},
       {originX, originY},
       }
  };

  return petal;
}

void PetalColoring::SetNumPetals(const uint32_t numPetals)
{
  m_numPetals = numPetals;

  const auto randomColorMaps = RandomColorMaps{MAX_ALPHA, *m_goomRand};

  m_lineColorMaps.resize(m_numPetals, randomColorMaps.GetRandomColorMap());
  m_dotColorMaps.resize(m_numPetals, randomColorMaps.GetRandomColorMap());

  for (auto i = 0U; i < numPetals; ++i)
  {
    static constexpr auto COLOR_CHANGE_FREQUENCY = 5U;
    if (0 == (i % COLOR_CHANGE_FREQUENCY))
    {
      m_lineColorMaps[i] = randomColorMaps.GetRandomColorMap();
      m_dotColorMaps[i]  = randomColorMaps.GetRandomColorMap();
    }
  }
}

inline void PetalColoring::StartColoringPetal(const uint32_t petalNum)
{
  Expects(petalNum < m_numPetals);
  m_currentPetalNum = petalNum;
}

inline auto PetalColoring::GetCurrentLineColorMap() const -> const IColorMap&
{
  return m_lineColorMaps.at(m_currentPetalNum);
}

inline auto PetalColoring::GetCurrentDotColorMap() const -> const IColorMap&
{
  return m_dotColorMaps.at(m_currentPetalNum);
}

} // namespace GOOM::VISUAL_FX::SHAPES
