#pragma once

#include "filter_fx/normalized_coords.h"
#include "filter_fx/zoom_adjustment_effect.h"
#include "goom/point2d.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class UniformZoomAdjustmentEffect : public IZoomAdjustmentEffect
{
public:
  UniformZoomAdjustmentEffect() noexcept = default;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords,
                                       float sqDistFromZero) const noexcept -> Point2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> GOOM::UTILS::NameValuePairs override;
};

inline auto UniformZoomAdjustmentEffect::GetZoomAdjustment(
    [[maybe_unused]] const NormalizedCoords& coords,
    [[maybe_unused]] const float sqDistFromZero) const noexcept -> Point2dFlt
{
  return GetBaseZoomAdjustment();
}

inline auto UniformZoomAdjustmentEffect::SetRandomParams() noexcept -> void
{
  // do nothing
}

inline auto UniformZoomAdjustmentEffect::GetZoomAdjustmentEffectNameValueParams() const noexcept
    -> GOOM::UTILS::NameValuePairs
{
  return GOOM::UTILS::NameValuePairs();
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
