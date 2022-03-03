#pragma once

#include <cstdint>
#include <string>

namespace GOOM
{

static constexpr auto* GOOM_VER_NAME = "Goom Library";
static constexpr auto* GOOM_VER_DESCRIPTION = "Goom++ Initial Release";
static constexpr auto GOOM_VER_MAJOR = 1;
static constexpr auto GOOM_VER_MINOR = 0;
static constexpr auto GOOM_VER_REVISION = 0;

struct VersionInfoInt
{
  int32_t verMajor;
  int32_t verMinor;
  int32_t verRevision;
};

[[nodiscard]] inline auto GetFullVersionStr() -> std::string
{
  return std::string(GOOM_VER_NAME) + " " + std::to_string(GOOM_VER_MAJOR) + "." +
         std::to_string(GOOM_VER_MINOR) + "." + std::to_string(GOOM_VER_REVISION) + "," + " " +
         GOOM_VER_DESCRIPTION;
}

[[nodiscard]] inline auto GetVersionInfo() -> VersionInfoInt
{
  return {GOOM_VER_MAJOR, GOOM_VER_MINOR, GOOM_VER_REVISION};
}

} // namespace GOOM
