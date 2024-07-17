module;

#include <cmath>

module Goom.FilterFx.FilterEffects.AdjustmentEffects.Wave;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.Misc;
import Goom.Utils.Math.GoomRandBase;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using FILTER_UTILS::RandomViewport;
using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::NumberRange;
using UTILS::MATH::PI;

using enum Wave::WaveEffect;

static constexpr auto DEFAULT_WAVE_EFFECT  = WAVE_SIN_EFFECT;
static constexpr auto DEFAULT_ANGLE_EFFECT = Wave::AngleEffect::SQ_DIST;

static constexpr auto DEFAULT_SQ_DIST_POWER = 1.0F;
static constexpr auto SQ_DIST_POWER_RANGE   = NumberRange{0.15F, 1.1F};

static constexpr auto DEFAULT_PERIODIC_FACTOR         = 1.0F;
static constexpr auto DEFAULT_SIN_COS_PERIODIC_FACTOR = 0.5F;
static constexpr auto PERIODIC_FACTOR_RANGE           = NumberRange{0.5F, 1.0F};
static constexpr auto SIN_COS_PERIODIC_FACTOR_RANGE   = NumberRange{0.1F, 0.9F};

static constexpr auto DEFAULT_FREQ_FACTOR = 20.0F;
static constexpr auto FREQ_FACTOR_RANGE   = NumberRange{1.0F, 50.0F};

static constexpr auto DEFAULT_AMPLITUDE = 0.01F;
static constexpr auto AMPLITUDE_RANGE   = NumberRange{0.001F, 0.25F};

static constexpr auto DEFAULT_REDUCER_COEFF   = 1.0F;
static constexpr auto REDUCER_COEFF_RANGE     = NumberRange{0.95F, 1.5F};
static constexpr auto TAN_REDUCER_COEFF_RANGE = NumberRange{4.0F, 10.0F};

static constexpr auto DEFAULT_SPIRAL_ROTATE_BASE_ANGLE = 0.25F * UTILS::MATH::PI;
static constexpr auto SPIRAL_ROTATE_FACTOR_RANGE       = NumberRange{0.9F, 1.1F};
static constexpr auto MIN_SPIRAL_ROTATE_LERP           = 0.5F;
static constexpr auto MAX_SPIRAL_ROTATE_LERP           = 1.0F;

static constexpr auto DEFAULT_USE_MODIFIED_ATAN_ANGLE    = false;
static constexpr auto DEFAULT_MODIFIED_ATAN_ANGLE_FACTOR = 1.0F;
static constexpr auto MODIFIED_ATAN_ANGLE_FACTOR_RANGE   = NumberRange{0.1F, 10.0F};

static constexpr auto VIEWPORT_BOUNDS = RandomViewport::Bounds{
    .minSideLength       = 0.1F,
    .probUseCentredSides = 1.0F,
    .rect                = {},
    .sides               = {.minMaxWidth = {2.0F, 10.0F}, .minMaxHeight = {2.0F, 10.0F}}
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
  if (m_goomRand->ProbabilityOf(PROB_SPIRAL_SQ_DIST_EFFECT))
  {
    return AngleEffect::SQ_DIST_AND_SPIRAL;
  }
  return AngleEffect::SQ_DIST;
}

