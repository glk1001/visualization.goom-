#include "noise.h"

#include "utils/randutils.h"
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

using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::GetRandInRange;
using UTILS::NameValuePairs;
using UTILS::NumberRange;

constexpr float DEFAULT_NOISE_FACTOR = 0.01F;
constexpr NumberRange<float> NOISE_FACTOR_RANGE = {0.05F, 0.5F};

Noise::Noise() noexcept : m_params{DEFAULT_NOISE_FACTOR}
{
}

void Noise::SetRandomParams()
{
  m_params.noiseFactor = GetRandInRange(NOISE_FACTOR_RANGE);
}

auto Noise::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  const std::string fullParamGroup = GetFullParamGroup({paramGroup, "noise"});
  return {
      GetPair(fullParamGroup, "noise factor", m_params.noiseFactor),
  };
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
