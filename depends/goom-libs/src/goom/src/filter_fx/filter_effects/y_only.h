#pragma once

#include "filter_fx/normalized_coords.h"
#include "filter_fx/speed_coefficients_effect.h"
#include "point2d.h"
#include "utils/enumutils.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class YOnly : public ISpeedCoefficientsEffect
{
public:
  explicit YOnly(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto SetRandomParams() -> void override;

  [[nodiscard]] auto GetSpeedCoefficients(const Point2dFlt& baseSpeedCoeffs,
                                          float sqDistFromZero,
                                          const NormalizedCoords& coords) const
      -> Point2dFlt override;

  [[nodiscard]] auto GetSpeedCoefficientsEffectNameValueParams() const
      -> UTILS::NameValuePairs override;

  enum class YOnlyEffect
  {
    NONE,
    X_SIN_Y_SIN,
    X_SIN_Y_COS,
    X_COS_Y_SIN,
    X_COS_Y_COS,
    _num // unused and must be last
  };
  struct Params
  {
    YOnlyEffect xEffect;
    YOnlyEffect yEffect;
    float xFreqFactor;
    float yFreqFactor;
    float xAmplitude;
    float yAmplitude;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  auto SetParams(const Params& params) -> void;

private:
  const UTILS::MATH::IGoomRand& m_goomRand;
  Params m_params;
  [[nodiscard]] auto GetYOnlySpeedMultiplier(YOnlyEffect effect,
                                             const NormalizedCoords& coords) const -> float;
};

inline auto YOnly::GetSpeedCoefficients(const Point2dFlt& baseSpeedCoeffs,
                                        [[maybe_unused]] const float sqDistFromZero,
                                        const NormalizedCoords& coords) const -> Point2dFlt
{
  const float xSpeedCoeff =
      baseSpeedCoeffs.x * m_params.xAmplitude * GetYOnlySpeedMultiplier(m_params.xEffect, coords);
  if (m_params.yEffect == YOnlyEffect::NONE)
  {
    return {xSpeedCoeff, xSpeedCoeff};
  }

  return {xSpeedCoeff, baseSpeedCoeffs.y * m_params.yAmplitude *
                           GetYOnlySpeedMultiplier(m_params.yEffect, coords)};
}

inline auto YOnly::GetParams() const -> const Params&
{
  return m_params;
}

inline auto YOnly::SetParams(const Params& params) -> void
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS