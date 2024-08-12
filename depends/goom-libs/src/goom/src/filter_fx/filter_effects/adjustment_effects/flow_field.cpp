module Goom.FilterFx.FilterEffects.AdjustmentEffects.FlowField;

import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::NameValuePairs;
using UTILS::MATH::GoomRand;

FlowField::FlowField(const GoomRand& goomRand) noexcept
: m_perlinFlowField{goomRand}, m_testFlowField{goomRand}
{
}

auto FlowField::GetZoomAdjustment(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
//  return m_perlinFlowField.GetZoomAdjustment(GetBaseZoomAdjustment(), coords);
  return m_testFlowField.GetZoomAdjustment(GetBaseZoomAdjustment(), coords);
}

auto FlowField::SetRandomParams() noexcept -> void
{
  //m_perlinFlowField.SetRandomParams();
  m_testFlowField.SetRandomParams();
}

auto FlowField::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  return {};
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
