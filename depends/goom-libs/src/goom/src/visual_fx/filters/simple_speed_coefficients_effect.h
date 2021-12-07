#pragma once

#include "normalized_coords.h"
#include "speed_coefficients_effect.h"
#include "utils/name_value_pairs.h"
#include "v2d.h"

namespace GOOM::VISUAL_FX::FILTERS
{

class SimpleSpeedCoefficientsEffect : public ISpeedCoefficientsEffect
{
public:
  SimpleSpeedCoefficientsEffect() noexcept = default;

  void SetRandomParams() override;

  [[nodiscard]] auto GetSpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                          float sqDistFromZero,
                                          const NormalizedCoords& coords) const -> V2dFlt override;

  [[nodiscard]] auto GetSpeedCoefficientsEffectNameValueParams() const
      -> UTILS::NameValuePairs override;
};

inline auto SimpleSpeedCoefficientsEffect::GetSpeedCoefficients(
    const V2dFlt& baseSpeedCoeffs,
    [[maybe_unused]] const float sqDistFromZero,
    [[maybe_unused]] const NormalizedCoords& coords) const -> V2dFlt
{
  return baseSpeedCoeffs;
}

inline void SimpleSpeedCoefficientsEffect::SetRandomParams()
{
  // do nothing
}

inline auto SimpleSpeedCoefficientsEffect::GetSpeedCoefficientsEffectNameValueParams() const
    -> UTILS::NameValuePairs
{
  return UTILS::NameValuePairs();
}

} // namespace GOOM::VISUAL_FX::FILTERS
