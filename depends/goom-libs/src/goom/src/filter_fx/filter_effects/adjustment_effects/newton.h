#pragma once

#include "complex_utils.h"
#include "filter_fx/common_types.h"
#include "filter_fx/filter_utils/utils.h"
#include "filter_fx/normalized_coords.h"
#include "filter_fx/zoom_adjustment_effect.h"
#include "goom/point2d.h"
#include "utils/math/goom_rand_base.h"

#include <complex>
#include <cstdint>
#include <functional>

import Goom.Utils;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
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

inline auto Newton::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline void Newton::SetParams(const Params& params) noexcept
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
