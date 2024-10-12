module;

#include <PerlinNoise.hpp>
#include <cstdint>

export module Goom.FilterFx.FilterEffects.AdjustmentEffects.PerlinNoise;

import Goom.FilterFx.FilterEffects.ZoomAdjustmentEffect;
import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.Point2d;

using GOOM::UTILS::NameValuePairs;
using GOOM::UTILS::MATH::GOOM_RAND_MAX;
using GOOM::UTILS::MATH::GoomRand;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class PerlinNoise : public IZoomAdjustmentEffect
{
public:
  explicit PerlinNoise(const GoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Vec2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> NameValuePairs override;

  struct Params
  {
    Amplitude amplitude;
    FrequencyFactor noiseFrequencyFactor;
    FrequencyFactor angleFrequencyFactor;
    FILTER_UTILS::LerpToOneTs lerpToOneTs;
    int32_t octaves;
    float persistence;
  };
  [[nodiscard]] auto GetParams() const noexcept -> const Params&;

protected:
  auto SetParams(const Params& params) noexcept -> void;

private:
  const GoomRand* m_goomRand;
  Params m_params;
  [[nodiscard]] auto GetRandomParams() const noexcept -> Params;
  siv::BasicPerlinNoise<float> m_perlinNoise;
  siv::BasicPerlinNoise<float> m_perlinNoise2;
  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt;
  using PerlinSeedType = siv::BasicPerlinNoise<float>::seed_type;
  [[nodiscard]] static auto GetRandSeedForPerlinNoise() -> PerlinSeedType;
};

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

inline auto PerlinNoise::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline void PerlinNoise::SetParams(const Params& params) noexcept
{
  m_params = params;
}

inline auto PerlinNoise::SetRandomParams() noexcept -> void
{
  m_perlinNoise.reseed(m_goomRand->GetNRand(GOOM_RAND_MAX));
  m_perlinNoise2.reseed(m_goomRand->GetNRand(GOOM_RAND_MAX));

  m_params = GetRandomParams();
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
