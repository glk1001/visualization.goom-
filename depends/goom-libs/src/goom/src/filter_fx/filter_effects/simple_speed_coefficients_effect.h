#pragma once

#include "filter_fx/normalized_coords.h"
#include "filter_fx/speed_coefficients_effect.h"
#include "point2d.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class SimpleSpeedCoefficientsEffect : public ISpeedCoefficientsEffect
{
public:
  SimpleSpeedCoefficientsEffect() noexcept = default;

  auto SetRandomParams() -> void override;

  [[nodiscard]] auto GetSpeedCoefficients(const NormalizedCoords& coords,
                                          float sqDistFromZero,
                                          const Point2dFlt& baseSpeedCoeffs) const
      -> Point2dFlt override;

  [[nodiscard]] auto GetSpeedCoefficientsEffectNameValueParams() const
      -> GOOM::UTILS::NameValuePairs override;
};

inline auto SimpleSpeedCoefficientsEffect::GetSpeedCoefficients(
    [[maybe_unused]] const NormalizedCoords& coords,
    [[maybe_unused]] const float sqDistFromZero,
    const Point2dFlt& baseSpeedCoeffs) const -> Point2dFlt
{
  return baseSpeedCoeffs;
}

inline auto SimpleSpeedCoefficientsEffect::SetRandomParams() -> void
{
  // do nothing
}

inline auto SimpleSpeedCoefficientsEffect::GetSpeedCoefficientsEffectNameValueParams() const
    -> GOOM::UTILS::NameValuePairs
{
  return GOOM::UTILS::NameValuePairs();
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
