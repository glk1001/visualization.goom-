module;

#include "goom/goom_types.h"
#include "goom/point2d.h"

export module Goom.FilterFx.FilterEffects.AdjustmentEffects.YOnly;

import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.FilterFx.ZoomAdjustmentEffect;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRandBase;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class YOnly : public IZoomAdjustmentEffect
{
public:
  explicit YOnly(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Vec2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> UTILS::NameValuePairs override;

  enum class YOnlyEffect : UnderlyingEnumType
  {
    NONE,
    X_SIN_Y_SIN,
    X_SIN_Y_COS,
    X_COS_Y_SIN,
    X_COS_Y_COS,
  };
  struct YOnlyXYEffect
  {
    YOnlyEffect xEffect;
    YOnlyEffect yEffect;
  };
  struct Params
  {
    YOnlyXYEffect xyEffect;
    FrequencyFactor frequencyFactor;
    Amplitude amplitude;
  };
  [[nodiscard]] auto GetParams() const noexcept -> const Params&;

protected:
  auto SetParams(const Params& params) noexcept -> void;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  Params m_params;
  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt;
  [[nodiscard]] auto GetYOnlyZoomAdjustmentMultiplier(YOnlyEffect effect,
                                                      const NormalizedCoords& coords) const noexcept
      -> float;
};

inline auto YOnly::GetZoomAdjustment(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto velocity = GetVelocity(coords);

  return {coords.GetX() * velocity.x, coords.GetY() * velocity.y};
}

inline auto YOnly::GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto xZoomAdjustment = GetBaseZoomAdjustment().x * m_params.amplitude.x *
                               GetYOnlyZoomAdjustmentMultiplier(m_params.xyEffect.xEffect, coords);
  if (m_params.xyEffect.yEffect == YOnlyEffect::NONE)
  {
    return {xZoomAdjustment, xZoomAdjustment};
  }

  return {xZoomAdjustment,
          GetBaseZoomAdjustment().y * m_params.amplitude.y *
              GetYOnlyZoomAdjustmentMultiplier(m_params.xyEffect.yEffect, coords)};
}

inline auto YOnly::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline auto YOnly::SetParams(const Params& params) noexcept -> void
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
