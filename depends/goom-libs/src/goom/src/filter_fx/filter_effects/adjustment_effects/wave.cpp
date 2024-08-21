module;

#include <cmath>

module Goom.FilterFx.FilterEffects.AdjustmentEffects.Wave;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.Misc;
import Goom.Utils.Math.GoomRand;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using FILTER_UTILS::RandomViewport;
using UTILS::NameValuePairs;
using UTILS::MATH::GoomRand;
using UTILS::MATH::HALF_CIRCLE_RANGE;
using UTILS::MATH::NumberRange;

using enum Wave::WaveEffect;

static constexpr auto SQ_DIST_POWER_RANGE = NumberRange{0.15F, 1.1F};

static constexpr auto DEFAULT_PERIODIC_FACTOR         = 1.0F;
static constexpr auto DEFAULT_SIN_COS_PERIODIC_FACTOR = 0.5F;
static constexpr auto PERIODIC_FACTOR_RANGE           = NumberRange{0.5F, 1.0F};
static constexpr auto SIN_COS_PERIODIC_FACTOR_RANGE   = NumberRange{0.1F, 0.9F};

static constexpr auto FREQ_FACTOR_RANGE = NumberRange{1.0F, 50.0F};

static constexpr auto AMPLITUDE_RANGE = NumberRange{0.001F, 0.25F};

static constexpr auto REDUCER_COEFF_RANGE     = NumberRange{0.95F, 1.5F};
static constexpr auto TAN_REDUCER_COEFF_RANGE = NumberRange{4.0F, 10.0F};

static constexpr auto SPIRAL_ROTATE_FACTOR_RANGE = NumberRange{0.9F, 1.1F};
static constexpr auto MIN_SPIRAL_ROTATE_LERP     = 0.5F;
static constexpr auto MAX_SPIRAL_ROTATE_LERP     = 1.0F;

static constexpr auto MODIFIED_ATAN_ANGLE_FACTOR_RANGE = NumberRange{0.1F, 10.0F};

static constexpr auto VIEWPORT_BOUNDS = RandomViewport::Bounds{
    .minSideLength       = 0.1F,
    .probUseCentredSides = 1.0F,
    .rect                = {},
    .sides               = {.minMaxWidth  = {.minValue = 2.0F, .maxValue = 10.0F},
                            .minMaxHeight = {.minValue = 2.0F, .maxValue = 10.0F}}
};


// These give weird but interesting wave results
static constexpr auto SMALL_FREQ_FACTOR_RANGE           = NumberRange{0.001F, 0.1F};
static constexpr auto BIG_AMPLITUDE_RANGE               = NumberRange{1.0F, 50.0F};
static constexpr auto BIG_PERIODIC_FACTOR_RANGE         = NumberRange{50.0F, 100.0F};
static constexpr auto BIG_SIN_COS_PERIODIC_FACTOR_RANGE = NumberRange{10.0F, 90.0F};

static constexpr auto PROB_ALLOW_STRANGE_WAVE_VALUES          = 0.1F;
static constexpr auto PROB_WAVE_XY_EFFECTS_EQUAL              = 0.75F;
static constexpr auto PROB_NO_PERIODIC_FACTOR                 = 0.2F;
static constexpr auto PROB_PERIODIC_FACTOR_USES_X_WAVE_EFFECT = 0.9F;
static constexpr auto PROB_SPIRAL_SQ_DIST_EFFECT              = 0.0F;
static constexpr auto PROB_USE_MODIFIED_ATAN_ANGLE            = 0.5F;
static constexpr auto PROB_NO_VIEWPORT                        = 0.5F;

static constexpr auto WAVE_SIN_EFFECT_WEIGHT      = 200.0F;
static constexpr auto WAVE_COS_EFFECT_WEIGHT      = 200.0F;
static constexpr auto WAVE_SIN_COS_EFFECT_WEIGHT  = 150.0F;
static constexpr auto WAVE_TAN_EFFECT_WEIGHT      = 010.0F;
static constexpr auto WAVE_TAN_SIN_EFFECT_WEIGHT  = 010.0F;
static constexpr auto CWAVE_TAN_COS_EFFECT_WEIGHT = 010.0F;
static constexpr auto WAVE_COT_EFFECT_WEIGHT      = 001.0F;
static constexpr auto WAVE_COT_SIN_EFFECT_WEIGHT  = 001.0F;
static constexpr auto WAVE_COT_COS_EFFECT_WEIGHT  = 001.0F;

