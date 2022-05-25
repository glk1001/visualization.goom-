#pragma once

#include "normalized_coords.h"
#include "point2d.h"
#include "speed_coefficients_effect.h"
#include "utils/enumutils.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

namespace GOOM::VISUAL_FX::FILTERS
{

class YOnly : public ISpeedCoefficientsEffect
{
public:
  explicit YOnly(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  void SetRandomParams() override;

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
  void SetParams(const Params& params);

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

inline auto YOnly::GetYOnlySpeedMultiplier(const YOnlyEffect effect,
                                           const NormalizedCoords& coords) const -> float
{
  switch (effect)
  {
    case YOnlyEffect::X_SIN_Y_SIN:
      return std::sin(m_params.xFreqFactor * coords.GetX()) *
             std::sin(m_params.yFreqFactor * coords.GetY());
    case YOnlyEffect::X_SIN_Y_COS:
      return std::sin(m_params.xFreqFactor * coords.GetX()) *
             std::cos(m_params.yFreqFactor * coords.GetY());
    case YOnlyEffect::X_COS_Y_SIN:
      return std::cos(m_params.xFreqFactor * coords.GetX()) *
             std::sin(m_params.yFreqFactor * coords.GetY());
    case YOnlyEffect::X_COS_Y_COS:
      return std::cos(m_params.xFreqFactor * coords.GetX()) *
             std::cos(m_params.yFreqFactor * coords.GetY());
    default:
      throw std::logic_error(
          std20::format("Switch: unhandled case '{}'.", UTILS::EnumToString(effect)));
  }
}

inline auto YOnly::GetParams() const -> const Params&
{
  return m_params;
}

inline void YOnly::SetParams(const Params& params)
{
  m_params = params;
}

} // namespace GOOM::VISUAL_FX::FILTERS
