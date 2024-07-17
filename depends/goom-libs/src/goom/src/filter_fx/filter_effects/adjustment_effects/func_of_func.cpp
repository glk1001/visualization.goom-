module;

#include <string>
#include <utility>

module Goom.FilterFx.FilterEffects.AdjustmentEffects.FunctionOfFunction;

import Goom.FilterFx.NormalizedCoords;
import Goom.FilterFx.ZoomAdjustmentEffect;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRandBase;
import Goom.Lib.Point2d;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::MATH::IGoomRand;

FunctionOfFunction::FunctionOfFunction(const IGoomRand& goomRand,
                                       const std::string& name,
                                       FuncEffect&& funcOf,
                                       FuncEffect&& func) noexcept
  : m_goomRand{&goomRand}, m_name{name}, m_funcOf{std::move(funcOf)}, m_func{std::move(func)}
{
}

auto FunctionOfFunction::SetRandomParams() noexcept -> void
{
  m_func->SetRandomParams();
  m_funcOf->SetRandomParams();

  m_funcToFuncOfLerpValue = m_goomRand->GetRandInRange(FUNC_TO_FUNC_OF_LERP_VALUE_RANGE);

  m_coordsToFuncCoordsLerpValue =
      m_goomRand->GetRandInRange(COORDS_TO_FUNC_COORDS_LERP_VALUE_RANGE);

  m_useFullFuncOf = m_goomRand->ProbabilityOf(PROB_USE_FULL_FUNC_OF);
}

auto FunctionOfFunction::GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
    -> Vec2dFlt
{
  //const auto funcCoords = m_func->GetZoomAdjustment(coords);
  //return m_funcOf->GetZoomAdjustment({funcCoords.x, funcCoords.y});

  //const auto funcOfFuncCoords = m_funcOf->GetZoomAdjustment(coords);
  //return m_funcOf->GetZoomAdjustment(
  //    {funcOfFuncCoords.x * funcCoords.x, funcOfFuncCoords.y * funcCoords.y});

  const auto coordsFlt  = ToVec2dFlt(coords.GetFltCoords());
  const auto funcCoords = m_func->GetZoomAdjustment(coords);

  const auto coordsToFuncCoordsLerpValue = m_useFullFuncOf ? 1.0F : m_coordsToFuncCoordsLerpValue;
  const auto coordsToFuncCoordsLerp      = lerp(coordsFlt, funcCoords, coordsToFuncCoordsLerpValue);
  const auto funcOfFuncCoords =
      m_funcOf->GetZoomAdjustment({coordsToFuncCoordsLerp.x, coordsToFuncCoordsLerp.y});

  return lerp(funcCoords, funcOfFuncCoords, m_funcToFuncOfLerpValue);
}

auto FunctionOfFunction::GetZoomAdjustmentEffectNameValueParams() const noexcept
    -> UTILS::NameValuePairs
{
  auto nameValueParams = UTILS::NameValuePairs{
      UTILS::GetPair(PARAM_GROUP, "FoF", m_name),
  };
  nameValueParams.emplace_back(UTILS::GetPair(PARAM_GROUP, "use full", m_useFullFuncOf));
  nameValueParams.emplace_back(UTILS::GetPair(PARAM_GROUP, "funcOfLerp", m_funcToFuncOfLerpValue));
  nameValueParams.emplace_back(
      UTILS::GetPair(PARAM_GROUP, "coordsToFuncLerp", m_coordsToFuncCoordsLerpValue));
  const auto funcNameValueParams   = m_func->GetZoomAdjustmentEffectNameValueParams();
  const auto funcOfNameValueParams = m_funcOf->GetZoomAdjustmentEffectNameValueParams();
  nameValueParams.insert(
      end(nameValueParams), cbegin(funcOfNameValueParams), cend(funcOfNameValueParams));
  nameValueParams.insert(
      end(nameValueParams), cbegin(funcNameValueParams), cend(funcNameValueParams));
  return nameValueParams;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
