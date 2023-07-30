#pragma once

#include "filter_fx/filter_utils/image_displacement_list.h"
#include "filter_fx/normalized_coords.h"
#include "filter_fx/zoom_adjustment_effect.h"
#include "point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

#include <string>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class ImageZoomAdjustment : public IZoomAdjustmentEffect
{
public:
  ImageZoomAdjustment(const std::string& resourcesDirectory,
                      const GOOM::UTILS::MATH::IGoomRand& goomRand);

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords,
                                       float sqDistFromZero) const noexcept -> Point2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> GOOM::UTILS::NameValuePairs override;

private:
  const GOOM::UTILS::MATH::IGoomRand* m_goomRand;
  FILTER_UTILS::ImageDisplacementList m_imageDisplacementList;
  auto DoSetRandomParams() noexcept -> void;
};

inline auto ImageZoomAdjustment::GetZoomAdjustment(
    const NormalizedCoords& coords, [[maybe_unused]] float sqDistFromZero) const noexcept
    -> Point2dFlt
{
  return m_imageDisplacementList.GetCurrentImageDisplacement().GetDisplacementVector(coords);
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
