#include "newton.h"

#include "complex_utils.h"
#include "filter_fx/filter_utils/utils.h"
#include "filter_fx/normalized_coords.h"
#include "goom/point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

#include <cmath>
#include <complex>
#include <cstdint>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using FILTER_UTILS::GetVelocityByZoomLerpedToOne;
using FILTER_UTILS::LerpToOneTs;
using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;

static constexpr auto DEFAULT_AMPLITUDE = 0.1F;
static constexpr auto AMPLITUDE_RANGE   = IGoomRand::NumberRange<float>{0.01F, 0.11F};

static constexpr auto DEFAULT_LERP_TO_ONE_T_S = LerpToOneTs{0.5F, 0.5F};
static constexpr auto LERP_TO_ONE_T_RANGE     = IGoomRand::NumberRange<float>{0.0F, 1.0F};

static constexpr auto DEFAULT_USE_Z_SIN       = true;
static constexpr auto DEFAULT_Z_SIN_AMPLITUDE = 0.1F;
static constexpr auto Z_SIN_AMPLITUDE_RANGE   = IGoomRand::NumberRange<float>{0.01F, 20.0F};

static constexpr auto DEFAULT_USE_SQ_DIST_DENOM = true;
static constexpr auto DEFAULT_DENOMINATOR       = 20.0F;
static constexpr auto DENOMINATOR_RANGE         = IGoomRand::NumberRange<float>{20.0F, 40.0F};

static constexpr auto DEFAULT_EXPONENT = 3U;
static constexpr auto EXPONENT_RANGE   = IGoomRand::NumberRange<uint32_t>{3U, 11U};

static constexpr auto DEFAULT_A_REAL = 1.0F;
static constexpr auto DEFAULT_A_IMAG = 0.0F;
static constexpr auto A_REAL_RANGE   = IGoomRand::NumberRange<float>{-1.0F, +1.0F};
static constexpr auto A_IMAG_RANGE   = IGoomRand::NumberRange<float>{-1.0F, +1.0F};

static constexpr auto DEFAULT_C_REAL = 0.0F;
static constexpr auto DEFAULT_C_IMAG = 0.0F;
static constexpr auto C_REAL_RANGE   = IGoomRand::NumberRange<float>{-1.0F, +1.0F};
static constexpr auto C_IMAG_RANGE   = IGoomRand::NumberRange<float>{-1.0F, +1.0F};

static constexpr auto PROB_AMPLITUDES_EQUAL       = 0.95F;
static constexpr auto PROB_LERP_TO_ONE_T_S_EQUAL  = 0.95F;
static constexpr auto PROB_Z_SIN                  = 0.80F;
static constexpr auto PROB_Z_SIN_AMPLITUDES_EQUAL = 0.95F;
static constexpr auto PROB_POLY_SIN_FUNC          = 0.5F;
static constexpr auto PROB_USE_SQ_DIST_DENOM      = 0.1F;

Newton::Newton(const IGoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_randomViewport{goomRand},
    m_params{
        Viewport{},
        DEFAULT_EXPONENT,
        {static_cast<FltCalcType>(DEFAULT_A_REAL),
         static_cast<FltCalcType>(DEFAULT_A_IMAG)},
        {static_cast<FltCalcType>(DEFAULT_C_REAL),
         static_cast<FltCalcType>(DEFAULT_C_IMAG)},
        {DEFAULT_AMPLITUDE, DEFAULT_AMPLITUDE},
        DEFAULT_LERP_TO_ONE_T_S,
        DEFAULT_USE_SQ_DIST_DENOM,
        DEFAULT_DENOMINATOR,
        DEFAULT_USE_Z_SIN,
        {DEFAULT_Z_SIN_AMPLITUDE, DEFAULT_Z_SIN_AMPLITUDE},
    }
{
}

auto Newton::SetRandomParams() noexcept -> void
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

  const auto useZSin        = m_goomRand->ProbabilityOf(PROB_Z_SIN);
  const auto xZSinAmplitude = m_goomRand->GetRandInRange(Z_SIN_AMPLITUDE_RANGE);
  const auto yZSinAmplitude = m_goomRand->ProbabilityOf(PROB_Z_SIN_AMPLITUDES_EQUAL)
                                  ? xZSinAmplitude
                                  : m_goomRand->GetRandInRange(Z_SIN_AMPLITUDE_RANGE);

  const auto exponent = m_goomRand->GetRandInRange(EXPONENT_RANGE);
  const auto a        = std::complex<float>{m_goomRand->GetRandInRange(A_REAL_RANGE),
                                            m_goomRand->GetRandInRange(A_IMAG_RANGE)};
  const auto c        = std::complex<float>{m_goomRand->GetRandInRange(C_REAL_RANGE),
                                            m_goomRand->GetRandInRange(C_IMAG_RANGE)};

  const auto useSqDistDenominator = m_goomRand->ProbabilityOf(PROB_USE_SQ_DIST_DENOM);
  const auto denominator          = m_goomRand->GetRandInRange(DENOMINATOR_RANGE);

  SetParams({
      viewport,
      exponent,
      a,
      c,
      {    xAmplitude,     yAmplitude},
      {   xLerpToOneT,    yLerpToOneT},
      useSqDistDenominator,
      denominator,
      useZSin,
      {xZSinAmplitude, yZSinAmplitude},
  });
}

