module Goom.FilterFx.FilterEffects.AdjustmentEffects.FlowField;

import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::NameValuePairs;
using UTILS::MATH::GoomRand;

static constexpr auto DIPOLE_WEIGHT = 1.0F;
static constexpr auto JULIA_WEIGHT  = 1.0F;
static constexpr auto PERLIN_WEIGHT = 1.0F;
static constexpr auto TEST_WEIGHT   = 0.01F;

FlowField::FlowField(const GoomRand& goomRand) noexcept
  : m_dipoleFlowField{goomRand},
    m_juliaFlowField{goomRand},
    m_perlinFlowField{goomRand},
    m_testFlowField{goomRand},
    m_flowFieldTypeWeights{
        goomRand,
  {
             {.key = FlowFieldTypes::DIPOLE, .weight = DIPOLE_WEIGHT},
             {.key = FlowFieldTypes::JULIA, .weight = JULIA_WEIGHT},
             {.key = FlowFieldTypes::PERLIN, .weight = PERLIN_WEIGHT},
             {.key = FlowFieldTypes::TEST,   .weight = TEST_WEIGHT},
            }
    }
{
}

auto FlowField::GetZoomAdjustment(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  switch (m_flowFieldType)
  {
    case FlowFieldTypes::DIPOLE:
      return m_dipoleFlowField.GetZoomAdjustment(GetBaseZoomAdjustment(), coords);
    case FlowFieldTypes::JULIA:
      return m_juliaFlowField.GetZoomAdjustment(GetBaseZoomAdjustment(), coords);
    case FlowFieldTypes::PERLIN:
      return m_perlinFlowField.GetZoomAdjustment(GetBaseZoomAdjustment(), coords);
    case FlowFieldTypes::TEST:
      return m_testFlowField.GetZoomAdjustment(GetBaseZoomAdjustment(), coords);
  }
}

auto FlowField::SetRandomParams() noexcept -> void
{
  m_flowFieldType = m_flowFieldTypeWeights.GetRandomWeighted();

  switch (m_flowFieldType)
  {
    case FlowFieldTypes::DIPOLE:
      m_dipoleFlowField.SetRandomParams();
      break;
    case FlowFieldTypes::JULIA:
      m_juliaFlowField.SetRandomParams();
      break;
    case FlowFieldTypes::PERLIN:
      m_perlinFlowField.SetRandomParams();
      break;
    case FlowFieldTypes::TEST:
      m_testFlowField.SetRandomParams();
      break;
  }
}

auto FlowField::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  return {};
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
