module;

#include <memory>
#include <string>

export module Goom.FilterFx.FilterEffects.AdjustmentEffects.FunctionOfFunction;

import Goom.FilterFx.FilterEffects.ZoomAdjustmentEffect;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.Point2d;

using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::NumberRange;
using GOOM::UTILS::MATH::UNIT_RANGE;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class FunctionOfFunction : public IZoomAdjustmentEffect
{
  using FuncEffect = std::unique_ptr<IZoomAdjustmentEffect>;

public:
  FunctionOfFunction(const GoomRand& goomRand,
                     const std::string& name,
                     FuncEffect&& funcOf,
                     FuncEffect&& func) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Vec2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> UTILS::NameValuePairs override;

private:
  const GoomRand* m_goomRand;
  std::string m_name;
  FuncEffect m_funcOf;
  FuncEffect m_func;

  static constexpr auto FUNC_TO_FUNC_OF_LERP_VALUE_RANGE = NumberRange{0.00F, 0.75F};
  float m_funcToFuncOfLerpValue                          = FUNC_TO_FUNC_OF_LERP_VALUE_RANGE.min;

  static constexpr auto COORDS_TO_FUNC_COORDS_LERP_VALUE_RANGE = UNIT_RANGE;
  float m_coordsToFuncCoordsLerpValue = COORDS_TO_FUNC_COORDS_LERP_VALUE_RANGE.min;

  static constexpr auto PROB_USE_FULL_FUNC_OF = 0.1F;
  bool m_useFullFuncOf                        = false;
};

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
