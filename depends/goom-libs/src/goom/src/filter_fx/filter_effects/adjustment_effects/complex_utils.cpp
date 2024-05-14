#include "complex_utils.h"

#include "filter_fx/common_types.h"

#include <cmath>
#include <complex>
#include <limits>

import Goom.Utils.Math.Misc;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::MATH::GetTriangle;

auto GetNormalizedAmplitude(const Amplitude& amplitude,
                            const bool noInverseSquare,
                            const std::complex<FltCalcType>& fz,
                            const float sqDistFromZero) noexcept -> std::complex<FltCalcType>
{
  const auto normalizedValue = std::polar(ONE, std::arg(fz));

  const auto realNormalizedPart = static_cast<FltCalcType>(amplitude.x) * normalizedValue.real();
  const auto imagNormalizedPart = static_cast<FltCalcType>(amplitude.y) * normalizedValue.imag();

  if (noInverseSquare)
  {
    return {realNormalizedPart, imagNormalizedPart};
  }

  static constexpr auto MIN_LOG_ARG = static_cast<FltCalcType>(1.5F);
  const auto inverseLogSqDistFromZero =
      ONE / std::log(MIN_LOG_ARG + static_cast<FltCalcType>(sqDistFromZero));

  return {inverseLogSqDistFromZero * realNormalizedPart,
          inverseLogSqDistFromZero * imagNormalizedPart};
}

auto GetModulatedValue(const FltCalcType absSqFz,
                       const std::complex<FltCalcType>& value,
                       const float modulatorPeriod) noexcept -> std::complex<FltCalcType>
{
  static constexpr auto MAX_ABS_SQ_FZ = static_cast<FltCalcType>(std::numeric_limits<float>::max());
  static constexpr auto MAX_LOG_VAL   = static_cast<FltCalcType>(1000.0F);
  const auto logAbsSqFz               = absSqFz > MAX_ABS_SQ_FZ ? MAX_LOG_VAL : std::log(absSqFz);

  const auto logAbsFzModulator =
      static_cast<FltCalcType>(GetTriangle(static_cast<float>(logAbsSqFz), modulatorPeriod));

  return logAbsFzModulator * value;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