inline auto Wave::GetSqDistEffect() const noexcept -> Wave::AngleEffect
{
  if (m_goomRand->ProbabilityOf<PROB_SPIRAL_SQ_DIST_EFFECT>())
  {
    return AngleEffect::SQ_DIST_AND_SPIRAL;
  }
  return AngleEffect::SQ_DIST;
}

Wave::Wave(const Modes mode, const GoomRand& goomRand)
  : m_mode{mode},
    m_goomRand{&goomRand},
    m_randomViewport{goomRand, VIEWPORT_BOUNDS},
    m_weightedEffects{
      *m_goomRand,
      {
        {    .key=WAVE_SIN_EFFECT,      .weight=WAVE_SIN_EFFECT_WEIGHT},
        {    .key=WAVE_COS_EFFECT,      .weight=WAVE_COS_EFFECT_WEIGHT},
        {.key=WAVE_SIN_COS_EFFECT,  .weight=WAVE_SIN_COS_EFFECT_WEIGHT},
        {    .key=WAVE_TAN_EFFECT,      .weight=WAVE_TAN_EFFECT_WEIGHT},
        {.key=WAVE_TAN_SIN_EFFECT,  .weight=WAVE_TAN_SIN_EFFECT_WEIGHT},
        {.key=WAVE_TAN_COS_EFFECT, .weight=CWAVE_TAN_COS_EFFECT_WEIGHT},
        {    .key=WAVE_COT_EFFECT,      .weight=WAVE_COT_EFFECT_WEIGHT},
        {.key=WAVE_COT_SIN_EFFECT,  .weight=WAVE_COT_SIN_EFFECT_WEIGHT},
        {.key=WAVE_COT_COS_EFFECT,  .weight=WAVE_COT_COS_EFFECT_WEIGHT},
      }
    },
    m_params{GetRandomParams()}
{
  m_randomViewport.SetProbNoViewport(PROB_NO_VIEWPORT);
}

auto Wave::GetRandomParams() const noexcept -> Params
{
  switch (m_mode)
  {
    case Modes::SQ_DIST_ANGLE_EFFECT_MODE0:
      return GetSqDistAngleEffectMode0RandomParams();
    case Modes::SQ_DIST_ANGLE_EFFECT_MODE1:
      return GetSqDistAngleEffectMode1RandomParams();
    case Modes::ATAN_ANGLE_EFFECT_MODE0:
      return GetAtanAngleEffectMode0RandomParams();
    case Modes::ATAN_ANGLE_EFFECT_MODE1:
      return GetAtanAngleEffectMode1RandomParams();
  }
}

auto Wave::GetSqDistAngleEffectMode0RandomParams() const noexcept -> Params
{
  return GetWaveModeSettings({.angleEffect               = GetSqDistEffect(),
                              .freqFactorRange           = FREQ_FACTOR_RANGE,
                              .amplitudeRange            = AMPLITUDE_RANGE,
                              .periodicFactorRange       = PERIODIC_FACTOR_RANGE,
                              .sinCosPeriodicFactorRange = SIN_COS_PERIODIC_FACTOR_RANGE});
}

auto Wave::GetSqDistAngleEffectMode1RandomParams() const noexcept -> Params
{
  if (m_goomRand->ProbabilityOf<PROB_ALLOW_STRANGE_WAVE_VALUES>())
  {
    return GetWaveModeSettings({.angleEffect               = GetSqDistEffect(),
                                .freqFactorRange           = SMALL_FREQ_FACTOR_RANGE,
                                .amplitudeRange            = BIG_AMPLITUDE_RANGE,
                                .periodicFactorRange       = BIG_PERIODIC_FACTOR_RANGE,
                                .sinCosPeriodicFactorRange = BIG_SIN_COS_PERIODIC_FACTOR_RANGE});
  }

  return GetWaveModeSettings({.angleEffect               = GetSqDistEffect(),
                              .freqFactorRange           = FREQ_FACTOR_RANGE,
                              .amplitudeRange            = AMPLITUDE_RANGE,
                              .periodicFactorRange       = PERIODIC_FACTOR_RANGE,
                              .sinCosPeriodicFactorRange = SIN_COS_PERIODIC_FACTOR_RANGE});
}

auto Wave::GetAtanAngleEffectMode0RandomParams() const noexcept -> Params
{
  return GetWaveModeSettings({.angleEffect               = AngleEffect::ATAN,
                              .freqFactorRange           = FREQ_FACTOR_RANGE,
                              .amplitudeRange            = AMPLITUDE_RANGE,
                              .periodicFactorRange       = PERIODIC_FACTOR_RANGE,
                              .sinCosPeriodicFactorRange = SIN_COS_PERIODIC_FACTOR_RANGE});
}

