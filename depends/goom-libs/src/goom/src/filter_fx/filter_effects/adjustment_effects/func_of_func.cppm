module;

#include <memory>
#include <string>

export module Goom.FilterFx.FilterEffects.AdjustmentEffects.FunctionOfFunction;

import Goom.FilterFx.NormalizedCoords;
import Goom.FilterFx.ZoomAdjustmentEffect;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRandBase;
import Goom.Lib.Point2d;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class FunctionOfFunction : public IZoomAdjustmentEffect
{
  using FuncEffect = std::unique_ptr<IZoomAdjustmentEffect>;

public:
  FunctionOfFunction(const UTILS::MATH::IGoomRand& goomRand,
                     const std::string& name,
                     FuncEffect&& funcOf,
                     FuncEffect&& func) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Vec2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> UTILS::NameValuePairs override;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  std::string m_name;
  FuncEffect m_funcOf;
  FuncEffect m_func;

  static constexpr auto MIN_FUNC_TO_FUNC_OF_LERP_VALUE = 0.00F;
  static constexpr auto MAX_FUNC_TO_FUNC_OF_LERP_VALUE = 0.75F;
  float m_funcToFuncOfLerpValue                        = MIN_FUNC_TO_FUNC_OF_LERP_VALUE;

  static constexpr auto MIN_COORDS_TO_FUNC_COORDS_LERP_VALUE = 0.0F;
  static constexpr auto MAX_COORDS_TO_FUNC_COORDS_LERP_VALUE = 1.0F;
  float m_coordsToFuncCoordsLerpValue                        = MIN_COORDS_TO_FUNC_COORDS_LERP_VALUE;

  static constexpr auto PROB_USE_FULL_FUNC_OF = 0.1F;
  bool m_useFullFuncOf                        = false;
};

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
