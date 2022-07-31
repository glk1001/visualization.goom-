#pragma once

#include <cstdint>
#include <string>

namespace GOOM
{

static constexpr auto GOOM_VER_NAME        = "Goom++ Library";
static constexpr auto GOOM_VER_DESCRIPTION = "Goom++ Initial Release";
static constexpr auto GOOM_VER_MAJOR       = 1;
static constexpr auto GOOM_VER_MINOR       = 0;
static constexpr auto GOOM_VER_REVISION    = 0;

struct VersionInfoInt
{
  int32_t verMajor;
  int32_t verMinor;
  int32_t verRevision;
};

[[nodiscard]] auto GetFullVersionStr() -> std::string;

[[nodiscard]] inline auto GetVersionInfo() -> VersionInfoInt
{
  return {GOOM_VER_MAJOR, GOOM_VER_MINOR, GOOM_VER_REVISION};
}

} // namespace GOOM
