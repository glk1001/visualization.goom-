#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <istream>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace GOOM::UTILS
{

auto FindAndReplaceAll(std::string& dataStr,
                       const std::string& searchStr,
                       const std::string& replaceStr) -> void;

// NOLINTNEXTLINE(readability-identifier-naming)
[[nodiscard]] auto bool_to_string(bool value) -> std::string;

[[nodiscard]] auto ImageBufferIndexToString(int32_t imageWidth, size_t bufferIndex) -> std::string;

[[nodiscard]] auto StringSplit(const std::string& str, const std::string_view& delim)
    -> std::vector<std::string>;

[[nodiscard]] auto StringJoin(const std::vector<std::string>& strings,
                              const std::string_view& delim) -> std::string;

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

[[nodiscard]] auto GetFileLinesWithExpandedIncludes(const std::string& includeDir,
                                                    const std::string& filepath)
    -> std::vector<std::string>;
[[nodiscard]] auto GetFileLinesWithExpandedIncludes(const std::string& includeDir,
                                                    const std::vector<std::string>& inLines)
    -> std::vector<std::string>;

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

} // namespace GOOM::UTILS
