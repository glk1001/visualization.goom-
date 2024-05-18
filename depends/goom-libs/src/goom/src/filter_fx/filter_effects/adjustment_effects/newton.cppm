module;

#include "goom/goom_config.h"

#include <complex>
#include <cstdint>
#include <functional>

export module Goom.FilterFx.FilterEffects.AdjustmentEffects.Newton;

import Goom.FilterFx.FilterEffects.AdjustmentEffects.ComplexUtils;
import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.FilterFx.ZoomAdjustmentEffect;
import Goom.Utils.EnumUtils;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRandBase;
import Goom.Lib.Point2d;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class Newton : public IZoomAdjustmentEffect
{
public:
  explicit Newton(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Vec2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> UTILS::NameValuePairs override;

  struct Params
  {
    Viewport viewport;
    uint32_t exponent;
    std::complex<FltCalcType> a;
    std::complex<FltCalcType> c;
    Amplitude amplitude;
    FILTER_UTILS::LerpToOneTs lerpToOneTs;
    bool useSqDistDenominator;
    float denominator;
    bool useZSinInput;
    Amplitude zSinAmplitude;
  };
  [[nodiscard]] auto GetParams() const noexcept -> const Params&;

protected:
  auto SetParams(const Params& params) noexcept -> void;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  FILTER_UTILS::RandomViewport m_randomViewport;
  Params m_params;
  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt;
  [[nodiscard]] auto GetZ(const NormalizedCoords& coords) const noexcept
      -> std::complex<FltCalcType>;

  struct FuncValueAndDerivative
  {
    std::complex<FltCalcType> fz;
    std::complex<FltCalcType> dFdz;
  };
  using GetFuncValueAndDerivativeFunc =
      std::function<FuncValueAndDerivative(const std::complex<FltCalcType>& z)>;
  GetFuncValueAndDerivativeFunc m_getFuncValueAndDerivative = GetNextFuncValueAndDerivativeFunc();
  [[nodiscard]] auto GetNextFuncValueAndDerivativeFunc() const noexcept
      -> GetFuncValueAndDerivativeFunc;
  [[nodiscard]] auto GetPolyFuncValueAndDerivative(
      const std::complex<FltCalcType>& z) const noexcept -> FuncValueAndDerivative;
  [[nodiscard]] auto GetPolySinFuncValueAndDerivative(
      const std::complex<FltCalcType>& z) const noexcept -> FuncValueAndDerivative;
};

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

inline auto Newton::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline void Newton::SetParams(const Params& params) noexcept
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
