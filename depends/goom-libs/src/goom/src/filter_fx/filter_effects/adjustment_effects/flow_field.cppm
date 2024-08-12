export module Goom.FilterFx.FilterEffects.AdjustmentEffects.FlowField;

import Goom.FilterFx.FilterEffects.AdjustmentEffects.PerlinFlowField;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.TestFlowField;
import Goom.FilterFx.NormalizedCoords;
import Goom.FilterFx.ZoomAdjustmentEffect;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.Point2d;

using GOOM::UTILS::NameValuePairs;
using GOOM::UTILS::MATH::GoomRand;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class FlowField : public IZoomAdjustmentEffect
{
public:
  explicit FlowField(const GoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Vec2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> NameValuePairs override;

private:
  PerlinFlowField m_perlinFlowField;
  TestFlowField m_testFlowField;
};

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