auto Wave::GetAtanAngleEffectMode1RandomParams() const noexcept -> Params
{
  if (m_goomRand->ProbabilityOf<PROB_ALLOW_STRANGE_WAVE_VALUES>())
  {
    return GetWaveModeSettings({.angleEffect               = AngleEffect::ATAN,
                                .freqFactorRange           = SMALL_FREQ_FACTOR_RANGE,
                                .amplitudeRange            = BIG_AMPLITUDE_RANGE,
                                .periodicFactorRange       = BIG_PERIODIC_FACTOR_RANGE,
                                .sinCosPeriodicFactorRange = BIG_SIN_COS_PERIODIC_FACTOR_RANGE});
  }

  return GetWaveModeSettings({.angleEffect               = AngleEffect::ATAN,
                              .freqFactorRange           = FREQ_FACTOR_RANGE,
                              .amplitudeRange            = AMPLITUDE_RANGE,
                              .periodicFactorRange       = PERIODIC_FACTOR_RANGE,
                              .sinCosPeriodicFactorRange = SIN_COS_PERIODIC_FACTOR_RANGE});
}

auto Wave::GetWaveModeSettings(const WaveModeSettings& waveModeSettings) const noexcept -> Params
{
  const auto viewport = m_randomViewport.GetRandomViewport();

  const auto waveEffectsEqual = m_goomRand->ProbabilityOf<PROB_WAVE_XY_EFFECTS_EQUAL>();

  const auto xWaveEffect = m_weightedEffects.GetRandomWeighted();
  const auto yWaveEffect = waveEffectsEqual ? xWaveEffect : m_weightedEffects.GetRandomWeighted();

  const auto sqDistPower = m_goomRand->GetRandInRange<SQ_DIST_POWER_RANGE>();

  const auto periodicFactor        = GetPeriodicFactor(xWaveEffect,
                                                yWaveEffect,
                                                waveModeSettings.periodicFactorRange,
                                                waveModeSettings.sinCosPeriodicFactorRange);
  const auto freqFactor            = m_goomRand->GetRandInRange(waveModeSettings.freqFactorRange);
  const auto amplitude             = m_goomRand->GetRandInRange(waveModeSettings.amplitudeRange);
  const auto reducerCoeff          = GetReducerCoeff(xWaveEffect, yWaveEffect, periodicFactor);
  const auto spiralRotateBaseAngle = m_goomRand->GetRandInRange<HALF_CIRCLE_RANGE>();
  const auto useModifiedATanAngle  = m_goomRand->ProbabilityOf<PROB_USE_MODIFIED_ATAN_ANGLE>();
  const auto modifiedATanAngleFactor =
      m_goomRand->GetRandInRange<MODIFIED_ATAN_ANGLE_FACTOR_RANGE>();

  return {.viewport                = viewport,
          .xWaveEffect             = xWaveEffect,
          .yWaveEffect             = yWaveEffect,
          .angleEffect             = waveModeSettings.angleEffect,
          .sqDistPower             = sqDistPower,
          .freqFactor              = freqFactor,
          .amplitude               = amplitude,
          .periodicFactor          = periodicFactor,
          .reducerCoeff            = reducerCoeff,
          .spiralRotateBaseAngle   = spiralRotateBaseAngle,
          .useModifiedATanAngle    = useModifiedATanAngle,
          .modifiedATanAngleFactor = modifiedATanAngleFactor};
}

inline auto Wave::GetReducerCoeff(const WaveEffect xWaveEffect,
                                  [[maybe_unused]] const WaveEffect yWaveEffect,
                                  const float periodicFactor) const noexcept -> float
{
  switch (xWaveEffect)
  {
    case WAVE_SIN_EFFECT:
    case WAVE_COS_EFFECT:
    case WAVE_SIN_COS_EFFECT:
      return m_goomRand->GetRandInRange<REDUCER_COEFF_RANGE>();
    case WAVE_TAN_EFFECT:
    case WAVE_COT_EFFECT:
      return m_goomRand->GetRandInRange<TAN_REDUCER_COEFF_RANGE>();
    case WAVE_TAN_SIN_EFFECT:
    case WAVE_TAN_COS_EFFECT:
    case WAVE_COT_SIN_EFFECT:
    case WAVE_COT_COS_EFFECT:
      return std::lerp(m_goomRand->GetRandInRange<TAN_REDUCER_COEFF_RANGE>(),
                       m_goomRand->GetRandInRange<REDUCER_COEFF_RANGE>(),
                       periodicFactor);
  }
}

