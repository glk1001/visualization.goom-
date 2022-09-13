#pragma once

#include "filter_fx/normalized_coords.h"
#include "filter_fx/zoom_in_coefficients_effect.h"
#include "point2d.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class UniformZoomInCoefficientsEffect : public IZoomInCoefficientsEffect
{
public:
  UniformZoomInCoefficientsEffect() noexcept = default;

  auto SetRandomParams() -> void override;

  [[nodiscard]] auto GetZoomInCoefficients(const NormalizedCoords& coords,
                                           float sqDistFromZero,
                                           const Point2dFlt& baseZoomInCoeffs) const
      -> Point2dFlt override;

  [[nodiscard]] auto GetZoomInCoefficientsEffectNameValueParams() const
      -> GOOM::UTILS::NameValuePairs override;
};

inline auto UniformZoomInCoefficientsEffect::GetZoomInCoefficients(
    [[maybe_unused]] const NormalizedCoords& coords,
    [[maybe_unused]] float sqDistFromZero,
    const Point2dFlt& baseZoomInCoeffs) const -> Point2dFlt
{
  return baseZoomInCoeffs;
}

inline auto UniformZoomInCoefficientsEffect::SetRandomParams() -> void
{
  // do nothing
}

inline auto UniformZoomInCoefficientsEffect::GetZoomInCoefficientsEffectNameValueParams() const
    -> GOOM::UTILS::NameValuePairs
{
  return GOOM::UTILS::NameValuePairs();
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
