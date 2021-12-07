#pragma once

#include "normalized_coords.h"
#include "speed_coefficients_effect.h"
#include "utils/goom_rand_base.h"
#include "utils/name_value_pairs.h"
#include "v2d.h"

namespace GOOM::VISUAL_FX::FILTERS
{

class YOnly : public ISpeedCoefficientsEffect
{
public:
  explicit YOnly(const UTILS::IGoomRand& goomRand) noexcept;

  void SetRandomParams() override;

  [[nodiscard]] auto GetSpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                          float sqDistFromZero,
                                          const NormalizedCoords& coords) const -> V2dFlt override;

  [[nodiscard]] auto GetSpeedCoefficientsEffectNameValueParams() const
      -> UTILS::NameValuePairs override;

  enum class YOnlyEffect
  {
    NONE,
    XSIN_YSIN,
    XSIN_YCOS,
    XCOS_YSIN,
    XCOS_YCOS,
    _NUM // unused and must be last
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
  const UTILS::IGoomRand& m_goomRand;
  Params m_params;
  [[nodiscard]] auto GetYOnlySpeedMultiplier(YOnlyEffect effect,
                                             const NormalizedCoords& coords) const -> float;
};

inline auto YOnly::GetSpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                        [[maybe_unused]] const float sqDistFromZero,
                                        const NormalizedCoords& coords) const -> V2dFlt
{
  const float xSpeedCoeff = baseSpeedCoeffs.x * m_params.xAmplitude *
                            GetYOnlySpeedMultiplier(m_params.xEffect, coords);
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
    case YOnlyEffect::XSIN_YSIN:
      return std::sin(m_params.xFreqFactor * coords.GetX()) *
             std::sin(m_params.yFreqFactor * coords.GetY());
    case YOnlyEffect::XSIN_YCOS:
      return std::sin(m_params.xFreqFactor * coords.GetX()) *
             std::cos(m_params.yFreqFactor * coords.GetY());
    case YOnlyEffect::XCOS_YSIN:
      return std::cos(m_params.xFreqFactor * coords.GetX()) *
             std::sin(m_params.yFreqFactor * coords.GetY());
    case YOnlyEffect::XCOS_YCOS:
      return std::cos(m_params.xFreqFactor * coords.GetX()) *
             std::cos(m_params.yFreqFactor * coords.GetY());
    default:
      throw std::logic_error(std20::format("Switch: unhandled case '{}'.", effect));
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
