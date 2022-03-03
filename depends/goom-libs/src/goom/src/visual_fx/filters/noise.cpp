#include "noise.h"

#include "utils/name_value_pairs.h"

namespace GOOM::VISUAL_FX::FILTERS
{

using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;

static constexpr float DEFAULT_NOISE_FACTOR = 0.01F;
static constexpr IGoomRand::NumberRange<float> NOISE_FACTOR_RANGE = {0.001F, 0.100F};

Noise::Noise(const IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand}, m_params{DEFAULT_NOISE_FACTOR}
{
}

void Noise::SetRandomParams()
{
  SetParams({m_goomRand.GetRandInRange(NOISE_FACTOR_RANGE)});
}

auto Noise::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  const std::string fullParamGroup = GetFullParamGroup({paramGroup, "noise"});
  return {
      GetPair(fullParamGroup, "noise factor", m_params.noiseFactor),
  };
}

} // namespace GOOM::VISUAL_FX::FILTERS
