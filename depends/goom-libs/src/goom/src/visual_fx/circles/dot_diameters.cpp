#include "dot_diameters.h"

#include "goom_config.h"

namespace GOOM::VISUAL_FX::CIRCLES
{

DotDiameters::DotDiameters(const UTILS::MATH::IGoomRand& goomRand,
                           const DotProperties& dotProperties) noexcept
  : m_goomRand{&goomRand},
    m_numDots{dotProperties.numDots},
    m_minDiameter{dotProperties.minDiameter + MIN_DIAMETER_EXTRA},
    m_maxDiameter{dotProperties.maxDiameter}
{
  Expects(dotProperties.numDots > 0);
  Expects((dotProperties.minDiameter + MIN_DIAMETER_EXTRA) <= dotProperties.maxDiameter);
  ChangeDiameters();
}

auto DotDiameters::GetInitialDiameters(const NumDotsAndMaxDiameter& numDotsAndMaxDiameter) noexcept
    -> std::vector<uint32_t>
{
  auto diameters = std::vector<uint32_t>(numDotsAndMaxDiameter.numDots);
  std::fill(begin(diameters), end(diameters), numDotsAndMaxDiameter.maxDiameter);
  return diameters;
}

auto DotDiameters::ChangeDiameters() noexcept -> void
{
  if (m_goomRand->ProbabilityOf(PROB_FIXED_DIAMETER))
  {
    ChangeToFixedDiameters();
  }
  else
  {
    ChangeToVariableDiameters();
  }
}

auto DotDiameters::ChangeToFixedDiameters() noexcept -> void
{
  const auto fixedDiameter = m_goomRand->GetRandInRange(m_minDiameter, m_maxDiameter + 1);

  std::fill(begin(m_diameters), end(m_diameters), fixedDiameter);
}

auto DotDiameters::ChangeToVariableDiameters() noexcept -> void
{
  const auto smallDiameter = m_goomRand->GetRandInRange(m_minDiameter, m_maxDiameter - 1);

  static constexpr auto INCREASED_DIAMETER = 3U;
  const auto minLargerDiameter = std::min(m_maxDiameter, smallDiameter + INCREASED_DIAMETER);
  const auto largerDotDiameter = m_goomRand->GetRandInRange(minLargerDiameter, m_maxDiameter + 1);

  static constexpr auto MIN_LARGER_DIAMETER_FREQ = 2U;
  static constexpr auto MAX_LARGER_DIAMETER_FREQ = 5U;
  const auto largerDiameterEvery =
      m_goomRand->GetRandInRange(MIN_LARGER_DIAMETER_FREQ, MAX_LARGER_DIAMETER_FREQ + 1);

  for (auto i = 1U; i < m_numDots; ++i)
  {
    m_diameters.at(i) = 0 == (i % largerDiameterEvery) ? largerDotDiameter : smallDiameter;
  }
}

} // namespace GOOM::VISUAL_FX::CIRCLES
