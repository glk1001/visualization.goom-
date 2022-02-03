#pragma once

#include "image_displacement_list.h"
#include "normalized_coords.h"
#include "point2d.h"
#include "speed_coefficients_effect.h"
#include "utils/goom_rand_base.h"
#include "utils/name_value_pairs.h"

#include <string>

namespace GOOM::VISUAL_FX::FILTERS
{

class ImageSpeedCoefficients : public ISpeedCoefficientsEffect
{
public:
  ImageSpeedCoefficients(const std::string& resourcesDirectory, const UTILS::IGoomRand& goomRand);

  void SetRandomParams() override;

  [[nodiscard]] auto GetSpeedCoefficients(const Point2dFlt& baseSpeedCoeffs,
                                          float sqDistFromZero,
                                          const NormalizedCoords& coords) const
      -> Point2dFlt override;

  [[nodiscard]] auto GetSpeedCoefficientsEffectNameValueParams() const
      -> UTILS::NameValuePairs override;

private:
  const UTILS::IGoomRand& m_goomRand;
  ImageDisplacementList m_imageDisplacementList;
  void DoSetRandomParams();
};

inline auto ImageSpeedCoefficients::GetSpeedCoefficients(
    [[maybe_unused]] const Point2dFlt& baseSpeedCoeffs,
    [[maybe_unused]] const float sqDistFromZero,
    const NormalizedCoords& coords) const -> Point2dFlt
{
  return m_imageDisplacementList.GetCurrentImageDisplacement().GetDisplacementVector(coords);
}

} // namespace GOOM::VISUAL_FX::FILTERS
