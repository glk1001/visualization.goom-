#pragma once

#include "filter_fx/filter_settings.h"
#include "filter_fx/filter_settings_service.h"

#include <memory>
#include <string>

namespace GOOM::UTILS::MATH
{
class IGoomRand;
}

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

[[nodiscard]] auto CreateZoomAdjustmentEffect(ZoomFilterMode filterMode,
                                              const UTILS::MATH::IGoomRand& goomRand,
                                              const std::string& resourcesDirectory)
    -> std::unique_ptr<IZoomAdjustmentEffect>;

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
