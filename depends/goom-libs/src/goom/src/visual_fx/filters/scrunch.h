#pragma once

#include "normalized_coords.h"
#include "point2d.h"
#include "speed_coefficients_effect.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

namespace GOOM::VISUAL_FX::FILTERS
{

class Scrunch : public ISpeedCoefficientsEffect
{
public:
  explicit Scrunch(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  void SetRandomParams() override;

  [[nodiscard]] auto GetSpeedCoefficients(const Point2dFlt& baseSpeedCoeffs,
                                          float sqDistFromZero,
                                          const NormalizedCoords& coords) const
      -> Point2dFlt override;

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
  const UTILS::MATH::IGoomRand& m_goomRand;
  Params m_params;
};

inline auto Scrunch::GetSpeedCoefficients(const Point2dFlt& baseSpeedCoeffs,
                                          const float sqDistFromZero,
                                          [[maybe_unused]] const NormalizedCoords& coords) const
    -> Point2dFlt
{
  const float xSpeedCoeff = baseSpeedCoeffs.x + (m_params.xAmplitude * sqDistFromZero);
  const float ySpeedCoeff = m_params.yAmplitude * xSpeedCoeff;
  return {xSpeedCoeff, ySpeedCoeff};
}

inline auto Scrunch::GetParams() const -> const Params&
{
  return m_params;
}

inline void Scrunch::SetParams(const Params& params)
{
  m_params = params;
}

} // namespace GOOM::VISUAL_FX::FILTERS
