#include "filter_speedway.h"

#include "goomutils/goomrand.h"

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
using UTILS::NumberRange;

constexpr float X_DEFAULT_AMPLITUDE = 4.0F;
constexpr float Y_DEFAULT_AMPLITUDE = 1.0F;
constexpr NumberRange<float> X_AMPLITUDE_RANGE = {+01.0F, +8.0F};
constexpr NumberRange<float> Y_AMPLITUDE_RANGE = {-15.0F, +5.0F};

Speedway::Speedway() noexcept : m_params{X_DEFAULT_AMPLITUDE, Y_DEFAULT_AMPLITUDE}
{
}

void Speedway::SetRandomParams()
{
  m_params.xAmplitude = GetRandInRange(X_AMPLITUDE_RANGE);
  m_params.yAmplitude = GetRandInRange(Y_AMPLITUDE_RANGE);
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