Wave::Wave(const Modes mode, const IGoomRand& goomRand)
  : m_mode{mode},
    m_goomRand{&goomRand},
    m_randomViewport{goomRand, VIEWPORT_BOUNDS},
    m_weightedEffects{
      *m_goomRand,
      {
        {    WAVE_SIN_EFFECT,      WAVE_SIN_EFFECT_WEIGHT},
        {    WAVE_COS_EFFECT,      WAVE_COS_EFFECT_WEIGHT},
        {WAVE_SIN_COS_EFFECT,  WAVE_SIN_COS_EFFECT_WEIGHT},
        {    WAVE_TAN_EFFECT,      WAVE_TAN_EFFECT_WEIGHT},
        {WAVE_TAN_SIN_EFFECT,  WAVE_TAN_SIN_EFFECT_WEIGHT},
        {WAVE_TAN_COS_EFFECT, CWAVE_TAN_COS_EFFECT_WEIGHT},
        {    WAVE_COT_EFFECT,      WAVE_COT_EFFECT_WEIGHT},
        {WAVE_COT_SIN_EFFECT,  WAVE_COT_SIN_EFFECT_WEIGHT},
        {WAVE_COT_COS_EFFECT,  WAVE_COT_COS_EFFECT_WEIGHT},
      }
    },
    m_params{Viewport{},
             DEFAULT_WAVE_EFFECT,
             DEFAULT_WAVE_EFFECT,
             DEFAULT_ANGLE_EFFECT,
             DEFAULT_SQ_DIST_POWER,
             DEFAULT_FREQ_FACTOR,
             DEFAULT_AMPLITUDE,
             DEFAULT_PERIODIC_FACTOR,
             DEFAULT_REDUCER_COEFF,
             DEFAULT_SPIRAL_ROTATE_BASE_ANGLE,
             DEFAULT_USE_MODIFIED_ATAN_ANGLE,
             DEFAULT_MODIFIED_ATAN_ANGLE_FACTOR}
{
  m_randomViewport.SetProbNoViewport(PROB_NO_VIEWPORT);
}

auto Wave::SetRandomParams() noexcept -> void
{
  switch (m_mode)
  {
    case Modes::SQ_DIST_ANGLE_EFFECT_MODE0:
      SetSqDistAngleEffectMode0RandomParams();
      break;
    case Modes::SQ_DIST_ANGLE_EFFECT_MODE1:
      SetSqDistAngleEffectMode1RandomParams();
      break;
    case Modes::ATAN_ANGLE_EFFECT_MODE0:
      SetAtanAngleEffectMode0RandomParams();
      break;
    case Modes::ATAN_ANGLE_EFFECT_MODE1:
      SetAtanAngleEffectMode1RandomParams();
      break;
  }
}

auto Wave::SetSqDistAngleEffectMode0RandomParams() noexcept -> void
{
  SetWaveModeSettings({GetSqDistEffect(),
                       FREQ_FACTOR_RANGE,
                       AMPLITUDE_RANGE,
                       PERIODIC_FACTOR_RANGE,
                       SIN_COS_PERIODIC_FACTOR_RANGE});
}

auto Wave::SetSqDistAngleEffectMode1RandomParams() noexcept -> void
{
  if (m_goomRand->ProbabilityOf(PROB_ALLOW_STRANGE_WAVE_VALUES))
  {
    SetWaveModeSettings({GetSqDistEffect(),
                         SMALL_FREQ_FACTOR_RANGE,
                         BIG_AMPLITUDE_RANGE,
                         BIG_PERIODIC_FACTOR_RANGE,
                         BIG_SIN_COS_PERIODIC_FACTOR_RANGE});
  }
  else
  {
    SetWaveModeSettings({GetSqDistEffect(),
                         FREQ_FACTOR_RANGE,
                         AMPLITUDE_RANGE,
                         PERIODIC_FACTOR_RANGE,
                         SIN_COS_PERIODIC_FACTOR_RANGE});
  }
}

auto Wave::SetAtanAngleEffectMode0RandomParams() noexcept -> void
{
  SetWaveModeSettings({AngleEffect::ATAN,
                       FREQ_FACTOR_RANGE,
                       AMPLITUDE_RANGE,
                       PERIODIC_FACTOR_RANGE,
                       SIN_COS_PERIODIC_FACTOR_RANGE});
}

auto Wave::SetAtanAngleEffectMode1RandomParams() noexcept -> void
{
  if (m_goomRand->ProbabilityOf(PROB_ALLOW_STRANGE_WAVE_VALUES))
  {
    SetWaveModeSettings({AngleEffect::ATAN,
                         SMALL_FREQ_FACTOR_RANGE,
                         BIG_AMPLITUDE_RANGE,
                         BIG_PERIODIC_FACTOR_RANGE,
                         BIG_SIN_COS_PERIODIC_FACTOR_RANGE});
  }
  else
  {
    SetWaveModeSettings({AngleEffect::ATAN,
                         FREQ_FACTOR_RANGE,
                         AMPLITUDE_RANGE,
                         PERIODIC_FACTOR_RANGE,
                         SIN_COS_PERIODIC_FACTOR_RANGE});
  }
}

