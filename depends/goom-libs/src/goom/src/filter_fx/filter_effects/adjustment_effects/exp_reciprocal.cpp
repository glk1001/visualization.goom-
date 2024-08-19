module;

//#undef NO_LOGGING
//#include "goom/goom_logger.h"

#include <cmath>
#include <complex>
#include <cstdint>
#include <numeric>

module Goom.FilterFx.FilterEffects.AdjustmentEffects.ExpReciprocal;

import Goom.FilterFx.FilterEffects.AdjustmentEffects.ComplexUtils;
import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Lib.Point2d;
//import Goom.Utils.DebuggingLogger;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using FILTER_UTILS::GetVelocityByZoomLerpedToOne;
using FILTER_UTILS::RandomViewport;
using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;
using UTILS::MATH::SMALL_FLOAT;
using UTILS::MATH::TWO_PI;

static constexpr auto MAX_DOUBLE = std::numeric_limits<double>::max();

static constexpr auto AMPLITUDE_RANGE           = NumberRange{0.01F, 0.2F};
static constexpr auto LERP_TO_ONE_T_RANGE       = NumberRange{0.1F, 1.0F};
static constexpr auto ROTATE_RANGE              = NumberRange{0.0F, TWO_PI};
static constexpr auto MAGNIFY_RANGE             = NumberRange{0.95F, 1.05F};
static constexpr auto RECIPROCAL_EXPONENT_RANGE = NumberRange{3U, 6U};
static constexpr auto MODULATOR_PERIOD_RANGE    = NumberRange{1.0F, 100.0F};
static constexpr auto MAX_ABS_SQ_FZ_RANGE       = NumberRange{1.0e-300 * MAX_DOUBLE, MAX_DOUBLE};

static constexpr auto MAX_VIEWPORT_COORD = 6.0F;
static constexpr auto MIN_VIEWPORT_COORD = -MAX_VIEWPORT_COORD;
static constexpr auto MAX_SIDE_LEN       = MAX_VIEWPORT_COORD - MIN_VIEWPORT_COORD;
static_assert(MIN_VIEWPORT_COORD < 1.0F);
// clang-format off
static constexpr auto VIEWPORT_BOUNDS = RandomViewport::Bounds{
    .minSideLength       = 0.1F,
    .probUseCentredSides = 0.5F,
    .rect                = {.minMaxXMin = {.minValue = MIN_VIEWPORT_COORD, .maxValue = +1.0F},
                            .minMaxYMin = {.minValue = MIN_VIEWPORT_COORD, .maxValue = +1.0F},
                            .minMaxXMax = {.minValue = MIN_VIEWPORT_COORD + 0.1F,
                                           .maxValue = MAX_VIEWPORT_COORD},
                            .minMaxYMax = {.minValue = MIN_VIEWPORT_COORD + 0.1F,
                                           .maxValue = MAX_VIEWPORT_COORD}},
    .sides               = {.minMaxWidth  = {.minValue = 0.1F, .maxValue = MAX_SIDE_LEN},
                            .minMaxHeight = {.minValue = 0.1F, .maxValue = MAX_SIDE_LEN}}
};
// clang-format on

static constexpr auto PROB_AMPLITUDES_EQUAL         = 0.95F;
static constexpr auto PROB_LERP_TO_ONE_T_S_EQUAL    = 0.95F;
static constexpr auto PROB_NO_INVERSE_SQUARE        = 0.50F;
static constexpr auto PROB_USE_NORMALIZED_AMPLITUDE = 0.50F;
static constexpr auto PROB_USE_MODULATOR_CONTOURS   = 0.10F;
static constexpr auto PROB_USE_MAX_ABS_SQ_FZ        = 0.10F;
static constexpr auto PROB_USE_RANDOM_VIEWPORT      = 0.80F;

ExpReciprocal::ExpReciprocal(const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand}, m_randomViewport{goomRand, VIEWPORT_BOUNDS}, m_params{GetRandomParams()}
{
}

auto ExpReciprocal::GetRandomParams() const noexcept -> Params
{
  const auto useRandomViewport = m_goomRand->ProbabilityOf<PROB_USE_RANDOM_VIEWPORT>();
  const auto viewport = useRandomViewport ? m_randomViewport.GetRandomViewport() : Viewport{};

  const auto xAmplitude = m_goomRand->GetRandInRange<AMPLITUDE_RANGE>();
  const auto yAmplitude = m_goomRand->ProbabilityOf<PROB_AMPLITUDES_EQUAL>()
                              ? xAmplitude
                              : m_goomRand->GetRandInRange<AMPLITUDE_RANGE>();

  const auto xLerpToOneT = m_goomRand->GetRandInRange<LERP_TO_ONE_T_RANGE>();
  const auto yLerpToOneT = m_goomRand->ProbabilityOf<PROB_LERP_TO_ONE_T_S_EQUAL>()
                               ? xLerpToOneT
                               : m_goomRand->GetRandInRange<LERP_TO_ONE_T_RANGE>();

  const auto noInverseSquare        = m_goomRand->ProbabilityOf<PROB_NO_INVERSE_SQUARE>();
  const auto useNormalizedAmplitude = m_goomRand->ProbabilityOf<PROB_USE_NORMALIZED_AMPLITUDE>();

  const auto rotate  = std::polar(1.0F, m_goomRand->GetRandInRange<ROTATE_RANGE>());
  const auto magnify = m_goomRand->GetRandInRange<MAGNIFY_RANGE>();

  const auto reciprocalExponent =
      static_cast<float>(m_goomRand->GetRandInRange<RECIPROCAL_EXPONENT_RANGE>());

  const auto useModulatorContours = m_goomRand->ProbabilityOf<PROB_USE_MODULATOR_CONTOURS>();
  const auto modulatorPeriod =
      not useModulatorContours ? 0.0F : m_goomRand->GetRandInRange<MODULATOR_PERIOD_RANGE>();

  const auto useMaxAbsSqFz = m_goomRand->ProbabilityOf<PROB_USE_MAX_ABS_SQ_FZ>();
  const auto maxAbsSqFz =
      not useMaxAbsSqFz ? MAX_DOUBLE : m_goomRand->GetRandInRange<MAX_ABS_SQ_FZ_RANGE>();

  return {
      .viewport               = viewport,
      .amplitude              = {           xAmplitude,            yAmplitude},
      .lerpToOneTs            = {.xLerpT = xLerpToOneT, .yLerpT = yLerpToOneT},
      .noInverseSquare        = noInverseSquare,
      .magnifyAndRotate       = magnify * rotate,
      .useNormalizedAmplitude = useNormalizedAmplitude,
      .reciprocalExponent     = reciprocalExponent,
      .useModulatorContours   = useModulatorContours,
      .modulatorPeriod        = modulatorPeriod,
      .maxAbsSqFz             = maxAbsSqFz,
  };
}

