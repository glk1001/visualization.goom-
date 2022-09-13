#pragma once

#include "filter_fx/normalized_coords.h"
#include "point2d.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX
{

class IZoomInCoefficientsEffect
{
public:
  IZoomInCoefficientsEffect() noexcept                                           = default;
  IZoomInCoefficientsEffect(const IZoomInCoefficientsEffect&) noexcept           = default;
  IZoomInCoefficientsEffect(IZoomInCoefficientsEffect&&) noexcept                = default;
  virtual ~IZoomInCoefficientsEffect() noexcept                                  = default;
  auto operator=(const IZoomInCoefficientsEffect&) -> IZoomInCoefficientsEffect& = default;
  auto operator=(IZoomInCoefficientsEffect&&) -> IZoomInCoefficientsEffect&      = default;

  virtual auto SetRandomParams() -> void = 0;

  [[nodiscard]] virtual auto GetZoomInCoefficients(const NormalizedCoords& coords,
                                                   float sqDistFromZero,
                                                   const Point2dFlt& baseZoomInCoeffs) const
      -> Point2dFlt = 0;

  [[nodiscard]] virtual auto GetZoomInCoefficientsEffectNameValueParams() const
      -> GOOM::UTILS::NameValuePairs = 0;
};

} // namespace GOOM::FILTER_FX
