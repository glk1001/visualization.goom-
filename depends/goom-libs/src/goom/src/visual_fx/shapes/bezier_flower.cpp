#include "bezier_flower.h"

#include "color/colormaps.h"
#include "color/random_colormaps.h"

namespace GOOM::VISUAL_FX::SHAPES
{
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using UTILS::m_two_pi;

void BezierFlower::Draw(const Point2dInt& origin,
                        const uint32_t numPetals,
                        const uint32_t petalOffset)
{
  Bezier::Bezier<3> petal = GetBezierPetal(origin, petalOffset);
  const auto originX = static_cast<float>(origin.x);
  const auto originY = static_cast<float>(origin.y);

  m_petalColoring.SetNumPetals(numPetals);
  m_bezierDrawer.SetLineColorFunc([&](const float t)
                                  { return m_petalColoring.GetCurrentLineColorMap().GetColor(t); });
  m_bezierDrawer.SetDotColorFunc([&](const float t)
                                 { return m_petalColoring.GetCurrentDotColorMap().GetColor(t); });

  const float angleStep = 1.0F / static_cast<float>(numPetals);
  for (uint32_t i = 0; i < numPetals; ++i)
  {
    m_petalColoring.StartColoringPetal(i);
    const float petalRotation = m_rotation + (angleStep * m_two_pi);
    petal.rotate(petalRotation, {originX, originY});
    m_bezierDrawer.Draw(petal, 0.0F, 1.0F);
  }
}

inline auto BezierFlower::GetBezierPetal(const Point2dInt& origin, const uint32_t offset)
    -> Bezier::Bezier<3>
{
  const auto originX = static_cast<float>(origin.x);
  const auto originY = static_cast<float>(origin.y);
  const auto fltOffset = static_cast<float>(offset);

  const Bezier::Bezier<3> petal{
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

  const RandomColorMaps randomColorMaps{m_goomRand};

  m_lineColorMaps.resize(m_numPetals, &randomColorMaps.GetRandomColorMap());
  m_dotColorMaps.resize(m_numPetals, &randomColorMaps.GetRandomColorMap());

  for (size_t i = 0; i < numPetals; ++i)
  {
    if (0 == (i % 5))
    {
      m_lineColorMaps[i] = &randomColorMaps.GetRandomColorMap();
      m_dotColorMaps[i] = &randomColorMaps.GetRandomColorMap();
    }
  }
}

inline void PetalColoring::StartColoringPetal(const uint32_t petalNum)
{
  assert(petalNum < m_numPetals);
  m_currentPetalNum = petalNum;
}

inline auto PetalColoring::GetCurrentLineColorMap() const -> const IColorMap&
{
  return *m_lineColorMaps.at(m_currentPetalNum);
}

inline auto PetalColoring::GetCurrentDotColorMap() const -> const IColorMap&
{
  return *m_dotColorMaps.at(m_currentPetalNum);
}

} // namespace GOOM::VISUAL_FX::SHAPES
