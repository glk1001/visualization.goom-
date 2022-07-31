#pragma once

#include "filter_fx/normalized_coords.h"
#include "point2d.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX
{

class ISpeedCoefficientsEffect
{
public:
  ISpeedCoefficientsEffect() noexcept                                          = default;
  ISpeedCoefficientsEffect(const ISpeedCoefficientsEffect&) noexcept           = default;
  ISpeedCoefficientsEffect(ISpeedCoefficientsEffect&&) noexcept                = default;
  virtual ~ISpeedCoefficientsEffect() noexcept                                 = default;
  auto operator=(const ISpeedCoefficientsEffect&) -> ISpeedCoefficientsEffect& = default;
  auto operator=(ISpeedCoefficientsEffect&&) -> ISpeedCoefficientsEffect&      = default;

  virtual auto SetRandomParams() -> void = 0;

  [[nodiscard]] virtual auto GetSpeedCoefficients(const Point2dFlt& baseSpeedCoeffs,
                                                  float sqDistFromZero,
                                                  const NormalizedCoords& coords) const
      -> Point2dFlt = 0;

  [[nodiscard]] virtual auto GetSpeedCoefficientsEffectNameValueParams() const
      -> UTILS::NameValuePairs = 0;
};

} // namespace GOOM::FILTER_FX
