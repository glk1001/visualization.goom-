module;

#include "goom/point2d.h"

export module Goom.FilterFx.FilterEffects.AdjustmentEffects.Scrunch;

import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.FilterFx.ZoomAdjustmentEffect;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRandBase;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class Scrunch : public IZoomAdjustmentEffect
{
public:
  explicit Scrunch(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Vec2dFlt override;

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
  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt;
};

inline auto Scrunch::GetZoomAdjustment(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto velocity = GetVelocity(coords);

  return {coords.GetX() * velocity.x, coords.GetY() * velocity.y};
}

inline auto Scrunch::GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto sqDistFromZero  = SqDistanceFromZero(coords);
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
