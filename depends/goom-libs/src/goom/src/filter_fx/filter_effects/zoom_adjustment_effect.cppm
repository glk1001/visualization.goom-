export module Goom.FilterFx.FilterEffects.ZoomAdjustmentEffect;

import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Lib.Point2d;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
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
  static constexpr auto* PARAM_GROUP = "Filter Effects";
  [[nodiscard]] auto GetBaseZoomAdjustment() const noexcept -> const Vec2dFlt&;

private:
  Vec2dFlt m_baseZoomAdjustment{};
};

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

inline auto IZoomAdjustmentEffect::GetBaseZoomAdjustment() const noexcept -> const Vec2dFlt&
{
  return m_baseZoomAdjustment;
}

inline auto IZoomAdjustmentEffect::SetBaseZoomAdjustment(
    const Vec2dFlt& baseZoomAdjustment) noexcept -> void
{
  m_baseZoomAdjustment = baseZoomAdjustment;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
