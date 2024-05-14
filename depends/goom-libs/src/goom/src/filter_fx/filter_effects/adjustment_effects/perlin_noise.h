#pragma once

#include "filter_fx/common_types.h"
#include "filter_fx/filter_utils/utils.h"
#include "filter_fx/normalized_coords.h"
#include "filter_fx/zoom_adjustment_effect.h"
#include "goom/point2d.h"

#include <PerlinNoise.hpp>
#include <cstdint>

import Goom.Utils.Math.GoomRandBase;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
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
