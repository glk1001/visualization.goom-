module;

#include <memory>
#include <string>

export module Goom.FilterFx.FilterEffects.ZoomAdjustmentEffectFactory;

import Goom.FilterFx.FilterSettings;
import Goom.FilterFx.FilterSettingsService;
import Goom.FilterFx.ZoomAdjustmentEffect;
import Goom.Utils.Math.GoomRandBase;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

[[nodiscard]] extern auto CreateZoomAdjustmentEffect(ZoomFilterMode filterMode,
                                                     const UTILS::MATH::IGoomRand& goomRand,
                                                     const std::string& resourcesDirectory)
    -> std::shared_ptr<IZoomAdjustmentEffect>;

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
