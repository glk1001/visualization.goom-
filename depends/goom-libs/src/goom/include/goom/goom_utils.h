#pragma once

#include <cstdint>
#include <string>

namespace GOOM
{

[[nodiscard]] auto GetRandSeed() noexcept -> uint64_t;
auto SetRandSeed(uint64_t seed) noexcept -> void;

[[nodiscard]] auto GetGoomLibCompilerVersion() noexcept -> std::string;

[[nodiscard]] auto GetGoomLibVersionInfo() noexcept -> std::string;
[[nodiscard]] auto GetGoomLibBuildTime() noexcept -> std::string;

} // namespace GOOM
