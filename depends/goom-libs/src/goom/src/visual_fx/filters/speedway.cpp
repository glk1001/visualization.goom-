#include "speedway.h"

#include "utils/name_value_pairs.h"

#undef NDEBUG
#include <cassert>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
namespace FILTERS
{
#else
namespace GOOM::VISUAL_FX::FILTERS
{
#endif

using UTILS::IGoomRand;
using UTILS::NameValuePairs;

constexpr float X_DEFAULT_AMPLITUDE = 4.0F;
constexpr float Y_DEFAULT_AMPLITUDE = 1.0F;
constexpr IGoomRand::NumberRange<float> X_AMPLITUDE_RANGE = {+01.0F, +08.0F};
constexpr IGoomRand::NumberRange<float> Y_AMPLITUDE_RANGE = {-10.0F, +10.0F};

constexpr float PROB_AMPLITUDE_EQUAL = 0.5F;

Speedway::Speedway(Modes mode, const IGoomRand& goomRand) noexcept
  : m_mode{mode}, m_goomRand{goomRand}, m_params{X_DEFAULT_AMPLITUDE, Y_DEFAULT_AMPLITUDE}
{
}

void Speedway::SetRandomParams()
{
  switch (m_mode)
  {
    case Modes::MODE0:
      SetMode0RandomParams();
      break;
    case Modes::MODE1:
      SetMode1RandomParams();
      break;
    case Modes::MODE2:
      SetMode2RandomParams();
      break;
  }
}

void Speedway::SetMode0RandomParams()
{
  m_params.xAmplitude = m_goomRand.GetRandInRange(X_AMPLITUDE_RANGE);
  m_params.yAmplitude = m_goomRand.ProbabilityOf(PROB_AMPLITUDE_EQUAL) ? +1.0F : -1.0F;
}

void Speedway::SetMode1RandomParams()
{
  m_params.xAmplitude = m_goomRand.GetRandInRange(X_AMPLITUDE_RANGE);
  m_params.yAmplitude = m_goomRand.ProbabilityOf(PROB_AMPLITUDE_EQUAL)
                            ? m_params.xAmplitude
                            : m_goomRand.GetRandInRange(Y_AMPLITUDE_RANGE);
}

void Speedway::SetMode2RandomParams()
{
  m_params.xAmplitude = m_goomRand.GetRandInRange(X_AMPLITUDE_RANGE);
  m_params.yAmplitude = m_goomRand.ProbabilityOf(PROB_AMPLITUDE_EQUAL)
                            ? m_params.xAmplitude
                            : m_goomRand.GetRandInRange(Y_AMPLITUDE_RANGE);
}

auto Speedway::GetSpeedCoefficientsEffectNameValueParams() const -> NameValuePairs
{
  return NameValuePairs();
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::FILTERS
#endif
