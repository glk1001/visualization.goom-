#pragma once

#include "normalized_coords.h"
#include "utils/name_value_pairs.h"
#include "v2d.h"

namespace GOOM::VISUAL_FX::FILTERS
{

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

} // namespace GOOM::VISUAL_FX::FILTERS
