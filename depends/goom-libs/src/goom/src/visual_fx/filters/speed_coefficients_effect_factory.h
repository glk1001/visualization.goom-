#pragma once

#include "filter_settings_service.h"

#include <memory>

namespace GOOM
{

namespace UTILS
{
class IGoomRand;
class Parallel;
} // namespace UTILS

namespace VISUAL_FX::FILTERS
{

[[nodiscard]] extern auto CreateSpeedCoefficientsEffect(ZoomFilterMode filterMode,
                                                        const UTILS::MATH::IGoomRand& goomRand,
                                                        const std::string& resourcesDirectory)
    -> std::shared_ptr<ISpeedCoefficientsEffect>;

} // namespace VISUAL_FX::FILTERS
} // namespace GOOM