auto Newton::GetZoomAdjustment(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
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

auto Newton::GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto viewportCoords = m_params.viewport.GetViewportCoords(coords);
  const auto sqDistFromZero = SqDistanceFromZero(viewportCoords);

  const auto z = GetZ(viewportCoords);

  const auto fzAndDfDz = m_getFuncValueAndDerivative(z);

  const auto absSqDfDz = std::norm(fzAndDfDz.dFdz);
  if (absSqDfDz < SMALL_FLT)
  {
    return {0.0F, 0.0F};
  }

  const auto fullDenominator = m_params.useSqDistDenominator
                                   ? static_cast<FltCalcType>(m_params.denominator + sqDistFromZero)
                                   : static_cast<FltCalcType>(m_params.denominator);

  const auto zoomAdj =
      (z - ((m_params.a * fzAndDfDz.fz) / fzAndDfDz.dFdz) + m_params.c) / fullDenominator;

  return {m_params.amplitude.x * static_cast<float>(zoomAdj.real()),
          m_params.amplitude.y * static_cast<float>(zoomAdj.imag())};
}

auto Newton::GetZ(const NormalizedCoords& coords) const noexcept -> std::complex<FltCalcType>
{
  const auto zReal = static_cast<FltCalcType>(coords.GetX());
  const auto zImag = static_cast<FltCalcType>(coords.GetY());

  if (not m_params.useZSinInput)
  {
    return std::complex<FltCalcType>{zReal, zImag};
  }

  return HALF *
         std::complex<FltCalcType>{
             ONE + (static_cast<FltCalcType>(m_params.zSinAmplitude.x) * std::sin(zReal)),
             ONE + (static_cast<FltCalcType>(m_params.zSinAmplitude.y) * std::sin(zImag))} *
         std::complex<FltCalcType>{zReal, zImag};
}

auto Newton::GetNextFuncValueAndDerivativeFunc() const noexcept -> GetFuncValueAndDerivativeFunc
{
  if (m_goomRand->ProbabilityOf(PROB_POLY_SIN_FUNC))
  {
    return [this](const std::complex<FltCalcType>& z)
    { return GetPolySinFuncValueAndDerivative(z); };
  }

  return [this](const std::complex<FltCalcType>& z) { return GetPolyFuncValueAndDerivative(z); };
}

auto Newton::GetPolyFuncValueAndDerivative(const std::complex<FltCalcType>& z) const noexcept
    -> FuncValueAndDerivative
{
  const auto fz = std::pow(z, m_params.exponent) - ONE;
  const auto dFdz =
      static_cast<FltCalcType>(m_params.exponent) * std::pow(z, m_params.exponent - 1);

  return {fz, dFdz};
}

auto Newton::GetPolySinFuncValueAndDerivative(const std::complex<FltCalcType>& z) const noexcept
    -> FuncValueAndDerivative
{
  static constexpr auto FREQ = static_cast<FltCalcType>(1.0F);
  const auto fz              = (std::pow(z, m_params.exponent) * std::sin(FREQ * z)) - ONE;
  const auto dFdz            = std::pow(z, m_params.exponent - 1) *
                    ((static_cast<FltCalcType>(m_params.exponent) * std::sin(FREQ * z)) +
                     (z * (FREQ * std::cos(FREQ * z))));

  return {fz, dFdz};
}

auto Newton::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  const auto fullParamGroup = GetFullParamGroup({PARAM_GROUP, "newton"});
  return {
      GetPair(fullParamGroup, "exponent", m_params.exponent),
      GetPair(fullParamGroup, "a real", m_params.a.real()),
      GetPair(fullParamGroup, "a imag", m_params.a.imag()),
      GetPair(fullParamGroup, "c real", m_params.c.real()),
      GetPair(fullParamGroup, "c imag", m_params.c.imag()),
      GetPair(fullParamGroup, "amplitude", Point2dFlt{m_params.amplitude.x, m_params.amplitude.y}),
      GetPair(fullParamGroup,
              "lerpToOneTs",
              Point2dFlt{m_params.lerpToOneTs.xLerpT, m_params.lerpToOneTs.yLerpT}),
      GetPair(fullParamGroup, "useSqDistDenominator", m_params.useSqDistDenominator),
      GetPair(fullParamGroup, "denominator", m_params.denominator),
      GetPair(fullParamGroup, "useZSin", m_params.useZSinInput),
      GetPair(fullParamGroup,
              "zSinAmplitude",
              Point2dFlt{m_params.zSinAmplitude.x, m_params.zSinAmplitude.y}),
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
