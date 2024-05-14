//#undef NO_LOGGING

#include "exp_reciprocal.h"

#include "complex_utils.h"
#include "filter_fx/filter_utils/utils.h"
#include "filter_fx/normalized_coords.h"
#include "goom/point2d.h"

#include <cmath>
#include <complex>
#include <cstdint>

import Goom.Utils.Math.GoomRandBase;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using FILTER_UTILS::GetVelocityByZoomLerpedToOne;
using FILTER_UTILS::LerpToOneTs;
using FILTER_UTILS::RandomViewport;
using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::SMALL_FLOAT;
using UTILS::MATH::TWO_PI;

static constexpr auto DEFAULT_AMPLITUDE = 0.1F;
static constexpr auto AMPLITUDE_RANGE   = IGoomRand::NumberRange<float>{0.01F, 0.11F};

static constexpr auto DEFAULT_LERP_TO_ONE_T_S = LerpToOneTs{0.5F, 0.5F};
static constexpr auto LERP_TO_ONE_T_RANGE     = IGoomRand::NumberRange<float>{0.0F, 1.0F};

static constexpr auto DEFAULT_MAGNIFY_AND_ROTATE = std::complex<float>{0.0F, 0.0F};
static constexpr auto ROTATE_RANGE               = IGoomRand::NumberRange<float>{0.0F, TWO_PI};
static constexpr auto MAGNIFY_RANGE              = IGoomRand::NumberRange<float>{0.95F, 1.05F};

static constexpr auto DEFAULT_RECIPROCAL_EXPONENT = 3.0F;
static constexpr auto RECIPROCAL_EXPONENT_RANGE   = IGoomRand::NumberRange<uint32_t>{3, 6};

static constexpr auto DEFAULT_MODULATOR_PERIOD = 2.0F;
static constexpr auto MODULATOR_PERIOD_RANGE   = IGoomRand::NumberRange<float>{1.0F, 100.0F};

static constexpr auto VIEWPORT_BOUNDS = RandomViewport::Bounds{
    .minSideLength       = 0.1F,
    .probUseCentredSides = 0.5F,
    .rect                = {.minMaxXMin = {-10.0F, +1.0F},
                            .minMaxYMin = {-10.0F, +1.0F},
                            .minMaxXMax = {-10.0F + 0.1F, +10.0F},
                            .minMaxYMax = {-10.0F + 0.1F, +10.0F}},
    .sides               = {.minMaxWidth = {0.1F, 20.0F}, .minMaxHeight = {0.1F, 20.0F}}
};

static constexpr auto PROB_AMPLITUDES_EQUAL         = 0.95F;
static constexpr auto PROB_LERP_TO_ONE_T_S_EQUAL    = 0.95F;
static constexpr auto PROB_NO_INVERSE_SQUARE        = 0.50F;
static constexpr auto PROB_USE_NORMALIZED_AMPLITUDE = 0.50F;
static constexpr auto PROB_USE_MODULATOR_CONTOURS   = 0.10F;

