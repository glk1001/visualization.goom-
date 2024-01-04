#pragma once

#include "filter_fx/normalized_coords.h"
#include "filter_fx/zoom_adjustment_effect.h"
#include "goom/point2d.h"
#include "utils/name_value_pairs.h"

#include <memory>
#include <utility>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class FunctionOfFunction : public IZoomAdjustmentEffect
{
  using FuncEffect = std::unique_ptr<IZoomAdjustmentEffect>;

public:
  explicit FunctionOfFunction(FuncEffect&& funcOf, FuncEffect&& func) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Vec2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> UTILS::NameValuePairs override;

private:
  FuncEffect m_funcOf;
  FuncEffect m_func;
};

inline FunctionOfFunction::FunctionOfFunction(FuncEffect&& funcOf, FuncEffect&& func) noexcept
  : m_funcOf{std::move(funcOf)}, m_func{std::move(func)}
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
  const auto funcCoords = m_func->GetZoomAdjustment(coords);
  return m_funcOf->GetZoomAdjustment({funcCoords.x, funcCoords.y});
}

inline auto FunctionOfFunction::GetZoomAdjustmentEffectNameValueParams() const noexcept
    -> UTILS::NameValuePairs
{
  // TODO(glk) - Incorporate m_funcOf.
  return m_func->GetZoomAdjustmentEffectNameValueParams();
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
