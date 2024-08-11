module;

#include <cmath>
#include <complex>
#include <cstdint>
#include <vector>

module Goom.FilterFx.FilterEffects.AdjustmentEffects.ComplexRational;

import Goom.FilterFx.FilterEffects.AdjustmentEffects.ComplexUtils;
import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.TValues;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Lib.Point2d;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using FILTER_UTILS::GetVelocityByZoomLerpedToOne;
using FILTER_UTILS::LerpToOneTs;
using FILTER_UTILS::RandomViewport;
using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;
using UTILS::MATH::SMALL_FLOAT;
using UTILS::MATH::TValue;
using UTILS::MATH::TWO_PI;

static constexpr auto DEFAULT_AMPLITUDE = 0.1F;
static constexpr auto AMPLITUDE_RANGE   = NumberRange{0.025F, 1.00F};

static constexpr auto DEFAULT_LERP_TO_ONE_T_S = LerpToOneTs{.xLerpT = 0.5F, .yLerpT = 0.5F};
static constexpr auto LERP_TO_ONE_T_RANGE     = NumberRange{0.0F, 1.0F};

static constexpr auto DEFAULT_MODULATOR_PERIOD = 2.0F;
static constexpr auto MODULATOR_PERIOD_RANGE   = NumberRange{1.0F, 100.0F};

static constexpr auto VIEWPORT_BOUNDS = RandomViewport::Bounds{
    .minSideLength       = 0.1F,
    .probUseCentredSides = 0.5F,
    .rect                = {.minMaxXMin = {.minValue = -10.0F, .maxValue = +1.0F},
                            .minMaxYMin = {.minValue = -10.0F, .maxValue = +1.0F},
                            .minMaxXMax = {.minValue = -10.0F + 0.1F, .maxValue = +10.0F},
                            .minMaxYMax = {.minValue = -10.0F + 0.1F, .maxValue = +10.0F}},
    .sides               = {.minMaxWidth  = {.minValue = 0.1F, .maxValue = 20.0F},
                            .minMaxHeight = {.minValue = 0.1F, .maxValue = 20.0F}}
};

static constexpr auto PROB_AMPLITUDES_EQUAL            = 0.95F;
static constexpr auto PROB_LERP_TO_ONE_T_S_EQUAL       = 0.95F;
static constexpr auto PROB_NO_INVERSE_SQUARE           = 0.50F;
static constexpr auto PROB_USE_NORMALIZED_AMPLITUDE    = 0.50F;
static constexpr auto PROB_USE_MODULATOR_CONTOURS      = 0.10F;
static constexpr auto PROB_USE_SIMPLE_ZEROES_AND_POLES = 0.50F;
static constexpr auto PROB_USE_INNER_ZEROES            = 0.25F;

