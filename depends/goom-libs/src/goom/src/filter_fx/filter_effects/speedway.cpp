#include "speedway.h"

#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;

static constexpr auto X_DEFAULT_AMPLITUDE = 4.0F;
static constexpr auto Y_DEFAULT_AMPLITUDE = 1.0F;
static constexpr auto X_AMPLITUDE_RANGE   = IGoomRand::NumberRange<float>{+01.0F, +08.0F};
static constexpr auto Y_AMPLITUDE_RANGE   = IGoomRand::NumberRange<float>{-10.0F, +10.0F};
static constexpr auto DEFAULT_T_FREQ      = 1.0F;
static constexpr auto T_FREQ_RANGE        = IGoomRand::NumberRange<float>{1.0F, 50.0F};

static constexpr auto PROB_AMPLITUDE_EQUAL = 0.5F;
static constexpr auto PROB_FLIP_Y          = 0.0F;

Speedway::Speedway(const Modes mode, const IGoomRand& goomRand) noexcept
  : m_mode{mode},
    m_goomRand{goomRand},
    m_params{X_DEFAULT_AMPLITUDE, Y_DEFAULT_AMPLITUDE, DEFAULT_T_FREQ, false}
{
}

auto Speedway::SetRandomParams() -> void
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
    default:
      throw std::logic_error("Unexpected Modes enum.");
  }
}

auto Speedway::SetMode0RandomParams() -> void
{
  const auto xAmplitude = m_goomRand.GetRandInRange(X_AMPLITUDE_RANGE);
  const auto yAmplitude = m_goomRand.ProbabilityOf(PROB_AMPLITUDE_EQUAL) ? +1.0F : -1.0F;

  SetParams({xAmplitude, yAmplitude, DEFAULT_T_FREQ, false});
}

auto Speedway::SetMode1RandomParams() -> void
{
  const auto xAmplitude = m_goomRand.GetRandInRange(X_AMPLITUDE_RANGE);
  const auto yAmplitude = m_goomRand.ProbabilityOf(PROB_AMPLITUDE_EQUAL)
                              ? xAmplitude
                              : m_goomRand.GetRandInRange(Y_AMPLITUDE_RANGE);

  SetParams({xAmplitude, yAmplitude, DEFAULT_T_FREQ, false});
}

auto Speedway::SetMode2RandomParams() -> void
{
  const auto xAmplitude = m_goomRand.GetRandInRange(X_AMPLITUDE_RANGE);
  const auto yAmplitude = m_goomRand.ProbabilityOf(PROB_AMPLITUDE_EQUAL)
                              ? xAmplitude
                              : m_goomRand.GetRandInRange(Y_AMPLITUDE_RANGE);
  const auto tFreq      = m_goomRand.GetRandInRange(T_FREQ_RANGE);
  const auto flipXY     = m_goomRand.ProbabilityOf(PROB_FLIP_Y);

  SetParams({xAmplitude, yAmplitude, tFreq, flipXY});
}

auto Speedway::GetSpeedCoefficientsEffectNameValueParams() const -> NameValuePairs
{
  return NameValuePairs();
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
