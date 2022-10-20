#include "exp_reciprocal.h"

//#undef NO_LOGGING

#include "logging.h"
#include "utils/math/misc.h"
#include "utils/name_value_pairs.h"

#include <cmath>
#include <complex>
#include <limits>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::Logging; // NOLINT(misc-unused-using-decls)
using UTILS::NameValuePairs;
using UTILS::MATH::GetSawTooth;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::TWO_PI;

static constexpr auto DEFAULT_AMPLITUDE = 0.1F;
static constexpr auto AMPLITUDE_RANGE   = IGoomRand::NumberRange<float>{0.01F, 0.11F};

static constexpr auto DEFAULT_ROTATE = std::complex{0.0F, 0.0F};
static constexpr auto ROTATE_RANGE   = IGoomRand::NumberRange<float>{0.0F, TWO_PI};

static constexpr auto DEFAULT_MAGNIFY = 1.0F;
static constexpr auto MAGNIFY_RANGE   = IGoomRand::NumberRange<float>{0.95F, 1.05F};

static constexpr auto DEFAULT_RECIPROCAL_EXPONENT = 3.0F;
static constexpr auto RECIPROCAL_EXPONENT_RANGE   = IGoomRand::NumberRange<uint32_t>{3, 6};

static constexpr auto PROB_USE_MODULUS_CONTOURS = 0.25F;
static constexpr auto PROB_USE_PHASE_CONTOURS   = 0.25F;

static constexpr auto DEFAULT_SAWTOOTH_MODULUS_PERIOD = 0.5F;
static constexpr auto DEFAULT_SAWTOOTH_PHASE_PERIOD   = 0.5F;
static constexpr auto SAWTOOTH_MODULUS_PERIOD_RANGE   = IGoomRand::NumberRange<float>{0.1F, 1.1F};
static constexpr auto SAWTOOTH_PHASE_PERIOD_RANGE     = IGoomRand::NumberRange<float>{0.1F, 1.1F};

ExpReciprocal::ExpReciprocal(const IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand},
    m_params{
        {DEFAULT_AMPLITUDE, DEFAULT_AMPLITUDE},
        DEFAULT_ROTATE,
        DEFAULT_MAGNIFY,
        DEFAULT_RECIPROCAL_EXPONENT,
        false,
        false,
        DEFAULT_SAWTOOTH_MODULUS_PERIOD,
        DEFAULT_SAWTOOTH_PHASE_PERIOD
    }
{
}

auto ExpReciprocal::SetRandomParams() noexcept -> void
{
  const auto xAmplitude = m_goomRand.GetRandInRange(AMPLITUDE_RANGE);
  const auto yAmplitude = m_goomRand.GetRandInRange(AMPLITUDE_RANGE);

  const auto rotate  = std::polar(1.0F, m_goomRand.GetRandInRange(ROTATE_RANGE));
  const auto magnify = m_goomRand.GetRandInRange(MAGNIFY_RANGE);

  const auto reciprocalExponent =
      static_cast<float>(m_goomRand.GetRandInRange(RECIPROCAL_EXPONENT_RANGE));

  const auto useModulusContours = m_goomRand.ProbabilityOf(PROB_USE_MODULUS_CONTOURS);
  const auto usePhaseContours   = m_goomRand.ProbabilityOf(PROB_USE_PHASE_CONTOURS);

  const auto sawtoothModulusPeriod =
      not useModulusContours ? 1.0F : m_goomRand.GetRandInRange(SAWTOOTH_MODULUS_PERIOD_RANGE);
  const auto sawtoothPhasePeriod =
      not usePhaseContours ? 1.0F : m_goomRand.GetRandInRange(SAWTOOTH_PHASE_PERIOD_RANGE);

  SetParams({
      {xAmplitude, yAmplitude},
      rotate,
      magnify,
      reciprocalExponent,
      useModulusContours,
      usePhaseContours,
      sawtoothModulusPeriod,
      sawtoothPhasePeriod
  });
}

