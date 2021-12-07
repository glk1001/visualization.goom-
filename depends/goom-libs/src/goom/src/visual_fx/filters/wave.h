#pragma once

#include "normalized_coords.h"
#include "speed_coefficients_effect.h"
#include "utils/goom_rand_base.h"
#include "utils/name_value_pairs.h"
#include "v2d.h"

namespace GOOM::VISUAL_FX::FILTERS
{

class Wave : public ISpeedCoefficientsEffect
{
public:
  enum class Modes
  {
    MODE0,
    MODE1
  };
  Wave(Modes mode, const UTILS::IGoomRand& goomRand) noexcept;

  void SetRandomParams() override;

  [[nodiscard]] auto GetSpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                          float sqDistFromZero,
                                          const NormalizedCoords& coords) const -> V2dFlt override;

  [[nodiscard]] auto GetSpeedCoefficientsEffectNameValueParams() const
      -> UTILS::NameValuePairs override;

  enum class WaveEffect
  {
    WAVE_SIN_EFFECT,
    WAVE_COS_EFFECT,
    WAVE_SIN_COS_EFFECT,
    _NUM // unused and must be last
  };
  struct Params
  {
    WaveEffect xWaveEffect;
    WaveEffect yWaveEffect;
    float freqFactor;
    float amplitude;
    float periodicFactor;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  void SetParams(const Params& params);

private:
  const Modes m_mode;
  const UTILS::IGoomRand& m_goomRand;
  Params m_params;
  void SetMode0RandomParams();
  void SetMode1RandomParams();
  void SetWaveModeSettings(const UTILS::IGoomRand::NumberRange<float>& freqFactorRange,
                           const UTILS::IGoomRand::NumberRange<float>& amplitudeRange);
  [[nodiscard]] auto GetSpeedAdd(WaveEffect waveEffect, float angle) const -> float;
  [[nodiscard]] static auto GetPeriodicPart(WaveEffect waveEffect,
                                            float angle,
                                            float periodicFactor) -> float;
};

inline auto Wave::GetSpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                       const float sqDistFromZero,
                                       [[maybe_unused]] const NormalizedCoords& coords) const
    -> V2dFlt
{
  const float angle = m_params.freqFactor * sqDistFromZero;
  const float xSpeedCoeff = baseSpeedCoeffs.x + GetSpeedAdd(m_params.xWaveEffect, angle);

  if (m_params.xWaveEffect == m_params.yWaveEffect)
  {
    return {xSpeedCoeff, xSpeedCoeff};
  }

  return {xSpeedCoeff, baseSpeedCoeffs.y + GetSpeedAdd(m_params.yWaveEffect, angle)};
}

inline auto Wave::GetSpeedAdd(const WaveEffect waveEffect, const float angle) const -> float
{
  return m_params.amplitude * GetPeriodicPart(waveEffect, angle, m_params.periodicFactor);
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
      return stdnew::lerp(std::sin(angle), std::cos(angle), periodicFactor);
    default:
      throw std::logic_error("Unknown WaveEffect enum");
  }
}

inline auto Wave::GetParams() const -> const Params&
{
  return m_params;
}

inline void Wave::SetParams(const Params& params)
{
  m_params = params;
}

} // namespace GOOM::VISUAL_FX::FILTERS
