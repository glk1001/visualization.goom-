#include "dot_diameters.h"

#include "goom_config.h"

namespace GOOM::VISUAL_FX::CIRCLES
{

DotDiameters::DotDiameters(const UTILS::MATH::IGoomRand& goomRand,
                           const uint32_t numDots,
                           const uint32_t minDotDiameter,
                           const uint32_t maxDotDiameter) noexcept
  : m_goomRand{goomRand},
    m_numDots{numDots},
    m_minDotDiameter{minDotDiameter + MIN_DIAMETER_EXTRA},
    m_maxDotDiameter{maxDotDiameter},
    m_dotDiameters{GetInitialDotDiameters(m_numDots, m_maxDotDiameter)}
{
  Expects(m_minDotDiameter <= m_maxDotDiameter);
  ChangeDotDiameters();
}

auto DotDiameters::GetInitialDotDiameters(const uint32_t numDots, const uint32_t maxDotDiameter)
    -> std::vector<uint32_t>
{
  std::vector<uint32_t> diameters(numDots);
  std::fill(begin(diameters), end(diameters), maxDotDiameter);
  return diameters;
}

void DotDiameters::ChangeDotDiameters()
{
  if (m_goomRand.ProbabilityOf(PROB_FIXED_DIAMETER))
  {
    ChangeToFixedDotDiameters();
  }
  else
  {
    ChangeToVariableDotDiameters();
  }
}

void DotDiameters::ChangeToFixedDotDiameters()
{
  const uint32_t fixedDotDiameter =
      m_goomRand.GetRandInRange(m_minDotDiameter, m_maxDotDiameter + 1);

  for (auto& dotDiameter : m_dotDiameters)
  {
    dotDiameter = fixedDotDiameter;
  }
}

void DotDiameters::ChangeToVariableDotDiameters()
{
  const uint32_t smallDotDiameter =
      m_goomRand.GetRandInRange(m_minDotDiameter, m_maxDotDiameter - 1);

  static constexpr uint32_t INCREASED_DIAMETER = 3;
  const uint32_t minLargerDotDiameter =
      std::min(m_maxDotDiameter, smallDotDiameter + INCREASED_DIAMETER);
  const uint32_t largerDotDiameter =
      m_goomRand.GetRandInRange(minLargerDotDiameter, m_maxDotDiameter + 1);

  static constexpr uint32_t MIN_LARGER_DIAMETER_FREQ = 2;
  static constexpr uint32_t MAX_LARGER_DIAMETER_FREQ = 5;
  const size_t largerDiameterEvery =
      m_goomRand.GetRandInRange(MIN_LARGER_DIAMETER_FREQ, MAX_LARGER_DIAMETER_FREQ + 1);

  for (size_t i = 1; i < m_numDots; ++i)
  {
    m_dotDiameters.at(i) = 0 == (i % largerDiameterEvery) ? largerDotDiameter : smallDotDiameter;
  }
}

} // namespace GOOM::VISUAL_FX::CIRCLES
