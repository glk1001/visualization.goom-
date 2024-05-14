module;

#include "goom/point2d.h"

#include <complex>

export module Goom.FilterFx.FilterEffects.AdjustmentEffects.ExpReciprocal;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.FilterFx.ZoomAdjustmentEffect;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRandBase;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class ExpReciprocal : public IZoomAdjustmentEffect
{
public:
  explicit ExpReciprocal(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Vec2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> UTILS::NameValuePairs override;

  struct Params
  {
    Viewport viewport;
    Amplitude amplitude;
    FILTER_UTILS::LerpToOneTs lerpToOneTs;
    bool noInverseSquare;
    std::complex<float> magnifyAndRotate;
    bool useNormalizedAmplitude;
    float reciprocalExponent;
    bool useModulatorContours;
    float modulatorPeriod;
  };
  [[nodiscard]] auto GetParams() const noexcept -> const Params&;

protected:
  auto SetParams(const Params& params) noexcept -> void;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  FILTER_UTILS::RandomViewport m_randomViewport;
  Params m_params;
  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt;
};

inline auto ExpReciprocal::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline void ExpReciprocal::SetParams(const Params& params) noexcept
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
