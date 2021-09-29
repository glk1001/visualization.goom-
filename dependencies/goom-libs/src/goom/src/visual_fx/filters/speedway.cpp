#include "speedway.h"

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

constexpr float X_DEFAULT_AMPLITUDE = 4.0F;
constexpr float Y_DEFAULT_AMPLITUDE = 1.0F;
constexpr NumberRange<float> X_AMPLITUDE_RANGE = {+1.0F, +8.0F};
constexpr NumberRange<float> Y_AMPLITUDE_RANGE = {-8.0F, +1.0F};

Speedway::Speedway(Modes mode) noexcept : m_mode{mode}, m_params{X_DEFAULT_AMPLITUDE, Y_DEFAULT_AMPLITUDE}
{
}

void Speedway::SetRandomParams()
{
  if (m_mode == Modes::MODE0)
  {
    SetMode0RandomParams();
  }
  else
  {
    SetMode1RandomParams();
  }
}

void Speedway::SetMode0RandomParams()
{
  m_params.xAmplitude = GetRandInRange(X_AMPLITUDE_RANGE);
  m_params.yAmplitude = GetRandInRange(Y_AMPLITUDE_RANGE);
}

void Speedway::SetMode1RandomParams()
{
  m_params.xAmplitude = GetRandInRange(X_AMPLITUDE_RANGE);
  m_params.yAmplitude = GetRandInRange(Y_AMPLITUDE_RANGE);
}

auto Speedway::GetSpeedCoefficientsEffectNameValueParams() const -> NameValuePairs
{
  return NameValuePairs();
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