inline auto Wave::GetPeriodicFactor(
    const WaveEffect xWaveEffect,
    const WaveEffect yWaveEffect,
    const NumberRange<float>& periodicFactorRange,
    const NumberRange<float>& sinCosPeriodicFactorRange) const noexcept -> float
{
  if (m_goomRand->ProbabilityOf<PROB_NO_PERIODIC_FACTOR>())
  {
    return xWaveEffect == WAVE_SIN_COS_EFFECT ? DEFAULT_SIN_COS_PERIODIC_FACTOR
                                              : DEFAULT_PERIODIC_FACTOR;
  }
  if (m_goomRand->ProbabilityOf<PROB_PERIODIC_FACTOR_USES_X_WAVE_EFFECT>())
  {
    return m_goomRand->GetRandInRange(xWaveEffect == WAVE_SIN_COS_EFFECT ? sinCosPeriodicFactorRange
                                                                         : periodicFactorRange);
  }

  return m_goomRand->GetRandInRange(yWaveEffect == WAVE_SIN_COS_EFFECT ? sinCosPeriodicFactorRange
                                                                       : periodicFactorRange);
}

auto Wave::GetPeriodicPart(const WaveEffect waveEffect,
                           const float angle,
                           const float periodicFactor) noexcept -> float
{
  switch (waveEffect)
  {
    case WAVE_SIN_EFFECT:
      return periodicFactor * std::sin(angle);
    case WAVE_COS_EFFECT:
      return periodicFactor * std::cos(angle);
    case WAVE_SIN_COS_EFFECT:
      return periodicFactor * std::lerp(std::sin(angle), std::cos(angle), periodicFactor);
    case WAVE_TAN_EFFECT:
      return periodicFactor * std::tan(angle);
    case WAVE_TAN_SIN_EFFECT:
      return periodicFactor * std::lerp(std::tan(angle), std::sin(angle), periodicFactor);
    case WAVE_TAN_COS_EFFECT:
      return periodicFactor * std::lerp(std::tan(angle), std::cos(angle), periodicFactor);
    case WAVE_COT_EFFECT:
      return periodicFactor * std::tan(UTILS::MATH::HALF_PI - angle);
    case WAVE_COT_SIN_EFFECT:
      return periodicFactor *
             std::lerp(std::tan(UTILS::MATH::HALF_PI - angle), std::sin(angle), periodicFactor);
    case WAVE_COT_COS_EFFECT:
      return periodicFactor *
             std::lerp(std::tan(UTILS::MATH::HALF_PI - angle), std::cos(angle), periodicFactor);
  }
}

auto Wave::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  return {};
}

auto Wave::GetAngle(const float sqDistFromZero,
                    const NormalizedCoords& coords) const noexcept -> float
{
  switch (m_params.angleEffect)
  {
    case AngleEffect::ATAN:
      return GetATanAngle(coords);
    case AngleEffect::SQ_DIST:
      return std::pow(sqDistFromZero, m_params.sqDistPower);
    case AngleEffect::SQ_DIST_AND_SPIRAL:
      return GetSqDistSpiralRotateAngle(sqDistFromZero, coords);
  }
}

auto Wave::GetATanAngle(const NormalizedCoords& coords) const noexcept -> float
{
  const auto atanAngle = std::atan2(coords.GetY(), coords.GetX());
  if (m_params.useModifiedATanAngle)
  {
    return std::sin(m_params.modifiedATanAngleFactor * atanAngle) * atanAngle;
  }
  return atanAngle;
}

auto Wave::GetSqDistSpiralRotateAngle(const float sqDistFromZero,
                                      const NormalizedCoords& coords) const noexcept -> float
{
  const auto spiralRotateAngle =
      m_goomRand->GetRandInRange<SPIRAL_ROTATE_FACTOR_RANGE>() * m_params.spiralRotateBaseAngle;
  const auto sinSpiralRotateAngle = std::sin(spiralRotateAngle);
  const auto cosSpiralRotateAngle = std::cos(spiralRotateAngle);
  const auto x = (coords.GetX() * cosSpiralRotateAngle) - (coords.GetY() * sinSpiralRotateAngle);
  const auto y = (coords.GetY() * cosSpiralRotateAngle) + (coords.GetX() * sinSpiralRotateAngle);

  const auto t     = (UTILS::MATH::PI + std::atan2(y, x)) / UTILS::MATH::TWO_PI;
  const auto angle = std::lerp(MIN_SPIRAL_ROTATE_LERP, MAX_SPIRAL_ROTATE_LERP, t);

  return std::pow(sqDistFromZero, m_params.sqDistPower) * angle;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
