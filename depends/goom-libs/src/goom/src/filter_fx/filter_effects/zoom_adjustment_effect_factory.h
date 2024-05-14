#pragma once

#include "filter_fx/filter_settings.h"
#include "filter_fx/filter_settings_service.h"

#include <memory>
#include <string>

import Goom.Utils.Math.GoomRandBase;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

[[nodiscard]] extern auto CreateZoomAdjustmentEffect(ZoomFilterMode filterMode,
                                                     const UTILS::MATH::IGoomRand& goomRand,
                                                     const std::string& resourcesDirectory)
    -> std::shared_ptr<IZoomAdjustmentEffect>;

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
