#pragma once

#include "format_utils.h"
#include "goom/goom_graphic.h"
#include "goom/point2d.h"
#include "strutils.h"

#include <algorithm>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace GOOM::UTILS
{

using NameValuePair  = std::pair<std::string, std::string>;
using NameValuePairs = std::vector<NameValuePair>;

[[nodiscard]] inline auto GetNameValueString(const NameValuePair& pair) -> std::string
{
  return pair.first + ": " + pair.second;
}

[[nodiscard]] auto GetNameValuesString(const NameValuePairs& nameValuePairs) -> std::string;

inline void MoveNameValuePairs(NameValuePairs&& nameValues, NameValuePairs& dest)
{
  std::move(cbegin(nameValues), cend(nameValues), std::back_inserter(dest));
}

[[nodiscard]] inline auto GetParamGroupPrefix(const std::string& paramGroup) -> std::string
{
  return paramGroup + "::";
}

[[nodiscard]] inline auto GetFullParamGroup(const std::vector<std::string>& paramGroups)
    -> std::string
{
  auto fullParamGroup = std::string{};
  for (auto str = cbegin(paramGroups); str != cend(paramGroups); ++str)
  {
    if (str != (cend(paramGroups) - 1))
    {
      fullParamGroup += GetParamGroupPrefix(*str);
    }
    else
    {
      fullParamGroup += *str;
    }
  }
  return fullParamGroup;
}

template<typename T>
[[nodiscard]] auto GetPair(const std::string& paramGroup,
                           const std::string& paramName,
                           const T& paramValue) -> NameValuePair;

template<typename T>
// NOLINTNEXTLINE(readability-identifier-naming)
[[nodiscard]] auto to_string(const T& value) -> std::string;


template<typename T>
auto GetPair(const std::string& paramGroup, const std::string& paramName, const T& paramValue)
    -> NameValuePair
{
  return {paramGroup + "::" + paramName, to_string(paramValue)};
}

// NOLINTBEGIN(readability-identifier-naming)
template<typename T>
inline auto to_string(const T& value) -> std::string
{
  return std::to_string(value);
}

template<>
inline auto to_string(const std::string& value) -> std::string
{
  return value;
}

template<>
inline auto to_string(const std::string_view& value) -> std::string
{
  return std::string(value);
}

template<>
inline auto to_string(const bool& value) -> std::string
{
  return bool_to_string(value);
}

template<>
inline auto to_string(const Pixel& value) -> std::string
{
  return FMT::Rgba("{}", value);
}

template<>
inline auto to_string(const Point2dInt& value) -> std::string
{
  return FMT::Pt("{}", value);
}

template<>
inline auto to_string(const Point2dFlt& value) -> std::string
{
  return FMT::Pt("{:.2f}", value);
}
// NOLINTEND(readability-identifier-naming)

} // namespace GOOM::UTILS
