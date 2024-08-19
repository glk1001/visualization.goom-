module;

#include <complex>
#include <cstdint>
#include <functional>

export module Goom.FilterFx.FilterEffects.AdjustmentEffects.JuliaFlowField;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.EnumUtils;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

using GOOM::FILTER_FX::FILTER_UTILS::LerpToOneTs;
using GOOM::UTILS::EnumMap;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::Weights;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class JuliaFlowField
{
public:
  explicit JuliaFlowField(const GoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void;

  [[nodiscard]] auto GetZoomAdjustment(const Vec2dFlt& baseZoomAdjustment,
                                       const NormalizedCoords& coords) const noexcept -> Vec2dFlt;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> UTILS::NameValuePairs;

  enum class ZFuncTypes : UnderlyingEnumType
  {
    STD_JULIA_FUNC,
    CUBIC_JULIA_FUNC1,
    CUBIC_JULIA_FUNC2,
    SIN_JULIA_FUNC1,
    SIN_JULIA_FUNC2,
    COS_JULIA_FUNC1,
    COS_JULIA_FUNC2,
  };

  using ZFunc = std::function<std::complex<float>(const std::complex<float>& z,
                                                  const std::complex<float>& c)>;
  struct Params
  {
    Viewport viewport;
    Amplitude amplitude{};
    LerpToOneTs lerpToOneTs{};
    std::complex<float> c; // NOLINT(readability-identifier-length)
    uint32_t maxIterations{};
    bool multiplyVelocity = false;
    ZFunc zFunc{};
  };
  [[nodiscard]] auto GetParams() const noexcept -> const Params&;

protected:
  auto SetParams(const Params& params) noexcept -> void;

private:
  const GoomRand* m_goomRand;
  FILTER_UTILS::RandomViewport m_randomViewport;
  Params m_params;
  Weights<ZFuncTypes> m_zFuncWeights;
  EnumMap<ZFuncTypes, ZFunc> m_zFuncs;
  [[nodiscard]] auto GetVelocity(const Vec2dFlt& baseZoomAdjustment,
                                 const NormalizedCoords& coords) const noexcept -> Vec2dFlt;
  [[nodiscard]] auto GetJuliaPoint(const std::complex<float>& trapPoint,
                                   const std::complex<float>& c,
                                   const std::complex<float>& z0) const noexcept
      -> std::complex<float>;
};

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

inline auto JuliaFlowField::GetZoomAdjustment(const Vec2dFlt& baseZoomAdjustment,
                                              const NormalizedCoords& coords) const noexcept
    -> Vec2dFlt
{
  const auto velocity = GetVelocity(baseZoomAdjustment, coords);

  return GetVelocityByZoomLerpedToOne(coords, m_params.lerpToOneTs, velocity);
}

inline auto JuliaFlowField::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline void JuliaFlowField::SetParams(const Params& params) noexcept
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
