#pragma once

#include "filter_fx/normalized_coords.h"
#include "filter_fx/zoom_adjustment_effect.h"
#include "goom/point2d.h"
#include "utils/name_value_pairs.h"

#include <memory>
#include <string>
#include <utility>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class FunctionOfFunction : public IZoomAdjustmentEffect
{
  using FuncEffect = std::unique_ptr<IZoomAdjustmentEffect>;

public:
  FunctionOfFunction(const std::string& name, FuncEffect&& funcOf, FuncEffect&& func) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Vec2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> UTILS::NameValuePairs override;

private:
  std::string m_name;
  FuncEffect m_funcOf;
  FuncEffect m_func;
};

inline FunctionOfFunction::FunctionOfFunction(const std::string& name,
                                              FuncEffect&& funcOf,
                                              FuncEffect&& func) noexcept
  : m_name{name}, m_funcOf{std::move(funcOf)}, m_func{std::move(func)}
{
}

inline auto FunctionOfFunction::SetRandomParams() noexcept -> void
{
  m_func->SetRandomParams();
  m_funcOf->SetRandomParams();
}

inline auto FunctionOfFunction::GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
    -> Vec2dFlt
{
  //const auto funcCoords = m_func->GetZoomAdjustment(coords);
  //return m_funcOf->GetZoomAdjustment({funcCoords.x, funcCoords.y});

  //const auto funcOfFuncCoords = m_funcOf->GetZoomAdjustment(coords);
  //return m_funcOf->GetZoomAdjustment(
  //    {funcOfFuncCoords.x * funcCoords.x, funcOfFuncCoords.y * funcCoords.y});

  const auto funcCoords       = ToPoint2dFlt(m_func->GetZoomAdjustment(coords));
  const auto funcOfFuncCoords = ToPoint2dFlt(m_funcOf->GetZoomAdjustment(coords));
  return ToVec2dFlt(lerp(funcCoords, funcOfFuncCoords, 0.2F));
}

inline auto FunctionOfFunction::GetZoomAdjustmentEffectNameValueParams() const noexcept
    -> UTILS::NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "Zoom Adj";
  auto nameValueParams               = UTILS::NameValuePairs{
      UTILS::GetPair(PARAM_GROUP, "FoF", m_name),
  };
  const auto funcNameValueParams   = m_func->GetZoomAdjustmentEffectNameValueParams();
  const auto funcOfNameValueParams = m_funcOf->GetZoomAdjustmentEffectNameValueParams();
  nameValueParams.insert(
      end(nameValueParams), cbegin(funcOfNameValueParams), cend(funcOfNameValueParams));
  nameValueParams.insert(
      end(nameValueParams), cbegin(funcNameValueParams), cend(funcNameValueParams));
  return nameValueParams;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
