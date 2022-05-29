#include "fractal.h"

#include "color/random_colormaps.h"
#include "similitudes.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/t_values.h"

namespace GOOM::VISUAL_FX::IFS
{

using COLOR::RandomColorMaps;
using UTILS::TValue;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::U_HALF;

Fractal::Fractal(const uint32_t screenWidth,
                 const uint32_t screenHeight,
                 const IGoomRand& goomRand,
                 const SmallImageBitmaps& smallBitmaps)
  : m_similitudes{goomRand, smallBitmaps},
    m_goomRand{goomRand},
    m_halfWidth{static_cast<Flt>(U_HALF * (screenWidth - 1))},
    m_halfHeight{static_cast<Flt>(U_HALF * (screenHeight - 1))},
    m_hits1{screenWidth, screenHeight},
    m_hits2{screenWidth, screenHeight}
{
  m_speedTransitionT.Reset(TValue::MAX_T_VALUE);

  InitHits();
  Reset();
}

auto Fractal::SetWeightedColorMaps(const std::shared_ptr<RandomColorMaps>& weightedColorMaps)
    -> void
{
  m_similitudes.SetWeightedColorMaps(weightedColorMaps);
}

auto Fractal::Init() -> void
{
  InitHits();
  m_similitudes.Init();
}

inline auto Fractal::InitHits() -> void
{
  m_prevHits.get().Reset();
  m_curHits.get().Reset();
}

auto Fractal::GetSpeed() const -> uint32_t
{
  if (m_speedTransitionT.IsStopped())
  {
    return m_speed;
  }
  return STD20::lerp(m_prevSpeed, m_speed, m_speedTransitionT());
}

auto Fractal::SetSpeed(const uint32_t val) -> void
{
  m_prevSpeed = GetSpeed();
  m_speed = val;
  m_speedTransitionT.Reset();
}

auto Fractal::Reset() -> void
{
  m_maxCountTimesSpeed =
      m_goomRand.GetRandInRange(MIN_MAX_COUNT_TIMES_SPEED, MAX_MAX_COUNT_TIMES_SPEED + 1U);

  m_similitudes.ResetCurrentIfsFunc();
}

auto Fractal::GetNextIfsPoints() -> const std::vector<IfsPoint>&
{
  m_speedTransitionT.Increment();

  UpdateMainSimis();

  const auto& currentIfsPoints = GetCurrentIfsPoints();

  UpdateIterationCount();

  return currentIfsPoints;
}

inline auto Fractal::GetCurrentIfsPoints() -> const std::vector<IfsPoint>&
{
  m_curHits.get().Reset();
  DrawFractal();
  const std::vector<IfsPoint>& currentBuffer = m_curHits.get().GetBuffer();
  std::swap(m_prevHits, m_curHits);
  return currentBuffer;
}

inline auto Fractal::UpdateIterationCount() -> void
{
  if (m_iterationCount < (m_maxCountTimesSpeed / GetSpeed()))
  {
    ++m_iterationCount;
    return;
  }

  m_similitudes.IterateSimis();

  m_iterationCount = 0;
}

auto Fractal::DrawFractal() -> void
{
  const std::vector<Similitude>& mainSimiGroup = m_similitudes.GetMainSimiGroup();
  const size_t numSimis = m_similitudes.GetNumSimis();

  for (size_t i = 0; i < numSimis; ++i)
  {
    const FltPoint point0 = mainSimiGroup[i].GetCPoint();

    for (size_t j = 0; j < numSimis; ++j)
    {
      if (i != j)
      {
        const FltPoint point = m_similitudes.Transform(mainSimiGroup[j], point0);
        Trace(m_similitudes.GetSimiDepth(), point);
      }
    }
  }
}

inline auto Fractal::UpdateMainSimis() -> void
{
  const Dbl uValue =
      static_cast<Dbl>(m_iterationCount * GetSpeed()) / static_cast<Dbl>(m_maxCountTimesSpeed);
  m_similitudes.UpdateMainSimis(uValue);
}

auto Fractal::Trace(const uint32_t curDepth, const FltPoint& point0) -> void
{
  const std::vector<Similitude>& mainSimiGroup = m_similitudes.GetMainSimiGroup();
  const size_t numSimis = m_similitudes.GetNumSimis();

  for (size_t i = 0; i < numSimis; ++i)
  {
    const FltPoint point = m_similitudes.Transform(mainSimiGroup[i], point0);

    UpdateHits(mainSimiGroup[i], point);

    if (0 == curDepth)
    {
      continue;
    }
    if (AreSimilarPoints(point0, point))
    {
      continue;
    }

    Trace(curDepth - 1, point);
  }
}

inline auto Fractal::AreSimilarPoints(const FltPoint& point1, const FltPoint& point2) -> bool
{
  // TODO What's going on here?
  static constexpr Flt CUTOFF = 16;
  return (std::abs(point1.x - point2.x) < CUTOFF) || (std::abs(point1.y - point2.y) < CUTOFF);
}

inline auto Fractal::UpdateHits(const Similitude& simi, const FltPoint& point) -> void
{
  const int32_t x = m_halfWidth + DivBy2Units(point.x * m_halfWidth);
  const int32_t y = m_halfHeight - DivBy2Units(point.y * m_halfHeight);
  m_curHits.get().AddHit(x, y, simi);
}

} // namespace GOOM::VISUAL_FX::IFS
