module;

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <istream>
#include <iterator>
#include <ostream>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

export module Goom.Utils.StrUtils;

export namespace GOOM::UTILS
{

auto FindAndReplaceAll(std::string& dataStr,
                       const std::string& searchStr,
                       const std::string& replaceStr) -> void;

// NOLINTNEXTLINE(readability-identifier-naming)
[[nodiscard]] auto bool_to_string(bool value) -> std::string;

[[nodiscard]] auto ImageBufferIndexToString(int32_t imageWidth, size_t bufferIndex) -> std::string;

[[nodiscard]] auto StringSplit(const std::string& str,
                               const std::string_view& delim) -> std::vector<std::string>;

[[nodiscard]] auto StringJoin(const std::vector<std::string>& strings,
                              const std::string_view& delim) -> std::string;

template<typename T, class UnaryOp>
[[nodiscard]] auto ToStrings(const std::vector<T>& array,
                             UnaryOp toString) -> std::vector<std::string>;

auto LTrim(std::string& str) noexcept -> void;
auto RTrim(std::string& str) noexcept -> void;
auto Trim(std::string& str) noexcept -> void;
[[nodiscard]] auto LTrimAndCopy(std::string str) noexcept -> std::string;
[[nodiscard]] auto RTrimAndCopy(std::string str) noexcept -> std::string;
[[nodiscard]] auto TrimAndCopy(std::string str) noexcept -> std::string;

[[nodiscard]] auto GetFileLines(const std::string& filepath) -> std::vector<std::string>;
[[nodiscard]] auto GetFileLines(std::istream& inStream) -> std::vector<std::string>;

auto PutFileLines(const std::string& filepath, const std::vector<std::string>& lines) -> void;
auto PutFileLines(std::ostream& outStream, const std::vector<std::string>& lines) -> void;

[[nodiscard]] auto GetFileLinesWithExpandedIncludes(
    const std::string& includeDir, const std::string& filepath) -> std::vector<std::string>;
[[nodiscard]] auto GetFileLinesWithExpandedIncludes(const std::string& includeDir,
                                                    const std::vector<std::string>& inLines)
    -> std::vector<std::string>;

} // namespace GOOM::UTILS

namespace GOOM::UTILS
{

inline auto LTrim(std::string& str) noexcept -> void
{
  str.erase(str.begin(),
            std::find_if(str.begin(),
                         str.end(),
                         [](const unsigned char chr) { return 0 == std::isspace(chr); }));
}

inline auto RTrim(std::string& str) noexcept -> void
{
  str.erase(std::find_if(str.rbegin(),
                         str.rend(),
                         [](const unsigned char chr) { return 0 == std::isspace(chr); })
                .base(),
            str.end());
}

inline auto Trim(std::string& str) noexcept -> void
{
  RTrim(str);
  LTrim(str);
}

inline auto LTrimAndCopy(std::string str) noexcept -> std::string
{
  LTrim(str);
  return str;
}

inline auto RTrimAndCopy(std::string str) noexcept -> std::string
{
  RTrim(str);
  return str;
}

inline auto TrimAndCopy(std::string str) noexcept -> std::string
{
  Trim(str);
  return str;
}

template<typename T, class UnaryOp>
auto ToStrings(const std::vector<T>& array, const UnaryOp toString) -> std::vector<std::string>
{
  auto output = std::vector<std::string>(array.size());

  std::ranges::transform(array, begin(output), toString);

  return output;
}

} // namespace GOOM::UTILS

module :private;

namespace GOOM::UTILS
{

[[nodiscard]] auto GetIncludeFileName(const std::string& includeDir,
                                      const std::string& parentFileDir,
                                      const std::string& includeLine) -> std::string;
[[nodiscard]] auto GetExpandedFileLines(const std::string& includeDir,
                                        const std::string& parentFileDir,
                                        const std::vector<std::string>& inLines)
    -> std::vector<std::string>;

auto GetFileLines(const std::string& filepath) -> std::vector<std::string>
{
  auto inStream = std::ifstream{filepath};
  if (not inStream)
  {
    throw std::runtime_error{std::format("Could not open file \"{}\"", filepath)};
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
    throw std::runtime_error{std::format(R"(Could not open file "{}")", filepath)};
  }

  PutFileLines(outStream, lines);
}

auto PutFileLines(std::ostream& outStream, const std::vector<std::string>& lines) -> void
{
  for (const auto& line : lines)
  {
    outStream << line << "\n";
  }
}

// NOLINTNEXTLINE(misc-no-recursion)
auto GetFileLinesWithExpandedIncludes(const std::string& includeDir,
                                      const std::string& filepath) -> std::vector<std::string>
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
  return std::format("{:5d}, {:5d}", x, y);
}

auto StringJoin(const std::vector<std::string>& strings,
                const std::string_view& delim) -> std::string
{
  auto joined = strings | std::views::join_with(delim);

  return std::ranges::to<std::string>(joined);
}

auto StringSplit(const std::string& str, const std::string_view& delim) -> std::vector<std::string>
{
  auto parts = str | std::ranges::views::split(delim);

  auto splitVec = std::ranges::to<std::vector<std::string>>(parts);

  if (splitVec.back().empty())
  {
    splitVec.pop_back();
  }

  return splitVec;
}

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
        std::format(R"(Expected #include filename '{}' to start with ")", filename));
  }
  if (filename[filename.size() - 1] != '\"')
  {
    throw std::runtime_error(
        std::format(R"(Expected #include filename '{}' to end with ")", filename));
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
        std::format(R"(Could not open file "{}" or "{}")", theFilename, inclFilename)};
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
    throw std::runtime_error{std::format(R"(Could not find include directory "{}")", includeDir)};
  }

  auto outLines = std::vector<std::string>{};
  for (const auto& line : inLines)
  {
    const auto trimmedLine = RTrimAndCopy(line);

    if (static constexpr auto INCLUDE = std::string_view{"#include"};
        not trimmedLine.starts_with(INCLUDE))
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

} // namespace GOOM::UTILS
