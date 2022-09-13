#pragma once

#include "filter_fx/normalized_coords.h"
#include "filter_fx/zoom_in_coefficients_effect.h"
#include "point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class Amulet : public IZoomInCoefficientsEffect
{
public:
  explicit Amulet(const GOOM::UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto SetRandomParams() -> void override;

  [[nodiscard]] auto GetZoomInCoefficients(const NormalizedCoords& coords,
                                           float sqDistFromZero,
                                           const Point2dFlt& baseZoomInCoeffs) const
      -> Point2dFlt override;

  [[nodiscard]] auto GetZoomInCoefficientsEffectNameValueParams() const
      -> GOOM::UTILS::NameValuePairs override;

  struct Params
  {
    float xAmplitude;
    float yAmplitude;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  auto SetParams(const Params& params) -> void;

private:
  const GOOM::UTILS::MATH::IGoomRand& m_goomRand;
  Params m_params;
};

inline auto Amulet::GetZoomInCoefficients([[maybe_unused]] const NormalizedCoords& coords,
                                          float sqDistFromZero,
                                          const Point2dFlt& baseZoomInCoeffs) const -> Point2dFlt
{
  return {baseZoomInCoeffs.x + (m_params.xAmplitude * sqDistFromZero),
          baseZoomInCoeffs.y + (m_params.yAmplitude * sqDistFromZero)};
  //?      speedCoeffs.y = 5.0F * std::cos(5.0F * speedCoeffs.x) * std::sin(5.0F * speedCoeffs.y);
}

inline auto Amulet::GetParams() const -> const Params&
{
  return m_params;
}

inline void Amulet::SetParams(const Params& params)
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
