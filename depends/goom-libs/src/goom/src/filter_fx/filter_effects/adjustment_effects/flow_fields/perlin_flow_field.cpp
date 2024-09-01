module;

#include <PerlinNoise.hpp>
#include <algorithm>
#include <cmath>
#include <mdspan.hpp>

module Goom.FilterFx.FilterEffects.AdjustmentEffects.PerlinFlowField;

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
using UTILS::MATH::PI;
using UTILS::MATH::Sq;
using UTILS::MATH::TWO_PI;

namespace
{

using PerlinSeedType = siv::BasicPerlinNoise<float>::seed_type;

auto GetRandSeedForPerlinNoise() -> PerlinSeedType
{
  if constexpr (std::is_same_v<PerlinSeedType, decltype(GetRandSeed())>)
  {
    return GetRandSeed();
  }

#if defined(__GNUC__) and not defined(__clang_major__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif

  return static_cast<PerlinSeedType>(GetRandSeed() % std::numeric_limits<PerlinSeedType>::max());

#if defined(__GNUC__) and not defined(__clang_major__)
#pragma GCC diagnostic pop
#endif
}

constexpr auto AMPLITUDE_RANGE         = NumberRange{0.15F, 0.5F};
constexpr auto LERP_TO_ONE_T_RANGE     = NumberRange{0.1F, 1.0F};
constexpr auto NOISE_FREQ_FACTOR_RANGE = NumberRange{0.005F, 1.000F};
constexpr auto ANGLE_FREQ_FACTOR_RANGE = NumberRange{0.5F, 5.0F};
constexpr auto MIN_ANGLE_RANGE         = NumberRange{PI, TWO_PI};
constexpr auto OCTAVES_RANGE           = NumberRange{1, 5};
constexpr auto PERSISTENCE_RANGE       = NumberRange{0.1F, 1.0F};
constexpr auto NOISE_FACTOR_RANGE      = NumberRange{0.05F, 1.00F};

static_assert((PI <= MIN_ANGLE_RANGE.min) and (MIN_ANGLE_RANGE.max <= TWO_PI));

constexpr auto PROB_XY_AMPLITUDES_EQUAL   = 0.98F;
constexpr auto PROB_LERP_TO_ONE_T_S_EQUAL = 0.95F;
constexpr auto PROB_XY_NOISE_FREQ_EQUAL   = 0.5F;
constexpr auto PROB_XY_ANGLE_FREQ_EQUAL   = 0.5F;
constexpr auto PROB_OCTAVES_EQUAL         = 0.5F;
constexpr auto PROB_PERSISTENCE_EQUAL     = 0.5F;
constexpr auto PROB_MULTIPLY_VELOCITY     = 0.1F;

} // namespace

