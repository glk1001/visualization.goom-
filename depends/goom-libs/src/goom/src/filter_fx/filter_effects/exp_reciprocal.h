#pragma once

#include "filter_fx/common_types.h"
#include "filter_fx/normalized_coords.h"
#include "filter_fx/zoom_in_coefficients_effect.h"
#include "point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

#include <complex>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class ExpReciprocal : public IZoomInCoefficientsEffect
{
public:
  explicit ExpReciprocal(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto SetRandomParams() -> void override;

  [[nodiscard]] auto GetZoomInCoefficients(const NormalizedCoords& coords,
                                           float sqDistFromZero) const -> Point2dFlt override;

  [[nodiscard]] auto GetZoomInCoefficientsEffectNameValueParams() const
      -> UTILS::NameValuePairs override;

  struct Params
  {
    Amplitude amplitude;
    std::complex<float> rotate;
    float magnify;
    float reciprocalExponent;
    bool useModulusContours;
    bool usePhaseContours;
    float sawtoothModulusPeriod;
    float sawtoothPhasePeriod;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  auto SetParams(const Params& params) -> void;

private:
  const UTILS::MATH::IGoomRand& m_goomRand;
  Params m_params;
};

inline auto ExpReciprocal::GetParams() const -> const Params&
{
  return m_params;
}

inline void ExpReciprocal::SetParams(const Params& params)
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
