module;

#include <string>
#include <vector>

export module Goom.Control.GoomDrawablesData;

import Goom.Control.GoomDrawables;
import Goom.Utils.Math.GoomRand;

using GOOM::UTILS::MATH::GoomRand;

export namespace GOOM::CONTROL
{

[[nodiscard]] auto GetDrawablesStateName(const std::vector<GoomDrawables>& drawables)
    -> std::string;

[[nodiscard]] auto GetRandInRangeBuffIntensities(
    const GoomRand& goomRand,
    const std::vector<GoomDrawables>& drawables) noexcept -> std::vector<float>;

[[nodiscard]] auto GetMidpointRangeBuffIntensities(
    const std::vector<GoomDrawables>& drawables) noexcept -> std::vector<float>;

[[nodiscard]] auto AreAnyMultiThreaded(const std::vector<GoomDrawables>& drawables) noexcept
    -> bool;

[[nodiscard]] auto GetProbCanBeSingleDrawable(GoomDrawables drawable) noexcept -> float;

} // namespace GOOM::CONTROL
