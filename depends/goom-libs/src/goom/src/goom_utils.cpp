#include "goom_utils.h"

#include "compiler_versions.h"
#include "goom_version.h"
#include "utils/build_time.h"
#include "utils/math/randutils.h"
#include "utils/strutils.h"

#include <filesystem>

namespace GOOM
{

using UTILS::GetFileLinesWithExpandedIncludes;
using UTILS::StringJoin;

auto GetRandSeed() noexcept -> uint64_t
{
  return UTILS::MATH::RAND::GetRandSeed();
}

auto SetRandSeed(const uint64_t seed) noexcept -> void
{
  UTILS::MATH::RAND::SetRandSeed(seed);
}

auto GetGoomLibCompilerVersion() noexcept -> std::string
{
  return GetCompilerVersion();
}

auto GetGoomLibVersionInfo() noexcept -> std::string
{
  return GetFullVersionStr();
}

auto GetGoomLibBuildTime() noexcept -> std::string
{
  return UTILS::GetBuildTime();
}

namespace
{

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto GetAbsoluteIncludeDir(const std::string& includeDir, const std::string& inFilename)
    -> std::string
{
  if (not includeDir.empty())
  {
    return std::filesystem::absolute(includeDir);
  }
  return std::filesystem::path(inFilename).parent_path();
}

} // namespace

auto GetFileContentsWithExpandedIncludes(const std::string& includeDir, const std::string& filepath)
    -> std::string
{
  const auto absoluteIncludeDir = GetAbsoluteIncludeDir(includeDir, filepath);

  return StringJoin(GetFileLinesWithExpandedIncludes(absoluteIncludeDir, filepath), "\n");
}

} // namespace GOOM
