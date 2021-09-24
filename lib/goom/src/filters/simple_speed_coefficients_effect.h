#ifndef VISUALIZATION_GOOM_LIB_FILTERS_SIMPLE_SPEED_COEFFICIENTS_EFFECT_H
#define VISUALIZATION_GOOM_LIB_FILTERS_SIMPLE_SPEED_COEFFICIENTS_EFFECT_H

#include "goomutils/name_value_pairs.h"
#include "normalized_coords.h"
#include "speed_coefficients_effect.h"
#include "v2d.h"

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

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

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif

#endif //VISUALIZATION_GOOM_LIB_FILTERS_SIMPLE_SPEED_COEFFICIENTS_EFFECT_H