ExpReciprocal::ExpReciprocal(const IGoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_randomViewport{goomRand, VIEWPORT_BOUNDS},
    m_params{
        Viewport{},
        {DEFAULT_AMPLITUDE, DEFAULT_AMPLITUDE},
        DEFAULT_LERP_TO_ONE_T_S,
        true,
        DEFAULT_MAGNIFY_AND_ROTATE,
        false,
        DEFAULT_RECIPROCAL_EXPONENT,
        false,
        DEFAULT_MODULATOR_PERIOD,
    }
{
}

auto ExpReciprocal::SetRandomParams() noexcept -> void
{
  const auto viewport = m_randomViewport.GetRandomViewport();

  const auto xAmplitude = m_goomRand->GetRandInRange(AMPLITUDE_RANGE);
  const auto yAmplitude = m_goomRand->ProbabilityOf(PROB_AMPLITUDES_EQUAL)
                              ? xAmplitude
                              : m_goomRand->GetRandInRange(AMPLITUDE_RANGE);

  const auto xLerpToOneT = m_goomRand->GetRandInRange(LERP_TO_ONE_T_RANGE);
  const auto yLerpToOneT = m_goomRand->ProbabilityOf(PROB_LERP_TO_ONE_T_S_EQUAL)
                               ? xLerpToOneT
                               : m_goomRand->GetRandInRange(LERP_TO_ONE_T_RANGE);

  const auto noInverseSquare        = m_goomRand->ProbabilityOf(PROB_NO_INVERSE_SQUARE);
  const auto useNormalizedAmplitude = m_goomRand->ProbabilityOf(PROB_USE_NORMALIZED_AMPLITUDE);

  const auto rotate  = std::polar(1.0F, m_goomRand->GetRandInRange(ROTATE_RANGE));
  const auto magnify = m_goomRand->GetRandInRange(MAGNIFY_RANGE);

  const auto reciprocalExponent =
      static_cast<float>(m_goomRand->GetRandInRange(RECIPROCAL_EXPONENT_RANGE));

  const auto useModulatorContours = m_goomRand->ProbabilityOf(PROB_USE_MODULATOR_CONTOURS);
  const auto modulatorPeriod =
      not useModulatorContours ? 0.0F : m_goomRand->GetRandInRange(MODULATOR_PERIOD_RANGE);

  SetParams({
      viewport,
      { xAmplitude,  yAmplitude},
      {xLerpToOneT, yLerpToOneT},
      noInverseSquare,
      magnify * rotate,
      useNormalizedAmplitude,
      reciprocalExponent,
      useModulatorContours,
      modulatorPeriod,
  });
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
    return {0.0F, 0.0F};
  }

  const auto zOffset = std::complex<FltCalcType>{};
  const auto z       = static_cast<std::complex<FltCalcType>>(m_params.magnifyAndRotate) *
                 (std::complex<FltCalcType>{static_cast<FltCalcType>(viewportCoords.GetX()),
                                            static_cast<FltCalcType>(viewportCoords.GetY())} +
                  zOffset);

  const auto fz      = std::exp(ONE / std::pow(z, m_params.reciprocalExponent));
  const auto absSqFz = std::norm(fz);

  if (absSqFz < SMALL_FLT)
  {
    return {0.0F, 0.0F};
  }
  if (not m_params.useNormalizedAmplitude)
  {
    return {m_params.amplitude.x * static_cast<float>(fz.real()),
            m_params.amplitude.y * static_cast<float>(fz.imag())};
  }

  const auto normalizedAmplitude =
      GetNormalizedAmplitude(m_params.amplitude, m_params.noInverseSquare, fz, sqDistFromZero);

  if (not m_params.useModulatorContours)
  {
    return {static_cast<float>(normalizedAmplitude.real()),
            static_cast<float>(normalizedAmplitude.imag())};
  }

  const auto modulatedValue =
      GetModulatedValue(absSqFz, normalizedAmplitude, m_params.modulatorPeriod);

  return {static_cast<float>(modulatedValue.real()), static_cast<float>(modulatedValue.imag())};
}

auto ExpReciprocal::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  const auto fullParamGroup = GetFullParamGroup({PARAM_GROUP, "exp reciprocal"});
  return {
      GetPair(fullParamGroup, "exp reciprocal", m_params.reciprocalExponent),
      GetPair(fullParamGroup, "amplitude", Point2dFlt{m_params.amplitude.x, m_params.amplitude.y}),
      GetPair(fullParamGroup,
              "lerpToOneTs",
              Point2dFlt{m_params.lerpToOneTs.xLerpT, m_params.lerpToOneTs.yLerpT}),
      GetPair(fullParamGroup, "noInverseSquare", m_params.noInverseSquare),
      GetPair(fullParamGroup, "useNormalizedAmp", m_params.useNormalizedAmplitude),
      GetPair(fullParamGroup,
              "magnify/rotate",
              Point2dFlt{m_params.magnifyAndRotate.real(), m_params.magnifyAndRotate.imag()}),
      GetPair(fullParamGroup, "modulatorPeriod", m_params.modulatorPeriod),
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
