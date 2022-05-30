#include "speedway.h"

#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;

static constexpr float X_DEFAULT_AMPLITUDE = 4.0F;
static constexpr float Y_DEFAULT_AMPLITUDE = 1.0F;
static constexpr IGoomRand::NumberRange<float> X_AMPLITUDE_RANGE = {+01.0F, +08.0F};
static constexpr IGoomRand::NumberRange<float> Y_AMPLITUDE_RANGE = {-10.0F, +10.0F};

static constexpr float PROB_AMPLITUDE_EQUAL = 0.5F;

Speedway::Speedway(const Modes mode, const IGoomRand& goomRand) noexcept
  : m_mode{mode}, m_goomRand{goomRand}, m_params{X_DEFAULT_AMPLITUDE, Y_DEFAULT_AMPLITUDE}
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
  const float xAmplitude = m_goomRand.GetRandInRange(X_AMPLITUDE_RANGE);
  const float yAmplitude = m_goomRand.ProbabilityOf(PROB_AMPLITUDE_EQUAL) ? +1.0F : -1.0F;

  SetParams({xAmplitude, yAmplitude});
}

auto Speedway::SetMode1RandomParams() -> void
{
  const float xAmplitude = m_goomRand.GetRandInRange(X_AMPLITUDE_RANGE);
  const float yAmplitude = m_goomRand.ProbabilityOf(PROB_AMPLITUDE_EQUAL)
                               ? xAmplitude
                               : m_goomRand.GetRandInRange(Y_AMPLITUDE_RANGE);

  SetParams({xAmplitude, yAmplitude});
}

auto Speedway::SetMode2RandomParams() -> void
{
  const float xAmplitude = m_goomRand.GetRandInRange(X_AMPLITUDE_RANGE);
  const float yAmplitude = m_goomRand.ProbabilityOf(PROB_AMPLITUDE_EQUAL)
                               ? xAmplitude
                               : m_goomRand.GetRandInRange(Y_AMPLITUDE_RANGE);

  SetParams({xAmplitude, yAmplitude});
}

auto Speedway::GetSpeedCoefficientsEffectNameValueParams() const -> NameValuePairs
{
  return NameValuePairs();
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
