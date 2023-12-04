#pragma once

#include "complex_utils.h"
#include "filter_fx/common_types.h"
#include "filter_fx/filter_utils/utils.h"
#include "filter_fx/normalized_coords.h"
#include "filter_fx/zoom_adjustment_effect.h"
#include "goom/point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

#include <complex>
#include <cstdint>
#include <vector>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class ComplexRational : public IZoomAdjustmentEffect
{
public:
  explicit ComplexRational(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Vec2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> UTILS::NameValuePairs override;

  struct Params
  {
    Viewport viewport;
    Amplitude amplitude;
    FILTER_UTILS::LerpToOneTs lerpToOneTs;
    bool noInverseSquare;
    bool useNormalizedAmplitude;
    bool useModulatorContours;
    float modulatorPeriod;
    struct ZeroesAndPoles
    {
      std::vector<std::complex<FltCalcType>> zeroes{};
      std::vector<std::complex<FltCalcType>> poles{};
    };
    ZeroesAndPoles zeroesAndPoles{};
  };
  [[nodiscard]] auto GetParams() const noexcept -> const Params&;

protected:
  auto SetParams(const Params& params) noexcept -> void;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  FILTER_UTILS::RandomViewport m_randomViewport;
  Params m_params;
  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt;
  [[nodiscard]] auto GetPolyValue(const std::complex<FltCalcType>& z) const noexcept
      -> std::complex<FltCalcType>;
  [[nodiscard]] static auto GetProduct(
      const std::complex<FltCalcType>& z,
      const std::vector<std::complex<FltCalcType>>& coeffs) noexcept -> std::complex<FltCalcType>;
  [[nodiscard]] auto GetNextZeroesAndPoles() const noexcept -> Params::ZeroesAndPoles;
  [[nodiscard]] static auto GetPointSpread(uint32_t numPoints, float radius) noexcept
      -> std::vector<std::complex<FltCalcType>>;
  [[nodiscard]] static auto GetSimpleZeroesAndPoles() noexcept -> Params::ZeroesAndPoles;
};

inline auto ComplexRational::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline void ComplexRational::SetParams(const Params& params) noexcept
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
