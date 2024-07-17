module;

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

module Goom.Lib.GoomUtils;

import Goom.Utils.StrUtils;
import Goom.Utils.BuildTime;
import Goom.Utils.Math.Rand.RandUtils;
import Goom.Lib.CompilerVersions;
import Goom.Lib.GoomVersion;

namespace GOOM
{

auto FindAndReplaceAll(std::string& dataStr,
                       const std::string& searchStr,
                       const std::string& replaceStr) -> void
{
  UTILS::FindAndReplaceAll(dataStr, searchStr, replaceStr);
}

auto PutFileLines(std::ostream& outStream, const std::vector<std::string>& lines) -> void
{
  UTILS::PutFileLines(outStream, lines);
}

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
auto GetAbsoluteIncludeDir(const std::string& includeDir,
                           const std::string& inFilename) -> std::string
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

  UTILS::PutFileLines(outFilepath,
                      UTILS::GetFileLinesWithExpandedIncludes(absoluteIncludeDir, inFilepath));
}

auto GetFileWithExpandedIncludes(const std::string& includeDir,
                                 const std::string& filepath) -> std::vector<std::string>
{
  const auto absoluteIncludeDir = GetAbsoluteIncludeDir(includeDir, filepath);

  return UTILS::GetFileLinesWithExpandedIncludes(absoluteIncludeDir, filepath);
}

} // namespace GOOM