ComplexRational::ComplexRational(const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_randomViewport{goomRand, VIEWPORT_BOUNDS},
    m_params{
        .viewport=Viewport{},
        .amplitude={DEFAULT_AMPLITUDE, DEFAULT_AMPLITUDE},
        .lerpToOneTs=DEFAULT_LERP_TO_ONE_T_S,
        .noInverseSquare=true,
        .useNormalizedAmplitude=false,
        .useModulatorContours=false,
        .modulatorPeriod=DEFAULT_MODULATOR_PERIOD
    }
{
}

auto ComplexRational::GetZoomAdjustment(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
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

auto ComplexRational::GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto viewportCoords = m_params.viewport.GetViewportCoords(coords);
  const auto sqDistFromZero = SqDistanceFromZero(viewportCoords);

  if (sqDistFromZero < SMALL_FLOAT)
  {
    return {.x = 0.0F, .y = 0.0F};
  }

  const auto z       = std::complex<FltCalcType>{static_cast<FltCalcType>(viewportCoords.GetX()),
                                                 static_cast<FltCalcType>(viewportCoords.GetY())};
  const auto fz      = GetPolyValue(z);
  const auto absSqFz = std::norm(fz);

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

  return {.x = GetBaseZoomAdjustment().x + static_cast<float>(modulatedValue.real()),
          .y = GetBaseZoomAdjustment().y + static_cast<float>(modulatedValue.imag())};
}

auto ComplexRational::GetPolyValue(const std::complex<FltCalcType>& z) const noexcept
    -> std::complex<FltCalcType>
{
  const auto numerator = GetProduct(z, m_params.zeroesAndPoles.zeroes);
  if (std::norm(numerator) < SMALL_FLT)
  {
    return {static_cast<FltCalcType>(0.0F), static_cast<FltCalcType>(0.0F)};
  }
  const auto denominator = GetProduct(z, m_params.zeroesAndPoles.poles);

  return numerator / denominator;
}

auto ComplexRational::GetProduct(const std::complex<FltCalcType>& z,
                                 const std::vector<std::complex<FltCalcType>>& coeffs) noexcept
    -> std::complex<FltCalcType>
{
  std::complex<float> product{1.0F};

  for (const auto& coeff : coeffs)
  {
    product *= z - coeff;
  }

  return product;
}

auto ComplexRational::SetRandomParams() noexcept -> void
{
  const auto viewport = m_randomViewport.GetRandomViewport();

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

  const auto useModulatorContours = m_goomRand->ProbabilityOf<PROB_USE_MODULATOR_CONTOURS>();
  const auto modulatorPeriod =
      not useModulatorContours ? 0.0F : m_goomRand->GetRandInRange<MODULATOR_PERIOD_RANGE>();

  const auto zeroesAndPoles = GetNextZeroesAndPoles();
  SetParams({
      .viewport               = viewport,
      .amplitude              = {           xAmplitude,            yAmplitude},
      .lerpToOneTs            = {.xLerpT = xLerpToOneT, .yLerpT = yLerpToOneT},
      .noInverseSquare        = noInverseSquare,
      .useNormalizedAmplitude = useNormalizedAmplitude,
      .useModulatorContours   = useModulatorContours,
      .modulatorPeriod        = modulatorPeriod,
      .zeroesAndPoles         = zeroesAndPoles,
  });
}

auto ComplexRational::GetNextZeroesAndPoles() const noexcept -> Params::ZeroesAndPoles
{
  if (m_goomRand->ProbabilityOf<PROB_USE_SIMPLE_ZEROES_AND_POLES>())
  {
    return GetSimpleZeroesAndPoles();
  }

  static constexpr auto ZEROES_INNER_RADIUS_RANGE = NumberRange{0.1F, 0.3F};
  static constexpr auto ZEROES_OUTER_RADIUS_RANGE = NumberRange{0.1F, 0.3F};
  static constexpr auto ZEROES_RADIUS_RANGE       = NumberRange{0.5F, 1.0F};

  static constexpr auto POLES_RADIUS_RANGE = NumberRange{1.5F, 2.0F};

  static constexpr auto INNER_ZEROES_RANGE = NumberRange{3U, 5U};
  static constexpr auto OUTER_ZEROES_RANGE = NumberRange{5U, 6U};
  static constexpr auto ZEROES_RANGE       = NumberRange{3U, 10U};

  static constexpr auto POLES_RANGE = NumberRange{4U, 8U};

  const auto numPoles    = m_goomRand->GetRandInRange<POLES_RANGE>();
  const auto polesRadius = m_goomRand->GetRandInRange<POLES_RADIUS_RANGE>();

  if (not m_goomRand->ProbabilityOf<PROB_USE_INNER_ZEROES>())
  {
    const auto numZeroes    = m_goomRand->GetRandInRange<ZEROES_RANGE>();
    const auto zeroesRadius = m_goomRand->GetRandInRange<ZEROES_RADIUS_RANGE>();
    return {.zeroes = GetPointSpread(numZeroes, zeroesRadius),
            .poles  = GetPointSpread(numPoles, polesRadius)};
  }

  const auto numInnerZeroes    = m_goomRand->GetRandInRange<INNER_ZEROES_RANGE>();
  const auto numOuterZeroes    = m_goomRand->GetRandInRange<OUTER_ZEROES_RANGE>();
  const auto innerZeroesRadius = m_goomRand->GetRandInRange<ZEROES_INNER_RADIUS_RANGE>();
  const auto outerZeroesRadius = m_goomRand->GetRandInRange<ZEROES_OUTER_RADIUS_RANGE>();

  const auto innerZeroes = GetPointSpread(numInnerZeroes, innerZeroesRadius);
  const auto outerZeroes = GetPointSpread(numOuterZeroes, outerZeroesRadius);

  auto zeroes = innerZeroes;
  zeroes.insert(zeroes.end(), outerZeroes.begin(), outerZeroes.end());

  return {.zeroes = zeroes, .poles = GetPointSpread(numPoles, polesRadius)};
}

auto ComplexRational::GetSimpleZeroesAndPoles() noexcept -> Params::ZeroesAndPoles
{
  static constexpr auto ZERO0_PT = static_cast<FltCalcType>(0.1F);
  static constexpr auto ZERO1_PT = static_cast<FltCalcType>(1.0F);
  const auto zeroes              = std::vector<std::complex<FltCalcType>>{
      //{ZERO, ZERO},
      {+ZERO0_PT, +ZERO0_PT},
      {-ZERO0_PT, +ZERO0_PT},
      {-ZERO0_PT, -ZERO0_PT},
      {+ZERO0_PT, -ZERO0_PT},
      {+ZERO1_PT,      ZERO},
      {+ZERO1_PT, +ZERO1_PT},
      {     ZERO, +ZERO1_PT},
      {-ZERO1_PT, +ZERO1_PT},
      {-ZERO1_PT,      ZERO},
      {-ZERO1_PT, -ZERO1_PT},
      {     ZERO, -ZERO1_PT},
      {+ZERO1_PT, -ZERO1_PT},
  };

  static constexpr auto POLE_PT = static_cast<FltCalcType>(1.5F);
  const auto poles              = std::vector<std::complex<FltCalcType>>{
      {    ZERO,     ZERO},
      {+POLE_PT,     ZERO},
      {+POLE_PT, +POLE_PT},
      {    ZERO, +POLE_PT},
      {-POLE_PT, +POLE_PT},
      {-POLE_PT,     ZERO},
      {-POLE_PT, -POLE_PT},
      {    ZERO, -POLE_PT},
      {+POLE_PT, -POLE_PT},
  };

  return {.zeroes = zeroes, .poles = poles};
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto ComplexRational::GetPointSpread(const uint32_t numPoints, const float radius) noexcept
    -> std::vector<std::complex<FltCalcType>>
{
  auto step = TValue{
      TValue::NumStepsProperties{.stepType = TValue::StepType::SINGLE_CYCLE,
                                 .numSteps = numPoints - 1}
  };

  auto points = std::vector<std::complex<FltCalcType>>(numPoints);

  for (auto& point : points)
  {
    const auto angle = step() * TWO_PI;

    point = {static_cast<FltCalcType>(radius * std::cos(angle)),
             static_cast<FltCalcType>(radius * std::sin(angle))};

    step.Increment();
  }

  return points;
}

auto ComplexRational::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  const auto fullParamGroup = GetFullParamGroup({PARAM_GROUP, "complex rat"});
  return {
      GetPair(fullParamGroup,
              "amplitude",
              Point2dFlt{.x = m_params.amplitude.x, .y = m_params.amplitude.y}),
      GetPair(fullParamGroup,
              "lerpToOneTs",
              Point2dFlt{.x = m_params.lerpToOneTs.xLerpT, .y = m_params.lerpToOneTs.yLerpT}),
      GetPair(fullParamGroup, "noInverseSquare", m_params.noInverseSquare),
      GetPair(fullParamGroup, "useNormalizedAmp", m_params.useNormalizedAmplitude),
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
