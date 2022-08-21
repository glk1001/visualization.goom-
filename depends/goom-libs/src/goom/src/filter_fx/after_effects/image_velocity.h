#pragma once

#include "filter_fx/normalized_coords.h"
#include "filter_fx/utils/image_displacement_list.h"
#include "point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

#include <string>

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

class ImageVelocity
{
public:
  ImageVelocity(const GOOM::UTILS::MATH::IGoomRand& goomRand,
                const std::string& resourcesDirectory);

  auto SetRandomParams() -> void;

  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords,
                                 const NormalizedCoords& velocity) const -> NormalizedCoords;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> GOOM::UTILS::NameValuePairs;

private:
  const GOOM::UTILS::MATH::IGoomRand& m_goomRand;
  UTILS::ImageDisplacementList m_imageDisplacementList;
};

inline auto ImageVelocity::GetVelocity(const NormalizedCoords& coords,
                                       const NormalizedCoords& velocity) const -> NormalizedCoords
{
  return velocity +
         NormalizedCoords{
             m_imageDisplacementList.GetCurrentImageDisplacement().GetDisplacementVector(coords)};
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS