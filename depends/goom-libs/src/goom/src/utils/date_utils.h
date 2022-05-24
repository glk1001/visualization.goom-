#pragma once

#include <chrono>
#include <string>

namespace GOOM::UTILS
{

[[nodiscard]] auto GetCurrentDateTimeAsString() noexcept -> std::string;

[[nodiscard]] auto GetSteadyClockAsString(
    const std::chrono::steady_clock::time_point& timePoint) noexcept -> std::string;

} // namespace GOOM::UTILS
