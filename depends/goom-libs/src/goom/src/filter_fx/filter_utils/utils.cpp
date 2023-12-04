#include "utils.h"

#include "filter_fx/normalized_coords.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"

namespace GOOM::FILTER_FX::FILTER_UTILS
{

using UTILS::MATH::HALF;

RandomViewport::RandomViewport(const UTILS::MATH::IGoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_minMaxXWidths{MIN_X_WIDTH, MAX_X_WIDTH},
    m_minMaxYHeights{MIN_Y_HEIGHT, MAX_Y_HEIGHT}
{
}

auto RandomViewport::GetRandomViewport() const noexcept -> Viewport
{
  if (m_goomRand->ProbabilityOf(m_probNoViewport))
  {
    return Viewport{};
  }

  const auto xHalfWidth =
      HALF * m_goomRand->GetRandInRange(m_minMaxXWidths.minValue, m_minMaxXWidths.maxValue);
  const auto yHalfHeight =
      m_goomRand->ProbabilityOf(m_probSquareViewport)
          ? xHalfWidth
          : HALF * m_goomRand->GetRandInRange(m_minMaxYHeights.minValue, m_minMaxYHeights.maxValue);

  const auto xCentre =
      m_goomRand->GetRandInRange((NormalizedCoords::MIN_COORD + MARGIN) + xHalfWidth,
                                 (NormalizedCoords::MAX_COORD - MARGIN) - xHalfWidth);
  const auto yCentre =
      m_goomRand->GetRandInRange((NormalizedCoords::MIN_COORD + MARGIN) + yHalfHeight,
                                 (NormalizedCoords::MAX_COORD - MARGIN) - yHalfHeight);

  const auto xMin = xCentre - xHalfWidth;
  const auto xMax = xCentre + xHalfWidth;
  const auto yMin = yCentre - yHalfHeight;
  const auto yMax = yCentre + yHalfHeight;

  Ensures(xMin >= NormalizedCoords::MIN_COORD + MARGIN);
  Ensures(yMin >= NormalizedCoords::MIN_COORD + MARGIN);
  Ensures(xMax <= NormalizedCoords::MAX_COORD - MARGIN);
  Ensures(yMax <= NormalizedCoords::MAX_COORD - MARGIN);

  return Viewport{
      Viewport::Rectangle{{xMin, yMin}, {xMax, yMax}}
  };
}

} // namespace GOOM::FILTER_FX::FILTER_UTILS
