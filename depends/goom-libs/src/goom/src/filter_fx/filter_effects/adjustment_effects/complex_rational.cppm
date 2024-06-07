module;

#include <complex>
#include <cstdint>
#include <vector>

export module Goom.FilterFx.FilterEffects.AdjustmentEffects.ComplexRational;

import Goom.FilterFx.FilterEffects.AdjustmentEffects.ComplexUtils;
import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.FilterFx.ZoomAdjustmentEffect;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRandBase;
import Goom.Lib.Point2d;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
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
      std::vector<std::complex<FltCalcType>> zeroes;
      std::vector<std::complex<FltCalcType>> poles;
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

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

inline auto ComplexRational::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline void ComplexRational::SetParams(const Params& params) noexcept
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
