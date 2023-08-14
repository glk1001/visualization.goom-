#include "strutils.h"

#if __cplusplus > 201703L // NOLINT: Can't include header for this.
#include <ranges>
#endif
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format> // NOLINT: Waiting to use C++20.
#include <fstream>
#include <istream>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace GOOM::UTILS
{

namespace
{

auto GetIncludeFileName(const std::string& includeDir,
                        // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
                        const std::string& parentFileDir,
                        const std::string& includeLine) -> std::string
{
  const auto words = StringSplit(includeLine, " ");
  if (words.size() <= 1)
  {
    throw std::runtime_error("Expected filename after #include");
  }
  const auto& filename = words.at(1);
  if (filename.size() < 2)
  {
    throw std::runtime_error("#include filename is not double quoted");
  }
  if (filename[0] != '\"')
  {
    throw std::runtime_error(
        // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
        std_fmt::format("Expected #include filename '{}' to start with \"", filename));
  }
  if (filename[filename.size() - 1] != '\"')
  {
    throw std::runtime_error(
        // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
        std_fmt::format("Expected #include filename '{}' to end with \"", filename));
  }
  if (filename.size() <= 2)
  {
    throw std::runtime_error("#include filename is empty");
  }

  auto theFilename = filename.substr(1, filename.size() - 2);
  if (not parentFileDir.empty())
  {
    theFilename = parentFileDir + "/" + filename.substr(1, filename.size() - 2);
    if (std::filesystem::exists(theFilename))
    {
      return theFilename;
    }
  }

  auto inclFilename = std::filesystem::absolute(includeDir + "/" + theFilename).string();
  if (not std::filesystem::exists(inclFilename))
  {
    throw std::runtime_error{
        // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
        std_fmt::format("Could not open file \"{}\" or \"{}\"", theFilename, inclFilename)};
  }
  return inclFilename;
}

// NOLINTNEXTLINE(misc-no-recursion)
auto GetExpandedFileLines(const std::string& includeDir,
                          const std::string& parentFileDir,
                          const std::vector<std::string>& inLines) -> std::vector<std::string>
{
  if (not std::filesystem::exists(includeDir))
  {
    throw std::runtime_error{
        // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
        std_fmt::format("Could not find include directory \"{}\"", includeDir)};
  }

  auto outLines = std::vector<std::string>{};
  for (const auto& line : inLines)
  {
    const auto trimmedLine = RTrimAndCopy(line);

    if (static constexpr auto INCLUDE = std::string_view{"#include"};
        trimmedLine.substr(0, INCLUDE.size()) != INCLUDE)
    {
      outLines.push_back(line);
    }
    else
    {
      const auto includeLines = GetFileLinesWithExpandedIncludes(
          includeDir, GetIncludeFileName(includeDir, parentFileDir, trimmedLine));
      std::copy(cbegin(includeLines), cend(includeLines), std::back_inserter(outLines));
    }
  }

  return outLines;
}

} // namespace

auto GetFileLines(const std::string& filepath) -> std::vector<std::string>
{
  auto inStream = std::ifstream{filepath};
  if (not inStream)
  {
    // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
    throw std::runtime_error{std_fmt::format("Could not open file \"{}\"", filepath)};
  }

  return GetFileLines(inStream);
}

auto GetFileLines(std::istream& inStream) -> std::vector<std::string>
{
  auto lines = std::vector<std::string>{};
  while (true)
  {
    auto line = std::string{};
    getline(inStream, line);
    if (inStream.eof())
    {
      break;
    }
    lines.push_back(line);
  }
  return lines;
}

auto PutFileLines(const std::string& filepath, const std::vector<std::string>& lines) -> void
{
  auto outStream = std::ofstream{filepath};
  if (not outStream)
  {
    // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
    throw std::runtime_error{std_fmt::format("Could not open file \"{}\"", filepath)};
  }

  PutFileLines(outStream, lines);
}

auto PutFileLines(std::ostream& outStream, const std::vector<std::string>& lines) -> void
{
  for (auto i = 0U; i < lines.size(); ++i)
  {
    outStream << lines.at(i) << "\n";
  }
}

// NOLINTNEXTLINE(misc-no-recursion)
auto GetFileLinesWithExpandedIncludes(const std::string& includeDir, const std::string& filepath)
    -> std::vector<std::string>
{
  const auto parentFileDir = std::filesystem::path(filepath).parent_path().string();
  return GetExpandedFileLines(includeDir, parentFileDir, GetFileLines(filepath));
}

auto GetFileLinesWithExpandedIncludes(const std::string& includeDir,
                                      const std::vector<std::string>& inLines)
    -> std::vector<std::string>
{
  return GetExpandedFileLines(includeDir, "", inLines);
}

auto FindAndReplaceAll(std::string& dataStr,
                       const std::string& searchStr,
                       const std::string& replaceStr) -> void
{
  auto pos = dataStr.find(searchStr);
  while (pos != std::string::npos)
  {
    dataStr.replace(pos, searchStr.size(), replaceStr);
    pos = dataStr.find(searchStr, pos + replaceStr.size());
  }
}

auto bool_to_string(const bool value) -> std::string
{
  return value ? "true" : "false";
}

auto ImageBufferIndexToString(const int32_t imageWidth, const size_t bufferIndex) -> std::string
{
  const auto y = bufferIndex / static_cast<size_t>(imageWidth);
  const auto x = bufferIndex % static_cast<size_t>(imageWidth);
  // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
  return std_fmt::format("{:5d}, {:5d}", x, y);
}

auto StringJoin(const std::vector<std::string>& strings, const std::string_view& delim)
    -> std::string
{
  auto joinedStr = std::string{};

  for (auto str = cbegin(strings); str != cend(strings); ++str)
  {
    joinedStr += *str;
    if (str != (cend(strings) - 1))
    {
      joinedStr += delim;
    }
  }

  return joinedStr;
}

auto StringSplit(const std::string& str, const std::string_view& delim) -> std::vector<std::string>
{
#if __cplusplus <= 201703L // NOLINT: Can't include header for this.
  auto vec       = std::vector<std::string>{};
  auto copyOfStr = str;
  while (true)
  {
    const auto pos = copyOfStr.find(delim);
    if (pos == std::string::npos)
    {
      break;
    }
    const auto token = copyOfStr.substr(0, pos);
    copyOfStr.erase(0, pos + delim.length());
    vec.emplace_back(token);
  }
  if (!copyOfStr.empty())
  {
    vec.emplace_back(copyOfStr);
  }
  return vec;
#else
  auto parts = str | std::ranges::views::split(delim);
  std::vector<std::string> vec;
  for (auto part : parts)
  {
    std::string s = "";
    for (auto c : part)
    {
      s += c;
    }
    vec.emplace_back(s);
  }
  return vec;
#endif
}

} // namespace GOOM::UTILS
