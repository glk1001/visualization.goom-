#pragma once

#include "filter_fx/normalized_coords.h"
#include "filter_fx/speed_coefficients_effect.h"
#include "point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class Wave : public ISpeedCoefficientsEffect
{
public:
  enum class Modes
  {
    MODE0,
    MODE1
  };
  Wave(Modes mode, const UTILS::MATH::IGoomRand& goomRand);

  auto SetRandomParams() -> void override;

  [[nodiscard]] auto GetSpeedCoefficients(const Point2dFlt& baseSpeedCoeffs,
                                          float sqDistFromZero,
                                          const NormalizedCoords& coords) const
      -> Point2dFlt override;

  [[nodiscard]] auto GetSpeedCoefficientsEffectNameValueParams() const
      -> UTILS::NameValuePairs override;

  enum class WaveEffect
  {
    WAVE_SIN_EFFECT,
    WAVE_COS_EFFECT,
    WAVE_SIN_COS_EFFECT,
    WAVE_TAN_EFFECT,
    WAVE_TAN_SIN_EFFECT,
    WAVE_TAN_COS_EFFECT,
    WAVE_COT_EFFECT,
    WAVE_COT_SIN_EFFECT,
    WAVE_COT_COS_EFFECT,
    _num // unused and must be last
  };
  struct Params
  {
    WaveEffect xWaveEffect;
    WaveEffect yWaveEffect;
    float freqFactor;
    float amplitude;
    float periodicFactor;
    float reducerCoeff;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  auto SetParams(const Params& params) -> void;

private:
  const Modes m_mode;
  const UTILS::MATH::IGoomRand& m_goomRand;
  const UTILS::MATH::Weights<WaveEffect> m_weightedEffects;
  Params m_params;
  auto SetMode0RandomParams() -> void;
  auto SetMode1RandomParams() -> void;
  auto SetWaveModeSettings(
      const UTILS::MATH::IGoomRand::NumberRange<float>& freqFactorRange,
      const UTILS::MATH::IGoomRand::NumberRange<float>& amplitudeRange,
      const UTILS::MATH::IGoomRand::NumberRange<float>& periodicFactorRange,
      const UTILS::MATH::IGoomRand::NumberRange<float>& sinCosPeriodicFactorRange) -> void;
  [[nodiscard]] auto GetSpeedAdd(float sqDistFromZero, WaveEffect waveEffect) const -> float;
  [[nodiscard]] static auto GetPeriodicPart(WaveEffect waveEffect,
                                            float angle,
                                            float periodicFactor) -> float;
  [[nodiscard]] auto GetPeriodicFactor(
      WaveEffect xWaveEffect,
      WaveEffect yWaveEffect,
      const UTILS::MATH::IGoomRand::NumberRange<float>& periodicFactorRange,
      const UTILS::MATH::IGoomRand::NumberRange<float>& sinCosPeriodicFactorRange) const -> float;
  [[nodiscard]] auto GetReducerCoeff(WaveEffect xWaveEffect,
                                     WaveEffect yWaveEffect,
                                     float periodicFactor) const -> float;
};

inline auto Wave::GetSpeedCoefficients(const Point2dFlt& baseSpeedCoeffs,
                                       const float sqDistFromZero,
                                       [[maybe_unused]] const NormalizedCoords& coords) const
    -> Point2dFlt
{
  const float xSpeedCoeff = baseSpeedCoeffs.x + GetSpeedAdd(sqDistFromZero, m_params.xWaveEffect);
  const float ySpeedCoeff = baseSpeedCoeffs.y + GetSpeedAdd(sqDistFromZero, m_params.yWaveEffect);

  return {xSpeedCoeff, ySpeedCoeff};
}

inline auto Wave::GetSpeedAdd(const float sqDistFromZero, const WaveEffect waveEffect) const
    -> float
{
  const float angle = m_params.freqFactor * sqDistFromZero;
  const float reducer = std::exp(-m_params.reducerCoeff * sqDistFromZero);
  return reducer * m_params.amplitude * GetPeriodicPart(waveEffect, angle, m_params.periodicFactor);
}

inline auto Wave::GetPeriodicPart(const WaveEffect waveEffect,
                                  const float angle,
                                  const float periodicFactor) -> float
{
  switch (waveEffect)
  {
    case WaveEffect::WAVE_SIN_EFFECT:
      return periodicFactor * std::sin(angle);
    case WaveEffect::WAVE_COS_EFFECT:
      return periodicFactor * std::cos(angle);
    case WaveEffect::WAVE_SIN_COS_EFFECT:
      return periodicFactor * STD20::lerp(std::sin(angle), std::cos(angle), periodicFactor);
    case WaveEffect::WAVE_TAN_EFFECT:
      return periodicFactor * std::tan(angle);
    case WaveEffect::WAVE_TAN_SIN_EFFECT:
      return periodicFactor * STD20::lerp(std::tan(angle), std::sin(angle), periodicFactor);
    case WaveEffect::WAVE_TAN_COS_EFFECT:
      return periodicFactor * STD20::lerp(std::tan(angle), std::cos(angle), periodicFactor);
    case WaveEffect::WAVE_COT_EFFECT:
      return periodicFactor * std::tan(UTILS::MATH::HALF_PI - angle);
    case WaveEffect::WAVE_COT_SIN_EFFECT:
      return periodicFactor *
             STD20::lerp(std::tan(UTILS::MATH::HALF_PI - angle), std::sin(angle), periodicFactor);
    case WaveEffect::WAVE_COT_COS_EFFECT:
      return periodicFactor *
             STD20::lerp(std::tan(UTILS::MATH::HALF_PI - angle), std::cos(angle), periodicFactor);
    default:
      throw std::logic_error("Unknown WaveEffect enum");
  }
}

inline auto Wave::GetParams() const -> const Params&
{
  return m_params;
}

inline auto Wave::SetParams(const Params& params) -> void
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