auto ExpReciprocal::GetZoomAdjustment(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto velocity = GetVelocity(coords);

  //return velocity;

  return GetVelocityByZoomLerpedToOne(coords, m_params.lerpToOneTs, velocity);

  //const auto absCoord = std::sqrt(SqDistanceFromZero(coords));
  //if (absCoord < SMALL_FLOAT)
  //{
  //  return {0.0F, 0.0F};
  //}
  //return {coords.GetX()/absCoord * velocity.x, coords.GetY()/absCoord * velocity.y};
}

auto ExpReciprocal::GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto viewportCoords = m_params.viewport.GetViewportCoords(coords);
  const auto sqDistFromZero = SqDistanceFromZero(viewportCoords);

  if (sqDistFromZero < SMALL_FLOAT)
  {
    return {.x = 0.0F, .y = 0.0F};
  }

  const auto zOffset = std::complex<FltCalcType>{};
  const auto z       = static_cast<std::complex<FltCalcType>>(m_params.magnifyAndRotate) *
                 (std::complex<FltCalcType>{static_cast<FltCalcType>(viewportCoords.GetX()),
                                            static_cast<FltCalcType>(viewportCoords.GetY())} +
                  zOffset);

  const auto fz      = std::exp(ONE / std::pow(z, m_params.reciprocalExponent));
  const auto absSqFz = std::norm(fz);

  if (absSqFz > m_params.maxAbsSqFz) // Covers the case: absSqFz = infinity
  {
    return {.x = 0.0F, .y = 0.0F};
  }
  if (absSqFz < SMALL_FLT)
  {
    return {.x = 0.0F, .y = 0.0F};
  }
  if (not m_params.useNormalizedAmplitude)
  {
    return {.x = m_params.amplitude.x * static_cast<float>(fz.real()),
            .y = m_params.amplitude.y * static_cast<float>(fz.imag())};
  }

  const auto normalizedAmplitude =
      GetNormalizedAmplitude(m_params.amplitude, m_params.noInverseSquare, fz, sqDistFromZero);

  if (not m_params.useModulatorContours)
  {
    return {.x = static_cast<float>(normalizedAmplitude.real()),
            .y = static_cast<float>(normalizedAmplitude.imag())};
  }

  const auto modulatedValue =
      GetModulatedValue(absSqFz, normalizedAmplitude, m_params.modulatorPeriod);

  return {.x = static_cast<float>(modulatedValue.real()),
          .y = static_cast<float>(modulatedValue.imag())};
}

auto ExpReciprocal::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  const auto fullParamGroup = GetFullParamGroup({PARAM_GROUP, "exp reciprocal"});
  return {
      GetPair(fullParamGroup, "exp reciprocal", m_params.reciprocalExponent),
      GetPair(fullParamGroup,
              "amplitude",
              Point2dFlt{.x = m_params.amplitude.x, .y = m_params.amplitude.y}),
      GetPair(fullParamGroup,
              "lerpToOneTs",
              Point2dFlt{.x = m_params.lerpToOneTs.xLerpT, .y = m_params.lerpToOneTs.yLerpT}),
      GetPair(fullParamGroup, "noInverseSquare", m_params.noInverseSquare),
      GetPair(fullParamGroup, "useNormalizedAmp", m_params.useNormalizedAmplitude),
      GetPair(
          fullParamGroup,
          "magnify/rotate",
          Point2dFlt{.x = m_params.magnifyAndRotate.real(), .y = m_params.magnifyAndRotate.imag()}),
      GetPair(fullParamGroup, "modulatorPeriod", m_params.modulatorPeriod),
      GetPair(fullParamGroup, "maxAbsSqFz", m_params.maxAbsSqFz),
      GetPair(PARAM_GROUP,
              "viewport0",
              m_params.viewport
                  .GetViewportCoords({NormalizedCoords::MIN_COORD, NormalizedCoords::MIN_COORD})
                  .GetFltCoords()),
      GetPair(PARAM_GROUP,
              "viewport1",
              m_params.viewport
                  .GetViewportCoords({NormalizedCoords::MAX_COORD, NormalizedCoords::MAX_COORD})
                  .GetFltCoords()),
  };
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
