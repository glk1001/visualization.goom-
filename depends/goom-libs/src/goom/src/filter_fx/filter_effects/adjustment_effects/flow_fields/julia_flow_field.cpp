module;

#include <cmath>
#include <complex>
#include <cstdint>

module Goom.FilterFx.FilterEffects.AdjustmentEffects.JuliaFlowField;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Lib.Point2d;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{
using UTILS::NameValuePairs;
using UTILS::MATH::GetRandSeed;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;
using UTILS::MATH::Sq;

namespace
{

constexpr auto DEFAULT_AMPLITUDE = Amplitude{1.0F, 1.0F};
constexpr auto AMPLITUDE_RANGE   = NumberRange{0.5F, 3.0F};

constexpr auto DEFAULT_LERP_TO_ONE_T_S = LerpToOneTs{.xLerpT = 0.5F, .yLerpT = 0.5F};
constexpr auto LERP_TO_ONE_T_RANGE     = NumberRange{0.0F, 1.0F};

constexpr auto DEFAULT_C = std::complex<float>{0.5F, 0.5F};
constexpr auto C_RANGE   = NumberRange{NormalizedCoords::MIN_COORD, NormalizedCoords::MAX_COORD};

constexpr auto DEFAULT_MAX_ITERATIONS = 8;
constexpr auto MAX_ITERATIONS_RANGE   = NumberRange{2, 20};

constexpr auto DEFAULT_MAX_Z = 4.0F;
constexpr auto MAX_Z_RANGE   = NumberRange{2.0F, 6.0F};

constexpr auto PROB_XY_AMPLITUDES_EQUAL   = 0.98F;
constexpr auto PROB_LERP_TO_ONE_T_S_EQUAL = 0.95F;
constexpr auto PROB_MULTIPLY_VELOCITY     = 0.2F;

} // namespace

JuliaFlowField::JuliaFlowField(const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_params{.amplitude        = DEFAULT_AMPLITUDE,
             .lerpToOneTs      = DEFAULT_LERP_TO_ONE_T_S,
             .c                = DEFAULT_C,
             .maxIterations    = DEFAULT_MAX_ITERATIONS,
             .maxZ             = DEFAULT_MAX_Z,
             .multiplyVelocity = false}
{
}

auto JuliaFlowField::GetVelocity(const Vec2dFlt& baseZoomAdjustment,
                                 const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  static constexpr auto RESET_POINT = std::complex<float>{2.0F, 2.0F};

  const auto c = m_params.c; // NOLINT(readability-identifier-length)

  auto z = std::complex<float>{coords.GetX(), coords.GetY()};
  for (auto i = 0; i < m_params.maxIterations; ++i)
  {
    z = (z * z) + c;
    if (std::abs(z) > m_params.maxZ)
    {
      z = RESET_POINT + std::sin(z);
      break;
    }
  }

  const auto x = m_params.amplitude.x * z.real();
  const auto y = m_params.amplitude.y * z.imag();

  if (not m_params.multiplyVelocity)
  {
    return {.x = baseZoomAdjustment.x + x, .y = baseZoomAdjustment.y + y};
  }

  return {.x = coords.GetX() * x, .y = coords.GetY() * y};
}

auto JuliaFlowField::SetRandomParams() noexcept -> void
{
  const auto xAmplitude = m_goomRand->GetRandInRange<AMPLITUDE_RANGE>();
  const auto yAmplitude = m_goomRand->ProbabilityOf<PROB_XY_AMPLITUDES_EQUAL>()
                              ? xAmplitude
                              : m_goomRand->GetRandInRange<AMPLITUDE_RANGE>();

  const auto xLerpToOneT = m_goomRand->GetRandInRange<LERP_TO_ONE_T_RANGE>();
  const auto yLerpToOneT = m_goomRand->ProbabilityOf<PROB_LERP_TO_ONE_T_S_EQUAL>()
                               ? xLerpToOneT
                               : m_goomRand->GetRandInRange<LERP_TO_ONE_T_RANGE>();

  // NOLINTNEXTLINE(readability-identifier-length)
  const auto c =
      std::complex<float>{m_goomRand->GetRandInRange(C_RANGE), m_goomRand->GetRandInRange(C_RANGE)};

  const auto maxIterations = m_goomRand->GetRandInRange<MAX_ITERATIONS_RANGE>();

  const auto maxZ = m_goomRand->GetRandInRange<MAX_Z_RANGE>();

  const auto multiplyVelocity = m_goomRand->ProbabilityOf<PROB_MULTIPLY_VELOCITY>();

  SetParams({
      .amplitude        = {           xAmplitude,            yAmplitude},
      .lerpToOneTs      = {.xLerpT = xLerpToOneT, .yLerpT = yLerpToOneT},
      .c                = c,
      .maxIterations    = maxIterations,
      .maxZ             = maxZ,
      .multiplyVelocity = multiplyVelocity
  });
}

auto JuliaFlowField::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  return {};
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
