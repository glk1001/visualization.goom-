#include "scrunch.h"

#include "utils/goomrand.h"
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

using UTILS::GetRandInRange;
using UTILS::NameValuePairs;
using UTILS::NumberRange;

constexpr float X_DEFAULT_AMPLITUDE = 0.1F;
constexpr float Y_DEFAULT_AMPLITUDE = 1.0F;
constexpr NumberRange<float> X_AMPLITUDE_RANGE = {0.05F, 0.2F};
constexpr NumberRange<float> Y_AMPLITUDE_RANGE = {0.50F, 2.0F};

Scrunch::Scrunch() noexcept : m_params{X_DEFAULT_AMPLITUDE, Y_DEFAULT_AMPLITUDE}
{
}

void Scrunch::SetRandomParams()
{
  m_params.xAmplitude = GetRandInRange(X_AMPLITUDE_RANGE);
  m_params.yAmplitude = GetRandInRange(Y_AMPLITUDE_RANGE);
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