auto ExpReciprocal::GetZoomInCoefficients(const NormalizedCoords& coords,
                                          const float sqDistFromZero) const noexcept -> Point2dFlt
{
  if (sqDistFromZero < UTILS::MATH::SMALL_FLOAT)
  {
    return GetBaseZoomInCoeffs();
  }

  using FltCalcType         = double;
  static constexpr auto ONE = static_cast<FltCalcType>(1.0F);

  const auto zOffset = std::complex<FltCalcType>{};
  const auto z       = static_cast<FltCalcType>(m_params.magnify) *
                 static_cast<std::complex<FltCalcType>>(m_params.rotate) *
                 (std::complex{static_cast<FltCalcType>(coords.GetX()),
                               static_cast<FltCalcType>(coords.GetY())} +
                  zOffset);

  const auto fz      = std::exp(ONE / std::pow(z, m_params.reciprocalExponent));
  const auto absSqFz = std::norm(fz);

  if (absSqFz < static_cast<FltCalcType>(UTILS::MATH::SMALL_FLOAT))
  {
    return GetBaseZoomInCoeffs();
  }

  const auto phase                  = std::polar(ONE, std::arg(fz));
  static constexpr auto MIN_LOG_ARG = static_cast<FltCalcType>(1.5F);
  const auto inverseLogSqDistFromZero =
      ONE / std::log(MIN_LOG_ARG + static_cast<FltCalcType>(sqDistFromZero));

  const auto realPart = static_cast<float>(
      inverseLogSqDistFromZero * (static_cast<FltCalcType>(m_params.amplitude.x) * phase.real()));
  const auto imagPart = static_cast<float>(
      inverseLogSqDistFromZero * (static_cast<FltCalcType>(m_params.amplitude.y) * phase.imag()));

  if (not m_params.useModulusContours and not m_params.usePhaseContours)
  {
    return {GetBaseZoomInCoeffs().x + realPart, GetBaseZoomInCoeffs().y + imagPart};
  }

  static constexpr auto MAX_ABS_SQ_FZ = static_cast<FltCalcType>(std::numeric_limits<float>::max());
  static constexpr auto MAX_LOG_VAL   = static_cast<FltCalcType>(1000.0F);
  const auto logAbsSqFz               = absSqFz > MAX_ABS_SQ_FZ ? MAX_LOG_VAL : std::log(absSqFz);

  const auto sawtoothLogAbsFz =
      GetSawTooth(static_cast<float>(logAbsSqFz), m_params.sawtoothModulusPeriod);
  const auto sawtoothPhaseReal =
      GetSawTooth(static_cast<float>(phase.real()), m_params.sawtoothPhasePeriod);
  const auto sawtoothPhaseImag =
      GetSawTooth(static_cast<float>(phase.imag()), m_params.sawtoothPhasePeriod);

  const auto enhancedRealPart = sawtoothLogAbsFz * sawtoothPhaseReal * static_cast<float>(realPart);
  const auto enhancedImagPart = sawtoothLogAbsFz * sawtoothPhaseImag * static_cast<float>(imagPart);

  return {GetBaseZoomInCoeffs().x + enhancedRealPart, GetBaseZoomInCoeffs().y + enhancedImagPart};
}

auto ExpReciprocal::GetZoomInCoefficientsEffectNameValueParams() const noexcept -> NameValuePairs
{
  const auto fullParamGroup = GetFullParamGroup({PARAM_GROUP, "exp reciprocal"});
  return {
      GetPair(fullParamGroup, "amplitude", Point2dFlt{m_params.amplitude.x, m_params.amplitude.y}),
      GetPair(fullParamGroup, "rotate", Point2dFlt{m_params.rotate.real(), m_params.rotate.imag()}),
      GetPair(fullParamGroup, "magnify", m_params.magnify),
      GetPair(fullParamGroup, "recipr exp", m_params.reciprocalExponent),
      GetPair(fullParamGroup, "sawtoothModulusPeriod", m_params.sawtoothModulusPeriod),
      GetPair(fullParamGroup, "sawtoothPhasePeriod", m_params.sawtoothPhasePeriod),
  };
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
