#include "dot_paths.h"

//#undef NO_LOGGING

#include "logging.h"
#include "point2d.h"
#include "utils/math/paths.h"
#include "utils/t_values.h"

#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX::CIRCLES
{

using UTILS::Logging;
using UTILS::TValue;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::LerpedPath;
using UTILS::MATH::OscillatingPath;

DotPaths::DotPaths(const IGoomRand& goomRand,
                   const uint32_t numDots,
                   std::vector<Point2dInt>&& dotStartingPositions,
                   const Point2dInt& dotTarget,
                   const OscillatingPath::Params& dotPathParams) noexcept
  : m_goomRand{goomRand},
    m_numDots{numDots},
    m_dotStartingPositions{std::move(dotStartingPositions)},
    m_target{dotTarget},
    m_pathParams{dotPathParams},
    m_dotPaths{GetNewDotPaths(m_dotStartingPositions)}
{
}

auto DotPaths::SetTarget(const Point2dInt& target) -> void
{
  m_target = target;
  std::for_each(begin(m_dotPaths), end(m_dotPaths),
                [&target](OscillatingPath& path) { path.SetEndPos(target); });
  static constexpr float PROB_RANDOMIZE_POINTS = 0.3F;
  m_randomizePoints = m_goomRand.ProbabilityOf(PROB_RANDOMIZE_POINTS);
}

auto DotPaths::GetNewDotPaths(const std::vector<Point2dInt>& dotStartingPositions)
    -> std::vector<OscillatingPath>
{
  std::vector<OscillatingPath> dotPaths{};

  for (size_t i = 0; i < m_numDots; ++i)
  {
    static const std::vector<TValue::DelayPoint> delayPoints{
        {0.0F,   DELAY_TIME_AT_EDGE},
        {1.0F, DELAY_TIME_AT_CENTRE}
    };
    auto positionT = std::make_unique<TValue>(TValue::StepType::CONTINUOUS_REVERSIBLE,
                                              DEFAULT_POSITION_STEPS, delayPoints, 0.0F);
    dotPaths.emplace_back(dotStartingPositions.at(i), m_target, std::move(positionT), m_pathParams,
                          true);
  }

  return dotPaths;
}

auto DotPaths::GetNextDotPositions() const -> std::vector<Point2dInt>
{
  std::vector<Point2dInt> nextDotPositions(m_numDots);
  for (size_t i = 0; i < m_numDots; ++i)
  {
    nextDotPositions.at(i) = m_dotPaths.at(i).GetNextPoint();
    if (m_randomizePoints)
    {
      nextDotPositions.at(i).Translate(GetSmallRandomOffset());
    }
  }
  return nextDotPositions;
}

inline auto DotPaths::GetSmallRandomOffset() const -> Vec2dInt
{
  static constexpr int32_t MIN_VARIATION = -5;
  static constexpr int32_t MAX_VARIATION = +5;
  return {m_goomRand.GetRandInRange(MIN_VARIATION, MAX_VARIATION + 1),
          m_goomRand.GetRandInRange(MIN_VARIATION, MAX_VARIATION + 1)};
}

} // namespace GOOM::VISUAL_FX::CIRCLES
