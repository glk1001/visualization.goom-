#include "goom/goom_utils.h"

#include "goom/compiler_versions.h"
#include "goom/goom_version.h"
#include "utils/build_time.h"
#include "utils/math/randutils.h"
#include "utils/strutils.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace GOOM
{

using UTILS::GetFileLinesWithExpandedIncludes;
using UTILS::PutFileLines;

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
    return std::filesystem::absolute(includeDir).string();
  }
  return std::filesystem::path(inFilename).parent_path().string();
}

} // namespace

auto PutFileWithExpandedIncludes(const std::string& includeDir,
                                 const std::string& inFilepath,
                                 const std::string& outFilepath) -> void
{
  const auto absoluteIncludeDir = GetAbsoluteIncludeDir(includeDir, inFilepath);

  PutFileLines(outFilepath, GetFileLinesWithExpandedIncludes(absoluteIncludeDir, inFilepath));
}

auto GetFileWithExpandedIncludes(const std::string& includeDir, const std::string& filepath)
    -> std::vector<std::string>
{
  const auto absoluteIncludeDir = GetAbsoluteIncludeDir(includeDir, filepath);

  return GetFileLinesWithExpandedIncludes(absoluteIncludeDir, filepath);
}

} // namespace GOOM
