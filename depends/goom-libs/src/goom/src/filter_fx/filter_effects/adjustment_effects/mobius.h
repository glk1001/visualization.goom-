#pragma once

#include "filter_fx/common_types.h"
#include "filter_fx/filter_utils/utils.h"
#include "filter_fx/normalized_coords.h"
#include "filter_fx/zoom_adjustment_effect.h"
#include "goom/point2d.h"
#include "utils/math/goom_rand_base.h"

import Goom.Utils;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class Mobius : public IZoomAdjustmentEffect
{
public:
  explicit Mobius(const UTILS::MATH::IGoomRand& goomRand) noexcept;

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
    float a;
    float b;
    float c;
    float d;
    bool noInverseSquare;
    bool useNormalizedAmplitude;
    bool useModulatorContours;
    float modulatorPeriod;
  };
  [[nodiscard]] auto GetParams() const noexcept -> const Params&;

protected:
  auto SetParams(const Params& params) noexcept -> void;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  FILTER_UTILS::RandomViewport m_randomViewport;
  Params m_params;
  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt;
};

inline auto Mobius::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline void Mobius::SetParams(const Params& params) noexcept
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
