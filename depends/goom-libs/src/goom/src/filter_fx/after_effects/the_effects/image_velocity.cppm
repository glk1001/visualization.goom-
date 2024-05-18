module;

#include <string>

export module Goom.FilterFx.AfterEffects.TheEffects.ImageVelocity;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.FilterUtils.ImageDisplacementList;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRandBase;

export namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

class ImageVelocity
{
public:
  ImageVelocity(const UTILS::MATH::IGoomRand& goomRand, const std::string& resourcesDirectory);

  auto SetRandomParams() -> void;

  [[nodiscard]] auto GetVelocity(const CoordsAndVelocity& coordsAndVelocity) const
      -> NormalizedCoords;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  FILTER_UTILS::ImageDisplacementList m_imageDisplacementList;
};

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

inline auto ImageVelocity::GetVelocity(const CoordsAndVelocity& coordsAndVelocity) const
    -> NormalizedCoords
{
  const auto displacementVector =
      m_imageDisplacementList.GetCurrentImageDisplacement().GetDisplacementVector(
          coordsAndVelocity.coords);

  return coordsAndVelocity.velocity + NormalizedCoords{displacementVector.x, displacementVector.y};
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
