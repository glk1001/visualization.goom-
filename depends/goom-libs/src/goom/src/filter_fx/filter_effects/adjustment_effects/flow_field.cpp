module Goom.FilterFx.FilterEffects.AdjustmentEffects.FlowField;

import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::NameValuePairs;
using UTILS::MATH::GoomRand;

static constexpr auto FORCE_FLOW_FIELD_TYPE  = true;
static constexpr auto FORCED_FLOW_FIELD_TYPE = FlowField::FlowFieldTypes::SIN_COS;

static constexpr auto DIPOLE_WEIGHT  = 1.0F;
static constexpr auto PERLIN_WEIGHT  = 1.0F;
static constexpr auto SIN_COS_WEIGHT = 1.0F;
static constexpr auto TEST_WEIGHT    = 0.01F;

FlowField::FlowField(const GoomRand& goomRand) noexcept
  : m_dipoleFlowField{goomRand},
    m_perlinFlowField{goomRand},
    m_sinCosFlowField{goomRand},
    m_testFlowField{goomRand},
    m_flowFieldTypeWeights{
        goomRand,
  {
             {.key = FlowFieldTypes::DIPOLE, .weight = DIPOLE_WEIGHT},
             {.key = FlowFieldTypes::PERLIN, .weight = PERLIN_WEIGHT},
             {.key = FlowFieldTypes::SIN_COS, .weight = SIN_COS_WEIGHT},
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
    case FlowFieldTypes::PERLIN:
      return m_perlinFlowField.GetZoomAdjustment(GetBaseZoomAdjustment(), coords);
    case FlowFieldTypes::SIN_COS:
      return m_sinCosFlowField.GetZoomAdjustment(GetBaseZoomAdjustment(), coords);
    case FlowFieldTypes::TEST:
      return m_testFlowField.GetZoomAdjustment(GetBaseZoomAdjustment(), coords);
  }
}

auto FlowField::SetRandomParams() noexcept -> void
{
  if constexpr (FORCE_FLOW_FIELD_TYPE)
  {
    m_flowFieldType = FORCED_FLOW_FIELD_TYPE;
  }
  else
  {
    m_flowFieldType = m_flowFieldTypeWeights.GetRandomWeighted();
  }

  switch (m_flowFieldType)
  {
    case FlowFieldTypes::DIPOLE:
      m_dipoleFlowField.SetRandomParams();
      break;
    case FlowFieldTypes::PERLIN:
      m_perlinFlowField.SetRandomParams();
      break;
    case FlowFieldTypes::SIN_COS:
      m_sinCosFlowField.SetRandomParams();
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
