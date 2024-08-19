module;

#include <cmath>
#include <complex>
#include <cstdint>

module Goom.FilterFx.FilterEffects.AdjustmentEffects.JuliaFlowField;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.EnumUtils;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Lib.Point2d;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{
using FILTER_UTILS::RandomViewport;
using UTILS::EnumMap;
using UTILS::NameValuePairs;
using UTILS::MATH::GetRandSeed;
using UTILS::MATH::GoomRand;
using UTILS::MATH::HALF;
using UTILS::MATH::NumberRange;
using UTILS::MATH::Sq;
using UTILS::MATH::THIRD;

namespace
{

struct CustomParamRanges
{
  float defaultAmplitude{};
  NumberRange<float> amplitudeRange{};
  uint32_t defaultMaxIterations{};
  NumberRange<uint32_t> maxIterationsRange{};
};

using enum JuliaFlowField::ZFuncTypes;

constexpr auto PARAM_RANGES = EnumMap<JuliaFlowField::ZFuncTypes, CustomParamRanges>{{{
    {STD_JULIA_FUNC,
     CustomParamRanges{.defaultAmplitude     = 1.0F,
                       .amplitudeRange       = NumberRange{0.5F, 10.0F},
                       .defaultMaxIterations = 8,
                       .maxIterationsRange   = NumberRange{2U, 10U}}},
    {CUBIC_JULIA_FUNC1,
     CustomParamRanges{.defaultAmplitude     = 1.0F,
                       .amplitudeRange       = NumberRange{0.5F, 10.0F},
                       .defaultMaxIterations = 2,
                       .maxIterationsRange   = NumberRange{2U, 20U}}},
    {CUBIC_JULIA_FUNC2,
     CustomParamRanges{.defaultAmplitude     = 1.0F,
                       .amplitudeRange       = NumberRange{0.5F, 10.0F},
                       .defaultMaxIterations = 2,
                       .maxIterationsRange   = NumberRange{2U, 20U}}},
    {SIN_JULIA_FUNC1,
     CustomParamRanges{.defaultAmplitude     = 1.0F,
                       .amplitudeRange       = NumberRange{0.5F, 10.0F},
                       .defaultMaxIterations = 8,
                       .maxIterationsRange   = NumberRange{2U, 20U}}},
    {SIN_JULIA_FUNC2,
     CustomParamRanges{.defaultAmplitude     = 1.0F,
                       .amplitudeRange       = NumberRange{0.5F, 10.0F},
                       .defaultMaxIterations = 8,
                       .maxIterationsRange   = NumberRange{2U, 20U}}},
    {COS_JULIA_FUNC1,
     CustomParamRanges{.defaultAmplitude     = 1.0F,
                       .amplitudeRange       = NumberRange{0.5F, 10.0F},
                       .defaultMaxIterations = 8,
                       .maxIterationsRange   = NumberRange{2U, 20U}}},
    {COS_JULIA_FUNC2,
     CustomParamRanges{.defaultAmplitude     = 1.0F,
                       .amplitudeRange       = NumberRange{0.5F, 10.0F},
                       .defaultMaxIterations = 8,
                       .maxIterationsRange   = NumberRange{2U, 20U}}},
}}};

constexpr auto DEFAULT_AMPLITUDE      = Amplitude{1.0F, 1.0F};
constexpr auto DEFAULT_MAX_ITERATIONS = 8U;

constexpr auto DEFAULT_LERP_TO_ONE_T_S = LerpToOneTs{.xLerpT = 0.5F, .yLerpT = 0.5F};
constexpr auto LERP_TO_ONE_T_RANGE     = NumberRange{0.5F, 1.0F};

constexpr auto DEFAULT_C = std::complex<float>{0.5F, 0.5F};
constexpr auto C_RANGE   = NumberRange{NormalizedCoords::MIN_COORD, NormalizedCoords::MAX_COORD};

constexpr auto VIEWPORT_BOUNDS = RandomViewport::Bounds{
    .minSideLength       = 0.1F,
    .probUseCentredSides = 0.5F,
    .rect                = {.minMaxXMin = {.minValue = -2.0F, .maxValue = +1.0F},
                            .minMaxYMin = {.minValue = -2.0F, .maxValue = +1.0F},
                            .minMaxXMax = {.minValue = -2.0F + 0.1F, .maxValue = +2.0F},
                            .minMaxYMax = {.minValue = -2.0F + 0.1F, .maxValue = +2.0F}},
    .sides               = {.minMaxWidth  = {.minValue = 0.1F, .maxValue = 4.0F},
                            .minMaxHeight = {.minValue = 0.1F, .maxValue = 4.0F}}
};

constexpr auto PROB_XY_AMPLITUDES_EQUAL   = 0.98F;
constexpr auto PROB_LERP_TO_ONE_T_S_EQUAL = 0.95F;
constexpr auto PROB_MULTIPLY_VELOCITY     = 0.2F;

// NOLINTBEGIN(readability-identifier-length)
constexpr auto StdJuliaFunc(const std::complex<float>& z, const std::complex<float>& c)
    -> std::complex<float>
{
  return (z * z) + c;
}

constexpr auto CubicJuliaFunc1(const std::complex<float>& z, const std::complex<float>& c)
    -> std::complex<float>
{
  return (HALF * (z * z)) + (THIRD * (z * z * z)) + c;
}

constexpr auto CubicJuliaFunc2(const std::complex<float>& z, const std::complex<float>& c)
    -> std::complex<float>
{
  return (z * z * z) + c;
}

constexpr auto SinJuliaFunc1(const std::complex<float>& z, const std::complex<float>& c)
    -> std::complex<float>
{
  return std::sin(z) * c;
}

constexpr auto SinJuliaFunc2(const std::complex<float>& z, const std::complex<float>& c)
    -> std::complex<float>
{
  return std::sin(z * z) + c;
}

constexpr auto CosJuliaFunc1(const std::complex<float>& z, const std::complex<float>& c)
    -> std::complex<float>
{
  return std::cos(z) + c;
}

constexpr auto CosJuliaFunc2(const std::complex<float>& z, const std::complex<float>& c)
    -> std::complex<float>
{
  return std::cos(z * z) + c;
}
// NOLINTEND(readability-identifier-length)

// The original Julia func
//    z = z*z*z*z + (z*z*z)/(z - 1.0F) + (z*z)/(z*z*z + 4.0F*z*z + 5.0F)  + c;

} // namespace

static constexpr auto STD_JULIA_FUNC_WEIGHT    = 1.0F;
static constexpr auto CUBIC_JULIA_FUNC1_WEIGHT = 1.0F;
static constexpr auto CUBIC_JULIA_FUNC2_WEIGHT = 1.0F;
static constexpr auto SIN_JULIA_FUNC1_WEIGHT   = 1.0F;
static constexpr auto SIN_JULIA_FUNC2_WEIGHT   = 1.0F;
static constexpr auto COS_JULIA_FUNC1_WEIGHT   = 1.0F;
static constexpr auto COS_JULIA_FUNC2_WEIGHT   = 1.0F;

JuliaFlowField::JuliaFlowField(const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_randomViewport{goomRand, VIEWPORT_BOUNDS},
    m_params{.viewport         = Viewport{},
             .amplitude        = DEFAULT_AMPLITUDE,
             .lerpToOneTs      = DEFAULT_LERP_TO_ONE_T_S,
             .c                = DEFAULT_C,
             .maxIterations    = DEFAULT_MAX_ITERATIONS,
             .multiplyVelocity = false,
             .zFunc            = StdJuliaFunc},
    m_zFuncWeights{
        goomRand,
{
            {.key = ZFuncTypes::STD_JULIA_FUNC,    .weight = STD_JULIA_FUNC_WEIGHT},
            {.key = ZFuncTypes::CUBIC_JULIA_FUNC1, .weight = CUBIC_JULIA_FUNC1_WEIGHT},
            {.key = ZFuncTypes::CUBIC_JULIA_FUNC2, .weight = CUBIC_JULIA_FUNC2_WEIGHT},
            {.key = ZFuncTypes::SIN_JULIA_FUNC1,   .weight = SIN_JULIA_FUNC1_WEIGHT},
            {.key = ZFuncTypes::SIN_JULIA_FUNC2,   .weight = SIN_JULIA_FUNC2_WEIGHT},
            {.key = ZFuncTypes::COS_JULIA_FUNC1,   .weight = COS_JULIA_FUNC1_WEIGHT},
            {.key = ZFuncTypes::COS_JULIA_FUNC2,   .weight = COS_JULIA_FUNC2_WEIGHT},
          }
    },
    m_zFuncs{{{
      {ZFuncTypes::STD_JULIA_FUNC,    StdJuliaFunc},
      {ZFuncTypes::CUBIC_JULIA_FUNC1, CubicJuliaFunc1},
      {ZFuncTypes::CUBIC_JULIA_FUNC2, CubicJuliaFunc2},
      {ZFuncTypes::SIN_JULIA_FUNC1,   SinJuliaFunc1},
      {ZFuncTypes::SIN_JULIA_FUNC2,   SinJuliaFunc2},
      {ZFuncTypes::COS_JULIA_FUNC1,   CosJuliaFunc1},
      {ZFuncTypes::COS_JULIA_FUNC2,   CosJuliaFunc2},
    }}}
{
}

auto JuliaFlowField::GetVelocity(const Vec2dFlt& baseZoomAdjustment,
                                 const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto viewportCoords = m_params.viewport.GetViewportCoords(coords);

  static constexpr auto TRAP_POINT = std::complex<float>{0.0F, 0.0F};
  const auto viewportTrapPoint =
      m_params.viewport.GetViewportCoords(NormalizedCoords{TRAP_POINT.real(), TRAP_POINT.imag()});
  const auto viewportC =
      m_params.viewport.GetViewportCoords(NormalizedCoords{m_params.c.real(), m_params.c.imag()});
  // NOLINTNEXTLINE(readability-identifier-length)
  const auto trapPoint = std::complex<float>{viewportTrapPoint.GetX(), viewportTrapPoint.GetY()};
  const auto c         = std::complex<float>{viewportC.GetX(), viewportC.GetY()};
  const auto z0        = std::complex<float>{viewportCoords.GetX(), viewportCoords.GetY()};

  const auto z = GetJuliaPoint(trapPoint, c, z0);

  const auto x = m_params.amplitude.x * z.real();
  const auto y = m_params.amplitude.y * z.imag();

  if (not m_params.multiplyVelocity)
  {
    return {.x = baseZoomAdjustment.x + x, .y = baseZoomAdjustment.y + y};
  }

  return {.x = viewportCoords.GetX() * x, .y = viewportCoords.GetY() * y};
}

auto JuliaFlowField::GetJuliaPoint(const std::complex<float>& trapPoint,
                                   const std::complex<float>& c,
                                   const std::complex<float>& z0) const noexcept
    -> std::complex<float>
{
  auto minDistSqToTrapPoint = 1.0e+10F;
  auto minPoint             = std::complex<float>{0.0F, 0.0F};

  auto z = z0;
  for (auto i = 0U; i < m_params.maxIterations; ++i)
  {
    z = m_params.zFunc(z, c);

    if (const auto distSq = std::norm(z); distSq > 4.0F)
    {
      //minPoint = 0.01F * std::log(z);
      minPoint = std::complex<float>{0.0F, 0.0F};
      break;
    }

    if (const auto distSqToTrapPoint = std::norm(z - trapPoint);
        minDistSqToTrapPoint > distSqToTrapPoint)
    {
      minDistSqToTrapPoint = distSqToTrapPoint;
      minPoint             = z;
    }
  }

  return minPoint;
}

auto JuliaFlowField::SetRandomParams() noexcept -> void
{
  const auto zFuncType = m_zFuncWeights.GetRandomWeighted();
  //const auto zFuncType = ZFuncTypes::STD_JULIA_FUNC;

  const auto viewport = m_randomViewport.GetRandomViewport();
  //const auto viewport = Viewport{};

  const auto& amplitudeRange = PARAM_RANGES[zFuncType].amplitudeRange;
  const auto xAmplitude      = m_goomRand->GetRandInRange(amplitudeRange);
  const auto yAmplitude      = m_goomRand->ProbabilityOf<PROB_XY_AMPLITUDES_EQUAL>()
                                   ? xAmplitude
                                   : m_goomRand->GetRandInRange(amplitudeRange);

  const auto xLerpToOneT = m_goomRand->GetRandInRange<LERP_TO_ONE_T_RANGE>();
  const auto yLerpToOneT = m_goomRand->ProbabilityOf<PROB_LERP_TO_ONE_T_S_EQUAL>()
                               ? xLerpToOneT
                               : m_goomRand->GetRandInRange<LERP_TO_ONE_T_RANGE>();

  // NOLINTNEXTLINE(readability-identifier-length)
  const auto c =
      std::complex<float>{m_goomRand->GetRandInRange(C_RANGE), m_goomRand->GetRandInRange(C_RANGE)};

  const auto maxIterations = m_goomRand->GetRandInRange(PARAM_RANGES[zFuncType].maxIterationsRange);

  const auto multiplyVelocity = m_goomRand->ProbabilityOf<PROB_MULTIPLY_VELOCITY>();

  const auto zFunc = m_zFuncs[zFuncType];

  SetParams({
      .viewport         = viewport,
      .amplitude        = {           xAmplitude,            yAmplitude},
      .lerpToOneTs      = {.xLerpT = xLerpToOneT, .yLerpT = yLerpToOneT},
      .c                = c,
      .maxIterations    = maxIterations,
      .multiplyVelocity = multiplyVelocity,
      .zFunc            = zFunc,
  });
}

auto JuliaFlowField::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  return {};
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
