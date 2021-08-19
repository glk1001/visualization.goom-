#ifndef VISUALIZATION_GOOM_FILTER_WAVE_H
#define VISUALIZATION_GOOM_FILTER_WAVE_H

#include "v2d.h"

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

class Wave
{
public:
  Wave() noexcept;
  [[nodiscard]] auto GetSpeedCoefficients(const V2dFlt& baseSpeedCoeffs, float sqDistFromZero) const
      -> V2dFlt;

  void SetMode0RandomParams();
  void SetMode1RandomParams();

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
  Params m_params;
  void SetWaveModeSettings(const UTILS::NumberRange<float>& freqFactorRange,
                           const UTILS::NumberRange<float>& amplitudeRange);
  [[nodiscard]] auto GetSpeedAdd(WaveEffect waveEffect, float angle) const -> float;
  [[nodiscard]] static auto GetPeriodicPart(WaveEffect waveEffect,
                                            float angle,
                                            float periodicFactor) -> float;
};

inline auto Wave::GetSpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                       const float sqDistFromZero) const -> V2dFlt
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

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif

#endif //VISUALIZATION_GOOM_FILTER_WAVE_H
