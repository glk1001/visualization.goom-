module;

#include <format>
#include <string>
#include <utility>

module Goom.FilterFx.FilterEffects.AdjustmentEffects.FunctionOfFunction;

import Goom.FilterFx.FilterEffects.ZoomAdjustmentEffect;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.Point2d;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::MATH::GoomRand;

FunctionOfFunction::FunctionOfFunction(const GoomRand& goomRand,
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

  m_funcToFuncOfLerpValue = m_goomRand->GetRandInRange<FUNC_TO_FUNC_OF_LERP_VALUE_RANGE>();

  m_coordsToFuncCoordsLerpValue =
      m_goomRand->GetRandInRange<COORDS_TO_FUNC_COORDS_LERP_VALUE_RANGE>();

  m_useFullFuncOf = m_goomRand->ProbabilityOf<PROB_USE_FULL_FUNC_OF>();
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

auto FunctionOfFunction::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  auto nameValueParams = NameValuePairs{GetPair(PARAM_GROUP,
                                                "FoF",
                                                std::format("{}, {}, {:.2f}, {:.2f}",
                                                            m_name,
                                                            m_useFullFuncOf,
                                                            m_funcToFuncOfLerpValue,
                                                            m_coordsToFuncCoordsLerpValue))};

  const auto funcNameValueParams   = m_func->GetZoomAdjustmentEffectNameValueParams();
  const auto funcOfNameValueParams = m_funcOf->GetZoomAdjustmentEffectNameValueParams();
  nameValueParams.insert(
      end(nameValueParams), cbegin(funcOfNameValueParams), cend(funcOfNameValueParams));
  nameValueParams.insert(
      end(nameValueParams), cbegin(funcNameValueParams), cend(funcNameValueParams));

  return nameValueParams;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
