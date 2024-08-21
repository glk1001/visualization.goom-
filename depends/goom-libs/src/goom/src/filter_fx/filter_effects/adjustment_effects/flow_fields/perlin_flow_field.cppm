module;

#include <PerlinNoise.hpp>
#include <cstdint>

export module Goom.FilterFx.FilterEffects.AdjustmentEffects.PerlinFlowField;

import Goom.FilterFx.FilterEffects.AdjustmentEffects.FlowFieldGrid;
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

class PerlinFlowField
{
public:
  explicit PerlinFlowField(const GoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void;

  [[nodiscard]] auto GetZoomAdjustment(const Vec2dFlt& baseZoomAdjustment,
                                       const NormalizedCoords& coords) const noexcept -> Vec2dFlt;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> UTILS::NameValuePairs;

  struct Params
  {
    Amplitude amplitude{};
    LerpToOneTs lerpToOneTs{};
    FrequencyFactor noiseFrequencyFactor{};
    FrequencyFactor angleFrequencyFactor{};
    float minAngle{};
    int32_t octaves1{};
    float persistence1{};
    int32_t octaves2{};
    float persistence2{};
    float noiseFactor{};
    bool multiplyVelocity = false;
  };
  [[nodiscard]] auto GetParams() const noexcept -> const Params&;

protected:
  auto SetParams(const Params& params) noexcept -> void;

private:
  const GoomRand* m_goomRand;
  FlowFieldGrid m_gridArray{};
  siv::BasicPerlinNoise<float> m_perlinNoise;
  siv::BasicPerlinNoise<float> m_perlinNoise2;
  auto SetupAngles() noexcept -> void;
  Params m_params;
  [[nodiscard]] auto GetRandomParams() const noexcept -> Params;
  [[nodiscard]] auto GetVelocity(const Vec2dFlt& baseZoomAdjustment,
                                 const NormalizedCoords& coords) const noexcept -> Vec2dFlt;
};

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

inline auto PerlinFlowField::GetZoomAdjustment(
    const Vec2dFlt& baseZoomAdjustment, const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto velocity = GetVelocity(baseZoomAdjustment, coords);

  return GetVelocityByZoomLerpedToOne(coords, m_params.lerpToOneTs, velocity);
}

inline auto PerlinFlowField::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline void PerlinFlowField::SetParams(const Params& params) noexcept
{
  m_params = params;
}

inline auto PerlinFlowField::SetRandomParams() noexcept -> void
{
  m_params = GetRandomParams();

  SetupAngles();
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
