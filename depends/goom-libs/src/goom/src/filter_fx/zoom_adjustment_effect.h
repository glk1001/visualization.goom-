#pragma once

#include "filter_fx/normalized_coords.h"
#include "goom/point2d.h"

import Goom.Utils;

namespace GOOM::FILTER_FX
{

class IZoomAdjustmentEffect
{
public:
  IZoomAdjustmentEffect() noexcept                                       = default;
  IZoomAdjustmentEffect(const IZoomAdjustmentEffect&) noexcept           = default;
  IZoomAdjustmentEffect(IZoomAdjustmentEffect&&) noexcept                = default;
  virtual ~IZoomAdjustmentEffect() noexcept                              = default;
  auto operator=(const IZoomAdjustmentEffect&) -> IZoomAdjustmentEffect& = default;
  auto operator=(IZoomAdjustmentEffect&&) -> IZoomAdjustmentEffect&      = default;

  virtual auto SetRandomParams() noexcept -> void = 0;

  auto SetBaseZoomAdjustment(const Vec2dFlt& baseZoomAdjustment) noexcept -> void;

  [[nodiscard]] virtual auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Vec2dFlt = 0;

  [[nodiscard]] virtual auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> GOOM::UTILS::NameValuePairs = 0;

protected:
  static constexpr auto* PARAM_GROUP = "Zoom Adj";
  [[nodiscard]] auto GetBaseZoomAdjustment() const noexcept -> const Vec2dFlt&;

private:
  Vec2dFlt m_baseZoomAdjustment{};
};

inline auto IZoomAdjustmentEffect::GetBaseZoomAdjustment() const noexcept -> const Vec2dFlt&
{
  return m_baseZoomAdjustment;
}

inline auto IZoomAdjustmentEffect::SetBaseZoomAdjustment(
    const Vec2dFlt& baseZoomAdjustment) noexcept -> void
{
  m_baseZoomAdjustment = baseZoomAdjustment;
}

} // namespace GOOM::FILTER_FX
