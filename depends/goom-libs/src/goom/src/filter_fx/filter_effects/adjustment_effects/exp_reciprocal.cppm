module;

#include <complex>

export module Goom.FilterFx.FilterEffects.AdjustmentEffects.ExpReciprocal;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.FilterFx.ZoomAdjustmentEffect;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.Point2d;

using GOOM::UTILS::NameValuePairs;
using GOOM::UTILS::MATH::GoomRand;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class ExpReciprocal : public IZoomAdjustmentEffect
{
public:
  explicit ExpReciprocal(const GoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Vec2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> NameValuePairs override;

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
  const GoomRand* m_goomRand;
  FILTER_UTILS::RandomViewport m_randomViewport;
  Params m_params;
  [[nodiscard]] auto GetRandomParams() const noexcept -> Params;
  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt;
};

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

inline auto ExpReciprocal::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline void ExpReciprocal::SetParams(const Params& params) noexcept
{
  m_params = params;
}

inline auto ExpReciprocal::SetRandomParams() noexcept -> void
{
  m_params = GetRandomParams();
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
