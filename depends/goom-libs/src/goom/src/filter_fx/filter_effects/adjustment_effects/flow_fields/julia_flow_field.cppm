module;

#include <complex>

export module Goom.FilterFx.FilterEffects.AdjustmentEffects.JuliaFlowField;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.Point2d;

using GOOM::FILTER_FX::FILTER_UTILS::LerpToOneTs;
using GOOM::UTILS::MATH::GoomRand;

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

  struct Params
  {
    Amplitude amplitude{};
    LerpToOneTs lerpToOneTs{};
    std::complex<float> c{}; // NOLINT(readability-identifier-length)
    int maxIterations{};
    float maxZ{};
    bool multiplyVelocity = false;
  };
  [[nodiscard]] auto GetParams() const noexcept -> const Params&;

protected:
  auto SetParams(const Params& params) noexcept -> void;

private:
  const GoomRand* m_goomRand;
  Params m_params;
  [[nodiscard]] auto GetVelocity(const Vec2dFlt& baseZoomAdjustment,
                                 const NormalizedCoords& coords) const noexcept -> Vec2dFlt;
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
