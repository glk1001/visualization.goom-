#include "scrunch.h"

#include "utils/name_value_pairs.h"

#undef NDEBUG
#include <cassert>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

using UTILS::IGoomRand;
using UTILS::NameValuePairs;

constexpr float X_DEFAULT_AMPLITUDE = 0.1F;
constexpr float Y_DEFAULT_AMPLITUDE = 1.0F;
constexpr IGoomRand::NumberRange<float> X_AMPLITUDE_RANGE = {0.05F, 0.2F};
constexpr IGoomRand::NumberRange<float> Y_AMPLITUDE_RANGE = {0.50F, 2.0F};

Scrunch::Scrunch(IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand}, m_params{X_DEFAULT_AMPLITUDE, Y_DEFAULT_AMPLITUDE}
{
}

void Scrunch::SetRandomParams()
{
  m_params.xAmplitude = m_goomRand.GetRandInRange(X_AMPLITUDE_RANGE);
  m_params.yAmplitude = m_goomRand.GetRandInRange(Y_AMPLITUDE_RANGE);
}

auto Scrunch::GetSpeedCoefficientsEffectNameValueParams() const -> NameValuePairs
{
  return NameValuePairs();
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
