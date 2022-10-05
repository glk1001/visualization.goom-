#pragma once

#include "filter_fx/normalized_coords.h"
#include "filter_fx/zoom_in_coefficients_effect.h"
#include "point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class Wave : public IZoomInCoefficientsEffect
{
public:
  enum class Modes
  {
    MODE0,
    MODE1
  };
  Wave(Modes mode, const UTILS::MATH::IGoomRand& goomRand);

  auto SetRandomParams() -> void override;

  [[nodiscard]] auto GetZoomInCoefficients(const NormalizedCoords& coords,
                                           float sqDistFromZero) const -> Point2dFlt override;

  [[nodiscard]] auto GetZoomInCoefficientsEffectNameValueParams() const
      -> UTILS::NameValuePairs override;

  enum class AngleEffect
  {
    ATAN,
    SQ_DIST
  };
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
    _num // unused, and marks the enum end
  };
  struct Params
  {
    WaveEffect xWaveEffect;
    WaveEffect yWaveEffect;
    AngleEffect angleEffect;
    float sqDistPower;
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
  [[nodiscard]] auto GetZoomInAdd(WaveEffect waveEffect, float angle, float reducer) const -> float;
  [[nodiscard]] auto GetAngle(float sqDistFromZero, const NormalizedCoords& coords) const -> float;
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

inline auto Wave::GetZoomInCoefficients(const NormalizedCoords& coords, float sqDistFromZero) const
    -> Point2dFlt
{
  const auto angle   = GetAngle(sqDistFromZero, coords);
  const auto reducer = std::exp(-m_params.reducerCoeff * sqDistFromZero);

  const auto xZoomInCoeff =
      GetBaseZoomInCoeffs().x + GetZoomInAdd(m_params.xWaveEffect, angle, reducer);
  const auto yZoomInCoeff =
      GetBaseZoomInCoeffs().y + GetZoomInAdd(m_params.yWaveEffect, angle, reducer);

  return {xZoomInCoeff, yZoomInCoeff};
}

inline auto Wave::GetAngle(const float sqDistFromZero, const NormalizedCoords& coords) const
    -> float
{
  if (m_params.angleEffect == AngleEffect::SQ_DIST)
  {
    return std::pow(sqDistFromZero, m_params.sqDistPower);
  }
  return std::atan2(coords.GetY(), coords.GetX());
}

inline auto Wave::GetZoomInAdd(const WaveEffect waveEffect,
                               const float angle,
                               const float reducer) const -> float
{
  return reducer * m_params.amplitude *
         GetPeriodicPart(waveEffect, m_params.freqFactor * angle, m_params.periodicFactor);
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
