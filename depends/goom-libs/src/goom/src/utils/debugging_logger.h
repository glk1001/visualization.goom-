#pragma once

namespace GOOM
{
class GoomLogger;
}

namespace GOOM::UTILS
{

auto SetGoomLogger(GoomLogger& goomLogger) noexcept -> void;
[[nodiscard]] auto GetGoomLogger() noexcept -> GoomLogger&;

} // namespace GOOM::UTILS
