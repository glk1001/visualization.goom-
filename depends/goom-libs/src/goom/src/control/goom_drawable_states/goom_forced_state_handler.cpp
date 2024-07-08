module;

#include <array>
#include <ranges>
#include <vector>

module Goom.Control.GoomForcedStateHandler;

import Goom.Control.GoomDrawables;
import Goom.Control.GoomDrawablesData;
import Goom.Control.GoomStateHandler;
import Goom.Utils.Math.GoomRandBase;

namespace GOOM::CONTROL
{

using UTILS::MATH::GetMidpoint;

//static constexpr auto FORCED_STATE = std::array{GoomDrawables::DOTS};
//static constexpr auto FORCED_STATE = std::array{GoomDrawables::TUBES};
//static constexpr auto FORCED_STATE = std::array{GoomDrawables::LINES};
//static constexpr auto FORCED_STATE = std::array{GoomDrawables::L_SYSTEM};
//static constexpr auto FORCED_STATE = std::array{GoomDrawables::CIRCLES};
//static constexpr auto FORCED_STATE = std::array{GoomDrawables::STARS};
static constexpr auto FORCED_STATE = std::array{GoomDrawables::PARTICLES};

GoomForcedStateHandler::GoomForcedStateHandler() noexcept
  : m_constDrawablesState{
        GetGoomDrawablesState(std::vector<GoomDrawables>{cbegin(FORCED_STATE), cend(FORCED_STATE)})}
{
}

auto GoomForcedStateHandler::GetGoomDrawablesState(
    const std::vector<GoomDrawables>& drawables) noexcept -> GoomDrawablesState
{
  return GoomDrawablesState{drawables, GetBuffIntensities(drawables)};
}

auto GoomForcedStateHandler::GetBuffIntensities(
    const std::vector<GoomDrawables>& drawables) noexcept -> std::vector<float>
{
  return std::ranges::to<std::vector<float>>(
      drawables | std::views::transform([](const auto drawable)
                                        { return GetMidpoint(BUFF_INTENSITY_RANGES[drawable]); }));
}

} // namespace GOOM::CONTROL
