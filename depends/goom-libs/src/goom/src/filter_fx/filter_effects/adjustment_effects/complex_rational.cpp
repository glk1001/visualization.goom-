#include "complex_rational.h"

#include "complex_utils.h"
#include "filter_fx/filter_utils/utils.h"
#include "filter_fx/normalized_coords.h"
#include "goom/goom_config.h"
#include "goom/point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/name_value_pairs.h"
#include "utils/t_values.h"

#include <cmath>
#include <complex>
#include <cstdint>
#include <vector>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using FILTER_UTILS::GetVelocityByZoomLerpedToOne;
using FILTER_UTILS::LerpToOneTs;
using FILTER_UTILS::RandomViewport;
using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::TValue;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::SMALL_FLOAT;
using UTILS::MATH::TWO_PI;

static constexpr auto DEFAULT_AMPLITUDE = 0.1F;
static constexpr auto AMPLITUDE_RANGE   = IGoomRand::NumberRange<float>{0.025F, 1.00F};

static constexpr auto DEFAULT_LERP_TO_ONE_T_S = LerpToOneTs{0.5F, 0.5F};
static constexpr auto LERP_TO_ONE_T_RANGE     = IGoomRand::NumberRange<float>{0.0F, 1.0F};

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

static constexpr auto PROB_AMPLITUDES_EQUAL            = 0.95F;
static constexpr auto PROB_LERP_TO_ONE_T_S_EQUAL       = 0.95F;
static constexpr auto PROB_NO_INVERSE_SQUARE           = 0.50F;
static constexpr auto PROB_USE_NORMALIZED_AMPLITUDE    = 0.50F;
static constexpr auto PROB_USE_MODULATOR_CONTOURS      = 0.10F;
static constexpr auto PROB_USE_SIMPLE_ZEROES_AND_POLES = 0.50F;
static constexpr auto PROB_USE_INNER_ZEROES            = 0.25F;

ComplexRational::ComplexRational(const IGoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_randomViewport{goomRand, VIEWPORT_BOUNDS},
    m_params{
        Viewport{},
        {DEFAULT_AMPLITUDE, DEFAULT_AMPLITUDE},
        DEFAULT_LERP_TO_ONE_T_S,
        true,
        false,
        false,
        DEFAULT_MODULATOR_PERIOD
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
    return {0.0F, 0.0F};
  }

  const auto z       = std::complex<FltCalcType>{static_cast<FltCalcType>(viewportCoords.GetX()),
                                                 static_cast<FltCalcType>(viewportCoords.GetY())};
  const auto fz      = GetPolyValue(z);
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

  return {GetBaseZoomAdjustment().x + static_cast<float>(modulatedValue.real()),
          GetBaseZoomAdjustment().y + static_cast<float>(modulatedValue.imag())};
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

  const auto useModulatorContours = m_goomRand->ProbabilityOf(PROB_USE_MODULATOR_CONTOURS);
  const auto modulatorPeriod =
      not useModulatorContours ? 0.0F : m_goomRand->GetRandInRange(MODULATOR_PERIOD_RANGE);

  const auto zeroesAndPoles = GetNextZeroesAndPoles();
  SetParams({
      viewport,
      { xAmplitude,  yAmplitude},
      {xLerpToOneT, yLerpToOneT},
      noInverseSquare,
      useNormalizedAmplitude,
      useModulatorContours,
      modulatorPeriod,
      zeroesAndPoles,
  });
}

auto ComplexRational::GetNextZeroesAndPoles() const noexcept -> Params::ZeroesAndPoles
{
  if (m_goomRand->ProbabilityOf(PROB_USE_SIMPLE_ZEROES_AND_POLES))
  {
    return GetSimpleZeroesAndPoles();
  }

  static constexpr auto MIN_ZEROES_INNER_RADIUS = 0.1F;
  static constexpr auto MAX_ZEROES_INNER_RADIUS = 0.3F;
  static constexpr auto MIN_ZEROES_OUTER_RADIUS = 0.1F;
  static constexpr auto MAX_ZEROES_OUTER_RADIUS = 0.3F;
  static constexpr auto MIN_ZEROES_RADIUS       = 0.5F;
  static constexpr auto MAX_ZEROES_RADIUS       = 1.0F;

  static constexpr auto MIN_POLES_RADIUS = 1.5F;
  static constexpr auto MAX_POLES_RADIUS = 2.0F;

  static constexpr auto MIN_INNER_ZEROES = 3U;
  static constexpr auto MAX_INNER_ZEROES = 5U;
  static constexpr auto MIN_OUTER_ZEROES = 5U;
  static constexpr auto MAX_OUTER_ZEROES = 6U;
  static constexpr auto MIN_ZEROES       = 3U;
  static constexpr auto MAX_ZEROES       = 10U;

  static constexpr auto MIN_POLES = 4U;
  static constexpr auto MAX_POLES = 8U;

  const auto numPoles    = m_goomRand->GetRandInRange(MIN_POLES, MAX_POLES + 1);
  const auto polesRadius = m_goomRand->GetRandInRange(MIN_POLES_RADIUS, MAX_POLES_RADIUS);

  if (not m_goomRand->ProbabilityOf(PROB_USE_INNER_ZEROES))
  {
    const auto numZeroes    = m_goomRand->GetRandInRange(MIN_ZEROES, MAX_ZEROES + 1);
    const auto zeroesRadius = m_goomRand->GetRandInRange(MIN_ZEROES_RADIUS, MAX_ZEROES_RADIUS);
    return {GetPointSpread(numZeroes, zeroesRadius), GetPointSpread(numPoles, polesRadius)};
  }

  const auto numInnerZeroes = m_goomRand->GetRandInRange(MIN_INNER_ZEROES, MAX_INNER_ZEROES + 1);
  const auto numOuterZeroes = m_goomRand->GetRandInRange(MIN_OUTER_ZEROES, MAX_OUTER_ZEROES + 1);
  const auto innerZeroesRadius =
      m_goomRand->GetRandInRange(MIN_ZEROES_INNER_RADIUS, MAX_ZEROES_INNER_RADIUS);
  const auto outerZeroesRadius =
      m_goomRand->GetRandInRange(MIN_ZEROES_OUTER_RADIUS, MAX_ZEROES_OUTER_RADIUS);

  const auto innerZeroes = GetPointSpread(numInnerZeroes, innerZeroesRadius);
  const auto outerZeroes = GetPointSpread(numOuterZeroes, outerZeroesRadius);

  auto zeroes = innerZeroes;
  zeroes.insert(zeroes.end(), outerZeroes.begin(), outerZeroes.end());

  return {zeroes, GetPointSpread(numPoles, polesRadius)};
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

  return {zeroes, poles};
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto ComplexRational::GetPointSpread(const uint32_t numPoints, const float radius) noexcept
    -> std::vector<std::complex<FltCalcType>>
{
  auto step = TValue{
      TValue::NumStepsProperties{TValue::StepType::SINGLE_CYCLE, numPoints - 1}
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
      GetPair(fullParamGroup, "amplitude", Point2dFlt{m_params.amplitude.x, m_params.amplitude.y}),
      GetPair(fullParamGroup,
              "lerpToOneTs",
              Point2dFlt{m_params.lerpToOneTs.xLerpT, m_params.lerpToOneTs.yLerpT}),
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
