export module Goom.FilterFx.FilterEffects.AdjustmentEffects.UniformZoomAdjustmentEffect;

import Goom.FilterFx.NormalizedCoords;
import Goom.FilterFx.ZoomAdjustmentEffect;
import Goom.Utils.NameValuePairs;
import Goom.Lib.Point2d;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class UniformZoomAdjustmentEffect : public IZoomAdjustmentEffect
{
public:
  UniformZoomAdjustmentEffect() noexcept = default;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Vec2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> GOOM::UTILS::NameValuePairs override;

private:
  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt;
};

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

inline auto UniformZoomAdjustmentEffect::GetZoomAdjustment(
    const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto velocity = GetVelocity(coords);

  return {.x = coords.GetX() * velocity.x, .y = coords.GetY() * velocity.y};
}

inline auto UniformZoomAdjustmentEffect::SetRandomParams() noexcept -> void
{
  // do nothing
}

inline auto UniformZoomAdjustmentEffect::GetZoomAdjustmentEffectNameValueParams() const noexcept
    -> GOOM::UTILS::NameValuePairs
{
  return {};
}

inline auto UniformZoomAdjustmentEffect::GetVelocity(
    [[maybe_unused]] const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  return GetBaseZoomAdjustment();
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