auto Wave::SetWaveModeSettings(const WaveModeSettings& waveModeSettings) noexcept -> void
{
  const auto viewport = m_randomViewport.GetRandomViewport();

  const auto waveEffectsEqual = m_goomRand->ProbabilityOf(PROB_WAVE_XY_EFFECTS_EQUAL);

  const auto xWaveEffect = m_weightedEffects.GetRandomWeighted();
  const auto yWaveEffect = waveEffectsEqual ? xWaveEffect : m_weightedEffects.GetRandomWeighted();

  const auto sqDistPower = m_goomRand->GetRandInRange(SQ_DIST_POWER_RANGE);

  const auto periodicFactor          = GetPeriodicFactor(xWaveEffect,
                                                yWaveEffect,
                                                waveModeSettings.periodicFactorRange,
                                                waveModeSettings.sinCosPeriodicFactorRange);
  const auto freqFactor              = m_goomRand->GetRandInRange(waveModeSettings.freqFactorRange);
  const auto amplitude               = m_goomRand->GetRandInRange(waveModeSettings.amplitudeRange);
  const auto reducerCoeff            = GetReducerCoeff(xWaveEffect, yWaveEffect, periodicFactor);
  const auto spiralRotateBaseAngle   = m_goomRand->GetRandInRange(0.0F, PI);
  const auto useModifiedATanAngle    = m_goomRand->ProbabilityOf(PROB_USE_MODIFIED_ATAN_ANGLE);
  const auto modifiedATanAngleFactor = m_goomRand->GetRandInRange(MODIFIED_ATAN_ANGLE_FACTOR_RANGE);

  SetParams({.viewport                = viewport,
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
             .modifiedATanAngleFactor = modifiedATanAngleFactor});
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
      return m_goomRand->GetRandInRange(REDUCER_COEFF_RANGE);
    case WAVE_TAN_EFFECT:
    case WAVE_COT_EFFECT:
      return m_goomRand->GetRandInRange(TAN_REDUCER_COEFF_RANGE);
    case WAVE_TAN_SIN_EFFECT:
    case WAVE_TAN_COS_EFFECT:
    case WAVE_COT_SIN_EFFECT:
    case WAVE_COT_COS_EFFECT:
      return std::lerp(m_goomRand->GetRandInRange(TAN_REDUCER_COEFF_RANGE),
                       m_goomRand->GetRandInRange(REDUCER_COEFF_RANGE),
                       periodicFactor);
  }
}

inline auto Wave::GetPeriodicFactor(
    const WaveEffect xWaveEffect,
    const WaveEffect yWaveEffect,
    const NumberRange<float>& periodicFactorRange,
    const NumberRange<float>& sinCosPeriodicFactorRange) const noexcept -> float
{
  if (m_goomRand->ProbabilityOf(PROB_NO_PERIODIC_FACTOR))
  {
    return xWaveEffect == WAVE_SIN_COS_EFFECT ? DEFAULT_SIN_COS_PERIODIC_FACTOR
                                              : DEFAULT_PERIODIC_FACTOR;
  }
  if (m_goomRand->ProbabilityOf(PROB_PERIODIC_FACTOR_USES_X_WAVE_EFFECT))
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
  return NameValuePairs();
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
#ifdef _MSC_VER
    default:
      FailFast();
#endif
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
      m_goomRand->GetRandInRange(SPIRAL_ROTATE_FACTOR_RANGE) * m_params.spiralRotateBaseAngle;
  const auto sinSpiralRotateAngle = std::sin(spiralRotateAngle);
  const auto cosSpiralRotateAngle = std::cos(spiralRotateAngle);
  const auto x = (coords.GetX() * cosSpiralRotateAngle) - (coords.GetY() * sinSpiralRotateAngle);
  const auto y = (coords.GetY() * cosSpiralRotateAngle) + (coords.GetX() * sinSpiralRotateAngle);

  const auto t     = (UTILS::MATH::PI + std::atan2(y, x)) / UTILS::MATH::TWO_PI;
  const auto angle = std::lerp(MIN_SPIRAL_ROTATE_LERP, MAX_SPIRAL_ROTATE_LERP, t);

  return std::pow(sqDistFromZero, m_params.sqDistPower) * angle;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
