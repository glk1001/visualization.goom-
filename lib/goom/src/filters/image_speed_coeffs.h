#ifndef VISUALIZATION_GOOM_LIB_FILTERS_IMAGE_SPEED_COEFFS_H
#define VISUALIZATION_GOOM_LIB_FILTERS_IMAGE_SPEED_COEFFS_H

#include "goomutils/name_value_pairs.h"
#include "image_displacement_list.h"
#include "normalized_coords.h"
#include "speed_coefficients_effect.h"
#include "v2d.h"

#include <string>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

class ImageSpeedCoefficients : public ISpeedCoefficientsEffect
{
public:
  explicit ImageSpeedCoefficients(const std::string& resourcesDirectory);

  void SetRandomParams() override;

  [[nodiscard]] auto GetSpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                          float sqDistFromZero,
                                          const NormalizedCoords& coords) const -> V2dFlt override;

  [[nodiscard]] auto GetSpeedCoefficientsEffectNameValueParams() const
      -> UTILS::NameValuePairs override;

private:
  ImageDisplacementList m_imageDisplacementList;
};

inline auto ImageSpeedCoefficients::GetSpeedCoefficients(
    [[maybe_unused]] const V2dFlt& baseSpeedCoeffs,
    [[maybe_unused]] float sqDistFromZero,
    const NormalizedCoords& coords) const -> V2dFlt
{
  return m_imageDisplacementList.GetCurrentImageDisplacement().GetDisplacementVector(
      coords.ToFlt());
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif

#endif //VISUALIZATION_GOOM_LIB_FILTERS_IMAGE_SPEED_COEFFS_H
