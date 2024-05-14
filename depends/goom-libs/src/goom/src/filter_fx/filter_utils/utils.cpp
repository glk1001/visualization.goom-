#include "utils.h"

#include "filter_fx/normalized_coords.h"

#include <algorithm>

import Goom.Utils.Math.GoomRandBase;

namespace GOOM::FILTER_FX::FILTER_UTILS
{

using UTILS::MATH::HALF;
using UTILS::MATH::IGoomRand;

RandomViewport::RandomViewport(const IGoomRand& goomRand, const Bounds& bounds) noexcept
  : m_goomRand{&goomRand}, m_bounds{bounds}
{
}

auto RandomViewport::GetRandomViewport() const noexcept -> Viewport
{
  if (m_goomRand->ProbabilityOf(m_probNoViewport))
  {
    return Viewport{};
  }

  if (m_goomRand->ProbabilityOf(m_bounds.probUseCentredSides))
  {
    return GetRandomCentredViewport();
  }

  return GetRandomUncentredViewport();
}

auto RandomViewport::GetRandomUncentredViewport() const noexcept -> Viewport
{
  const auto useSquareViewport = m_goomRand->ProbabilityOf(m_probSquareViewport);

  const auto xMin = m_goomRand->GetRandInRange(m_bounds.rect.minMaxXMin.minValue,
                                               m_bounds.rect.minMaxXMin.maxValue);
  const auto xMax = std::max(xMin + m_bounds.minSideLength,
                             m_goomRand->GetRandInRange(m_bounds.rect.minMaxXMax.minValue,
                                                        m_bounds.rect.minMaxXMax.maxValue));
  const auto yMin = useSquareViewport
                        ? xMin
                        : m_goomRand->GetRandInRange(m_bounds.rect.minMaxYMin.minValue,
                                                     m_bounds.rect.minMaxYMin.maxValue);
  const auto yMax = useSquareViewport
                        ? xMax
                        : std::max(yMin + m_bounds.minSideLength,
                                   m_goomRand->GetRandInRange(m_bounds.rect.minMaxYMax.minValue,
                                                              m_bounds.rect.minMaxYMax.maxValue));

  return Viewport{
      Viewport::Rectangle{{xMin, yMin}, {xMax, yMax}}
  };
}

auto RandomViewport::GetRandomCentredViewport() const noexcept -> Viewport
{
  const auto useSquareViewport = m_goomRand->ProbabilityOf(m_probSquareViewport);

  const auto width  = m_goomRand->GetRandInRange(m_bounds.sides.minMaxWidth.minValue,
                                                m_bounds.sides.minMaxWidth.maxValue);
  const auto height = useSquareViewport
                          ? width
                          : m_goomRand->GetRandInRange(m_bounds.sides.minMaxHeight.minValue,
                                                       m_bounds.sides.minMaxHeight.maxValue);

  const auto halfWidth  = HALF * width;
  const auto halfHeight = HALF * height;

  const auto xMin = -halfWidth;
  const auto xMax = +halfWidth;
  const auto yMin = -halfHeight;
  const auto yMax = +halfHeight;

  return Viewport{
      Viewport::Rectangle{{xMin, yMin}, {xMax, yMax}}
  };
}

} // namespace GOOM::FILTER_FX::FILTER_UTILS
