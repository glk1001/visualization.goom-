#pragma once

#include "filter_fx/normalized_coords.h"
#include "filter_fx/speed_coefficients_effect.h"
#include "filter_fx/utils/image_displacement_list.h"
#include "point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

#include <string>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class ImageSpeedCoefficients : public ISpeedCoefficientsEffect
{
public:
  ImageSpeedCoefficients(const std::string& resourcesDirectory,
                         const GOOM::UTILS::MATH::IGoomRand& goomRand);

  auto SetRandomParams() -> void override;

  [[nodiscard]] auto GetSpeedCoefficients(const NormalizedCoords& coords,
                                          float sqDistFromZero,
                                          const Point2dFlt& baseSpeedCoeffs) const
      -> Point2dFlt override;

  [[nodiscard]] auto GetSpeedCoefficientsEffectNameValueParams() const
      -> GOOM::UTILS::NameValuePairs override;

private:
  const GOOM::UTILS::MATH::IGoomRand& m_goomRand;
  UTILS::ImageDisplacementList m_imageDisplacementList;
  auto DoSetRandomParams() -> void;
};

inline auto ImageSpeedCoefficients::GetSpeedCoefficients(
    const NormalizedCoords& coords,
    [[maybe_unused]] const float sqDistFromZero,
    [[maybe_unused]] const Point2dFlt& baseSpeedCoeffs) const -> Point2dFlt
{
  return m_imageDisplacementList.GetCurrentImageDisplacement().GetDisplacementVector(coords);
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
