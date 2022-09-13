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
  Wave(Modes mode, const GOOM::UTILS::MATH::IGoomRand& goomRand);

  auto SetRandomParams() -> void override;

  [[nodiscard]] auto GetZoomInCoefficients(const NormalizedCoords& coords,
                                           float sqDistFromZero,
                                           const Point2dFlt& baseZoomInCoeffs) const
      -> Point2dFlt override;

  [[nodiscard]] auto GetZoomInCoefficientsEffectNameValueParams() const
      -> GOOM::UTILS::NameValuePairs override;

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
  const GOOM::UTILS::MATH::IGoomRand& m_goomRand;
  const GOOM::UTILS::MATH::Weights<WaveEffect> m_weightedEffects;
  Params m_params;
  auto SetMode0RandomParams() -> void;
  auto SetMode1RandomParams() -> void;
  auto SetWaveModeSettings(
      const GOOM::UTILS::MATH::IGoomRand::NumberRange<float>& freqFactorRange,
      const GOOM::UTILS::MATH::IGoomRand::NumberRange<float>& amplitudeRange,
      const GOOM::UTILS::MATH::IGoomRand::NumberRange<float>& periodicFactorRange,
      const GOOM::UTILS::MATH::IGoomRand::NumberRange<float>& sinCosPeriodicFactorRange) -> void;
  [[nodiscard]] auto GetZoomInAdd(float sqDistFromZero, WaveEffect waveEffect) const -> float;
  [[nodiscard]] static auto GetPeriodicPart(WaveEffect waveEffect,
                                            float angle,
                                            float periodicFactor) -> float;
  [[nodiscard]] auto GetPeriodicFactor(
      WaveEffect xWaveEffect,
      WaveEffect yWaveEffect,
      const GOOM::UTILS::MATH::IGoomRand::NumberRange<float>& periodicFactorRange,
      const GOOM::UTILS::MATH::IGoomRand::NumberRange<float>& sinCosPeriodicFactorRange) const
      -> float;
  [[nodiscard]] auto GetReducerCoeff(WaveEffect xWaveEffect,
                                     WaveEffect yWaveEffect,
                                     float periodicFactor) const -> float;
};

inline auto Wave::GetZoomInCoefficients([[maybe_unused]] const NormalizedCoords& coords,
                                        float sqDistFromZero,
                                        const Point2dFlt& baseZoomInCoeffs) const -> Point2dFlt
{
  const auto xZoomInCoeff = baseZoomInCoeffs.x + GetZoomInAdd(sqDistFromZero, m_params.xWaveEffect);
  const auto yZoomInCoeff = baseZoomInCoeffs.y + GetZoomInAdd(sqDistFromZero, m_params.yWaveEffect);

  return {xZoomInCoeff, yZoomInCoeff};
}

inline auto Wave::GetZoomInAdd(const float sqDistFromZero, const WaveEffect waveEffect) const
    -> float
{
  const auto angle   = m_params.freqFactor * sqDistFromZero;
  const auto reducer = std::exp(-m_params.reducerCoeff * sqDistFromZero);
  return reducer * m_params.amplitude * GetPeriodicPart(waveEffect, angle, m_params.periodicFactor);
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
