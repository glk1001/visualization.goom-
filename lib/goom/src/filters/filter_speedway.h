#ifndef VISUALIZATION_GOOM_FILTER_SPEEDWAY_H
#define VISUALIZATION_GOOM_FILTER_SPEEDWAY_H

#include "filter_normalized_coords.h"
#include "filter_speed_coefficients_effect.h"
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

class Speedway : public SpeedCoefficientEffects
{
public:
  Speedway() noexcept;
  [[nodiscard]] auto GetSpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                          float sqDistFromZero,
                                          const NormalizedCoords& coords) const -> V2dFlt;

  void SetRandomParams();

  struct Params
  {
    float xAmplitude;
    float yAmplitude;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  void SetParams(const Params& params);

private:
  Params m_params;
};

inline auto Speedway::GetSpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                           const float sqDistFromZero,
                                           const NormalizedCoords& coords) const -> V2dFlt
{
  constexpr float SQ_DIST_FACTOR = 0.01F;
  const float yAdd = SQ_DIST_FACTOR * sqDistFromZero;

  const float xSpeedCoeff = baseSpeedCoeffs.x * (m_params.xAmplitude * (coords.GetY() + yAdd));
  const float ySpeedCoeff = m_params.yAmplitude * xSpeedCoeff;

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

#endif //VISUALIZATION_GOOM_FILTER_SPEEDWAY_H
