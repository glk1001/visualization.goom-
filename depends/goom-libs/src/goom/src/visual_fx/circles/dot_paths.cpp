#include "dot_paths.h"

#include "point2d.h"
#include "utils/math/misc.h"
#include "utils/math/paths.h"
#include "utils/t_values.h"

#undef NDEBUG
#include <cmath>
#include <cstdint>
#include <vector>

namespace GOOM::VISUAL_FX::CIRCLES
{

using UTILS::TValue;
using UTILS::MATH::OscillatingPath;
using UTILS::MATH::PathParams;
using UTILS::MATH::TWO_PI;

DotPaths::DotPaths(const uint32_t numDots,
                   const Point2dInt& startTarget,
                   const float startRadius,
                   const PathParams& startPathParams) noexcept
  : m_numDots{numDots},
    m_dotStartingPositions{GetDotStartingPositions(m_numDots, startTarget, startRadius)},
    m_target{startTarget},
    m_pathParams{startPathParams},
    m_dotPaths{GetNewDotPaths()}
{
}

auto DotPaths::GetDotStartingPositions(const uint32_t numDots,
                                       const Point2dInt& target,
                                       const float radius) -> std::vector<Point2dInt>
{
  std::vector<Point2dInt> dotStartingPositions(numDots);

  const float angleStep = UTILS::MATH::TWO_PI / static_cast<float>(numDots);

  float angle = 0.0F;
  for (size_t i = 0; i < numDots; ++i)
  {
    const auto x = static_cast<int32_t>(std::lround(radius * std::cos(angle)));
    const auto y = static_cast<int32_t>(std::lround(radius * std::sin(angle)));

    dotStartingPositions.at(i) = target + Vec2dInt{x, y};

    angle += angleStep;
  }

  return dotStartingPositions;
}

void DotPaths::SetTarget(const Point2dInt& target)
{
  m_target = target;
  m_dotPaths = GetNewDotPaths();
}

auto DotPaths::GetNewDotPaths() -> std::vector<OscillatingPath>
{
  std::vector<UTILS::MATH::OscillatingPath> dotPaths{};

  for (size_t i = 0; i < m_numDots; ++i)
  {
    dotPaths.emplace_back(m_dotStartingPositions.at(i), m_target, m_positionT, m_pathParams, true);
  }

  return dotPaths;
}

auto DotPaths::GetNextDotPositions() const -> std::vector<Point2dInt>
{
  std::vector<Point2dInt> nextDotPositions(m_numDots);
  for (size_t i = 0; i < m_numDots; ++i)
  {
    //const Point2dInt jitter = {m_goomRand.GetRandInRange(-2, +3),
    //                           m_goomRand.GetRandInRange(-2, +3)};
    nextDotPositions.at(i) = m_dotPaths.at(i).GetNextPoint();
  }
  return nextDotPositions;
}

} // namespace GOOM::VISUAL_FX::CIRCLES
