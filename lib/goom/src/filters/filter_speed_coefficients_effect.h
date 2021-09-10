#ifndef VISUALIZATION_GOOM_FILTER_SPEED_COEFFICIENTS_EFFECT_H
#define VISUALIZATION_GOOM_FILTER_SPEED_COEFFICIENTS_EFFECT_H

#include "filter_normalized_coords.h"
#include "goomutils/name_value_pairs.h"
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

class ISpeedCoefficientsEffect
{
public:
  ISpeedCoefficientsEffect() noexcept = default;
  ISpeedCoefficientsEffect(const ISpeedCoefficientsEffect&) noexcept = default;
  ISpeedCoefficientsEffect(ISpeedCoefficientsEffect&&) noexcept = default;
  virtual ~ISpeedCoefficientsEffect() noexcept = default;
  auto operator=(const ISpeedCoefficientsEffect&) -> ISpeedCoefficientsEffect& = default;
  auto operator=(ISpeedCoefficientsEffect&&) -> ISpeedCoefficientsEffect& = default;

  virtual void SetRandomParams() = 0;

  [[nodiscard]] virtual auto GetSpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                                  float sqDistFromZero,
                                                  const NormalizedCoords& coords) const
      -> V2dFlt = 0;

  [[nodiscard]] virtual auto GetSpeedCoefficientsEffectNameValueParams() const
      -> UTILS::NameValuePairs = 0;
};

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif

#endif //VISUALIZATION_GOOM_FILTER_SPEED_COEFFICIENTS_EFFECT_H
