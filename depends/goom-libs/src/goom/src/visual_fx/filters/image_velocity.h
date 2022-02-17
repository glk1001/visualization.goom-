#pragma once

#include "image_displacement_list.h"
#include "normalized_coords.h"
#include "point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

#include <string>

namespace GOOM::VISUAL_FX::FILTERS
{

class ImageVelocity
{
public:
  ImageVelocity(const std::string& resourcesDirectory, const UTILS::MATH::IGoomRand& goomRand);

  void SetRandomParams();

  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const -> Point2dFlt;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs;

private:
  const UTILS::MATH::IGoomRand& m_goomRand;
  ImageDisplacementList m_imageDisplacementList;
};

inline auto ImageVelocity::GetVelocity(const NormalizedCoords& coords) const -> Point2dFlt
{
  return m_imageDisplacementList.GetCurrentImageDisplacement().GetDisplacementVector(coords);
}

} // namespace GOOM::VISUAL_FX::FILTERS
