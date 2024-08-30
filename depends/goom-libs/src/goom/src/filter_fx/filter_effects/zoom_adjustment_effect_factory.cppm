module;

#include <memory>
#include <string>

export module Goom.FilterFx.FilterEffects.ZoomAdjustmentEffectFactory;

import Goom.FilterFx.FilterEffects.ZoomAdjustmentEffect;
import Goom.FilterFx.FilterSettingsService;
import Goom.Utils.Math.GoomRand;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

[[nodiscard]] auto CreateZoomAdjustmentEffect(ZoomFilterMode filterMode,
                                              const UTILS::MATH::GoomRand& goomRand,
                                              const std::string& resourcesDirectory)
    -> std::unique_ptr<IZoomAdjustmentEffect>;

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
