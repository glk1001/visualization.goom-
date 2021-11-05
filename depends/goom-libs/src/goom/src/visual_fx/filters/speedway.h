#pragma once

#include "normalized_coords.h"
#include "speed_coefficients_effect.h"
#include "utils/goom_rand_base.h"
#include "utils/name_value_pairs.h"
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

class Speedway : public ISpeedCoefficientsEffect
{
public:
  enum class Modes
  {
    MODE0,
    MODE1,
  };
  Speedway(Modes mode, UTILS::IGoomRand& goomRand) noexcept;

  void SetRandomParams() override;

  [[nodiscard]] auto GetSpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                          float sqDistFromZero,
                                          const NormalizedCoords& coords) const -> V2dFlt override;

  [[nodiscard]] auto GetSpeedCoefficientsEffectNameValueParams() const
      -> UTILS::NameValuePairs override;

  struct Params
  {
    float xAmplitude;
    float yAmplitude;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  void SetParams(const Params& params);

private:
  const Modes m_mode;
  UTILS::IGoomRand& m_goomRand;
  Params m_params;
  void SetMode0RandomParams();
  void SetMode1RandomParams();
  [[nodiscard]] auto GetMode0SpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                               float sqDistFromZero,
                                               const NormalizedCoords& coords) const -> V2dFlt;
  [[nodiscard]] auto GetMode1SpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                               float sqDistFromZero,
                                               const NormalizedCoords& coords) const -> V2dFlt;
};

inline auto Speedway::GetSpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                           const float sqDistFromZero,
                                           const NormalizedCoords& coords) const -> V2dFlt
{
  if (m_mode == Modes::MODE0)
  {
    return GetMode0SpeedCoefficients(baseSpeedCoeffs, sqDistFromZero, coords);
  }

  return GetMode1SpeedCoefficients(baseSpeedCoeffs, sqDistFromZero, coords);
}

inline auto Speedway::GetMode0SpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                                const float sqDistFromZero,
                                                const NormalizedCoords& coords) const -> V2dFlt
{
  constexpr float SQ_DIST_FACTOR = 0.01F;
  const float yAdd = SQ_DIST_FACTOR * sqDistFromZero;

  const float xSpeedCoeff = baseSpeedCoeffs.x * (m_params.xAmplitude * (coords.GetY() + yAdd));
  const float ySpeedCoeff = m_params.yAmplitude * xSpeedCoeff;

  return {xSpeedCoeff, ySpeedCoeff};
}

inline auto Speedway::GetMode1SpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                                const float sqDistFromZero,
                                                const NormalizedCoords& coords) const -> V2dFlt
{
  float xAdd = -1.0F;

  if (m_goomRand.ProbabilityOf(0.0F))
  {
    xAdd = m_goomRand.ProbabilityOf(0.5F) ? m_goomRand.GetRandInRange(-1.9F, -0.5F)
                                          : m_goomRand.GetRandInRange(+0.5F, +1.9F);
  }
  else if (m_goomRand.ProbabilityOf(0.0F))
  {
    xAdd = +1.0F;
  }

  const float xDiff = coords.GetX() - xAdd;
  const float sign = xDiff < 0.0F ? -1.0F : +1.0F;
  const float xWarp = 0.1F * ((sign * UTILS::Sq(xDiff) / xAdd) + xAdd);

  const float xSpeedCoeff =
      (1.0F - sqDistFromZero / 4.0F) * baseSpeedCoeffs.x * (m_params.xAmplitude * xWarp);
  const float ySpeedCoeff = (1.0F - sqDistFromZero / 4.0F) * m_params.yAmplitude * xSpeedCoeff;

  return {xSpeedCoeff, ySpeedCoeff};
}

inline auto Speedway::GetParams() const -> const Params&
{
  return m_params;
}

inline void Speedway::SetParams(const Params& params)
{
  m_params = params;
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif

