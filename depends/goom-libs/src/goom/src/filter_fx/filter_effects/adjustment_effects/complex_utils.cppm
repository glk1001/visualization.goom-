module;

#include <complex>

export module Goom.FilterFx.FilterEffects.AdjustmentEffects.ComplexUtils;

import Goom.FilterFx.CommonTypes;
import Goom.Utils.Math.Misc;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using FltCalcType               = double;
inline constexpr auto ZERO      = static_cast<FltCalcType>(0.0F);
inline constexpr auto ONE       = static_cast<FltCalcType>(1.0F);
inline constexpr auto HALF      = static_cast<FltCalcType>(0.5F);
inline constexpr auto SMALL_FLT = static_cast<FltCalcType>(UTILS::MATH::SMALL_FLOAT);

[[nodiscard]] auto GetNormalizedAmplitude(const Amplitude& amplitude,
                                          bool noInverseSquare,
                                          const std::complex<FltCalcType>& fz,
                                          float sqDistFromZero) noexcept
    -> std::complex<FltCalcType>;
[[nodiscard]] auto GetModulatedValue(FltCalcType absSqFz,
                                     const std::complex<FltCalcType>& value,
                                     float modulatorPeriod) noexcept -> std::complex<FltCalcType>;

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