PerlinFlowField::PerlinFlowField(const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_perlinNoise{GetRandSeedForPerlinNoise()},
    m_perlinNoise2{GetRandSeedForPerlinNoise()},
    m_params{GetRandomParams()}
{
  SetupAngles();
}

auto PerlinFlowField::SetupAngles() noexcept -> void
{
  const auto setupFunc = [this](const uint32_t x, const uint32_t y) -> FlowFieldGrid::PolarCoords
  {
    const auto xFlt =
        2.0F * (-0.5F + (static_cast<float>(x) / static_cast<float>(FlowFieldGrid::GRID_WIDTH)));
    const auto yFlt =
        2.0F * (-0.5F + (static_cast<float>(y) / static_cast<float>(FlowFieldGrid::GRID_HEIGHT)));

    const auto distFromCentre = std::sqrt(Sq(xFlt) + Sq(yFlt));

    const auto xFreq = (0.1F + distFromCentre) * m_params.noiseFreqFactor.x;
    const auto yFreq = (0.1F + distFromCentre) * m_params.noiseFreqFactor.y;
    //const auto xFreq = m_params.noiseFrequencyFactor.x;
    //const auto yFreq = m_params.noiseFrequencyFactor.y;
    // const auto xFreq = 0.01F;
    // const auto yFreq  = 0.01F;

    const auto xNoise = m_perlinNoise.octave2D_11(xFreq * static_cast<float>(x),
                                                  xFreq * static_cast<float>(y),
                                                  m_params.octaves1,
                                                  m_params.persistence1);
    const auto yNoise = m_perlinNoise2.octave2D_11(yFreq * static_cast<float>(x),
                                                   yFreq * static_cast<float>(y),
                                                   m_params.octaves2,
                                                   m_params.persistence2);

    // const auto xNoise = m_perlinNoise.octave2D_11(
    //     xFreq * static_cast<float>(col), xFreq * static_cast<float>(row), 5, 0.5F);
    // const auto yNoise = m_perlinNoise2.octave2D_11(
    //     yFreq * static_cast<float>(col), yFreq * static_cast<float>(row), 2, 1.0F);

    const auto noise = m_params.noiseFactor * (0.5F * (xNoise + yNoise));
    const auto angle = distFromCentre * std::lerp(m_params.minAngle, TWO_PI, noise);

    return {.angle = angle, .radius = distFromCentre};
  };

  m_gridArray.Initialize(setupFunc);
}

auto PerlinFlowField::GetVelocity(const Vec2dFlt& baseZoomAdjustment,
                                  const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto gridPolarCoords = m_gridArray.GetPolarCoords(coords);
  const auto distFromCentre  = gridPolarCoords.radius;

  const auto x = (m_params.amplitude.x * distFromCentre) *
                 std::cos(m_params.angleFreqFactor.x * gridPolarCoords.angle);
  const auto y = (m_params.amplitude.y * distFromCentre) *
                 std::sin(m_params.angleFreqFactor.y * gridPolarCoords.angle);

  if (not m_params.multiplyVelocity)
  {
    return {.x = baseZoomAdjustment.x + x, .y = baseZoomAdjustment.y + y};
  }

  return {.x = coords.GetX() * x, .y = coords.GetY() * y};
}

auto PerlinFlowField::GetRandomParams() const noexcept -> Params
{
  const auto xAmplitude = m_goomRand->GetRandInRange<AMPLITUDE_RANGE>();
  const auto yAmplitude = m_goomRand->ProbabilityOf<PROB_XY_AMPLITUDES_EQUAL>()
                              ? xAmplitude
                              : m_goomRand->GetRandInRange<AMPLITUDE_RANGE>();

  const auto xLerpToOneT = m_goomRand->GetRandInRange<LERP_TO_ONE_T_RANGE>();
  const auto yLerpToOneT = m_goomRand->ProbabilityOf<PROB_LERP_TO_ONE_T_S_EQUAL>()
                               ? xLerpToOneT
                               : m_goomRand->GetRandInRange<LERP_TO_ONE_T_RANGE>();

  const auto xNoiseFreqFactor = m_goomRand->GetRandInRange<NOISE_FREQ_FACTOR_RANGE>();
  const auto yNoiseFreqFactor = m_goomRand->ProbabilityOf<PROB_XY_NOISE_FREQ_EQUAL>()
                                    ? xNoiseFreqFactor
                                    : m_goomRand->GetRandInRange<NOISE_FREQ_FACTOR_RANGE>();

  const auto xAngleFreqFactor = m_goomRand->GetRandInRange<ANGLE_FREQ_FACTOR_RANGE>();
  const auto yAngleFreqFactor = m_goomRand->ProbabilityOf<PROB_XY_ANGLE_FREQ_EQUAL>()
                                    ? xAngleFreqFactor
                                    : m_goomRand->GetRandInRange<ANGLE_FREQ_FACTOR_RANGE>();

  const auto minAngle = m_goomRand->GetRandInRange<MIN_ANGLE_RANGE>();

  const auto octaves1 = m_goomRand->GetRandInRange<OCTAVES_RANGE>();
  const auto octaves2 = m_goomRand->ProbabilityOf<PROB_OCTAVES_EQUAL>()
                            ? octaves1
                            : m_goomRand->GetRandInRange<OCTAVES_RANGE>();

  const auto persistence1 = m_goomRand->GetRandInRange<PERSISTENCE_RANGE>();
  const auto persistence2 = m_goomRand->ProbabilityOf<PROB_PERSISTENCE_EQUAL>()
                                ? persistence1
                                : m_goomRand->GetRandInRange<PERSISTENCE_RANGE>();

  const auto noiseFactor = m_goomRand->GetRandInRange<NOISE_FACTOR_RANGE>();

  const auto multiplyVelocity = m_goomRand->ProbabilityOf<PROB_MULTIPLY_VELOCITY>();

  return {
      .amplitude        = {           xAmplitude,            yAmplitude},
      .lerpToOneTs      = {.xLerpT = xLerpToOneT, .yLerpT = yLerpToOneT},
      .noiseFreqFactor  = {.x = xNoiseFreqFactor, .y = yNoiseFreqFactor},
      .angleFreqFactor  = {.x = xAngleFreqFactor, .y = yAngleFreqFactor},
      .minAngle         = minAngle,
      .octaves1         = octaves1,
      .persistence1     = persistence1,
      .octaves2         = octaves2,
      .persistence2     = persistence2,
      .noiseFactor      = noiseFactor,
      .multiplyVelocity = multiplyVelocity
  };
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
auto PerlinFlowField::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  return {};
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
