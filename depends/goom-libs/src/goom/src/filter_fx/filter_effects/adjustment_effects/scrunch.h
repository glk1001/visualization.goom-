#pragma once

#include "filter_fx/common_types.h"
#include "filter_fx/normalized_coords.h"
#include "filter_fx/zoom_adjustment_effect.h"
#include "goom/point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class Scrunch : public IZoomAdjustmentEffect
{
public:
  explicit Scrunch(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords,
                                       float sqDistFromZero) const noexcept -> Point2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> UTILS::NameValuePairs override;

  struct Params
  {
    Amplitude amplitude;
  };
  [[nodiscard]] auto GetParams() const noexcept -> const Params&;

protected:
  auto SetParams(const Params& params) noexcept -> void;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  Params m_params;
};

inline auto Scrunch::GetZoomAdjustment([[maybe_unused]] const NormalizedCoords& coords,
                                       const float sqDistFromZero) const noexcept -> Point2dFlt
{
  const auto xZoomAdjustment = GetBaseZoomAdjustment().x + (m_params.amplitude.x * sqDistFromZero);
  const auto yZoomAdjustment = m_params.amplitude.y * xZoomAdjustment;
  return {xZoomAdjustment, yZoomAdjustment};
}

inline auto Scrunch::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline auto Scrunch::SetParams(const Params& params) noexcept -> void
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
