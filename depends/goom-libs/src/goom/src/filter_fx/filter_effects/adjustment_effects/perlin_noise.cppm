module;

#include "goom/point2d.h"

#include <PerlinNoise.hpp>
#include <cstdint>

export module Goom.FilterFx.FilterEffects.AdjustmentEffects.PerlinNoise;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.FilterFx.ZoomAdjustmentEffect;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRandBase;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class PerlinNoise : public IZoomAdjustmentEffect
{
public:
  explicit PerlinNoise(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Vec2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> UTILS::NameValuePairs override;

  struct Params
  {
    Amplitude amplitude;
    FILTER_UTILS::LerpToOneTs lerpToOneTs;
    FrequencyFactor noiseFrequencyFactor;
    FrequencyFactor angleFrequencyFactor;
    int32_t octaves;
    float persistence;
  };
  [[nodiscard]] auto GetParams() const noexcept -> const Params&;

protected:
  auto SetParams(const Params& params) noexcept -> void;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  Params m_params;
  siv::BasicPerlinNoise<float> m_perlinNoise;
  siv::BasicPerlinNoise<float> m_perlinNoise2;
  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt;
  using PerlinSeedType = siv::BasicPerlinNoise<float>::seed_type;
  [[nodiscard]] static auto GetRandSeedForPerlinNoise() -> PerlinSeedType;
};

inline auto PerlinNoise::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline void PerlinNoise::SetParams(const Params& params) noexcept
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
