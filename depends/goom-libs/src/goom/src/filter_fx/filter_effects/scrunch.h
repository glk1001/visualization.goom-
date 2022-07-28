#pragma once

#include "filter_fx/normalized_coords.h"
#include "filter_fx/speed_coefficients_effect.h"
#include "point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class Scrunch : public ISpeedCoefficientsEffect
{
public:
  explicit Scrunch(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto SetRandomParams() -> void override;

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
  auto SetParams(const Params& params) -> void;

private:
  const UTILS::MATH::IGoomRand& m_goomRand;
  Params m_params;
};

inline auto Scrunch::GetSpeedCoefficients(const Point2dFlt& baseSpeedCoeffs,
                                          const float sqDistFromZero,
                                          [[maybe_unused]] const NormalizedCoords& coords) const
    -> Point2dFlt
{
  const auto xSpeedCoeff = baseSpeedCoeffs.x + (m_params.xAmplitude * sqDistFromZero);
  const auto ySpeedCoeff = m_params.yAmplitude * xSpeedCoeff;
  return {xSpeedCoeff, ySpeedCoeff};
}

inline auto Scrunch::GetParams() const -> const Params&
{
  return m_params;
}

inline auto Scrunch::SetParams(const Params& params) -> void
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
