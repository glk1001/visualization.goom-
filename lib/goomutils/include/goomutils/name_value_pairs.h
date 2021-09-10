#ifndef VISUALIZATION_GOOM_LIB_GOOMUTILS_NAME_VALUE_PAIRS_H_
#define VISUALIZATION_GOOM_LIB_GOOMUTILS_NAME_VALUE_PAIRS_H_

#include "goom_graphic.h"
#include "strutils.h"

#include <algorithm>
#include <string>
#include <tuple>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace UTILS
{
#else
namespace GOOM::UTILS
{
#endif

using NameValuePair = std::pair<std::string, std::string>;
using NameValuePairs = std::vector<NameValuePair>;

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
  std::string fullParamGroup{};
  for (auto str = cbegin(paramGroups); str != cend(paramGroups); ++str)
  {
    if (str != cend(paramGroups) - 1)
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

[[nodiscard]] inline auto GetNameValueString(const NameValuePair& pair) -> std::string
{
  return pair.first + ": " + pair.second;
}

[[nodiscard]] inline auto GetNameValuesString(const NameValuePairs& nameValuePairs) -> std::string
{
  std::vector<std::string> nameValueStrings{};
  std::transform(cbegin(nameValuePairs), cend(nameValuePairs), std::back_inserter(nameValueStrings),
                 [](const NameValuePair& pair) { return GetNameValueString(pair); });
  return StringJoin(nameValueStrings, "\n");
}

template<typename T>
[[nodiscard]] auto GetPair(const std::string& paramGroup,
                           const std::string& paramName,
                           const T& paramValue) -> NameValuePair;

template<typename T>
[[nodiscard]] auto to_string(const T& value) -> std::string;


template<typename T>
inline auto GetPair(const std::string& paramGroup,
                    const std::string& paramName,
                    const T& paramValue) -> NameValuePair
{
  return {paramGroup + "::" + paramName, to_string(paramValue)};
}

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
inline auto to_string(const bool& value) -> std::string
{
  return bool_to_string(value);
}

template<>
inline auto to_string(const Pixel& value) -> std::string
{
  return value.ToString();
}

#if __cplusplus <= 201402L
} // namespace UTILS
} // namespace GOOM
#else
} // namespace GOOM::UTILS
#endif
#endif // VISUALIZATION_GOOM_LIB_GOOMUTILS_NAME_VALUE_PAIRS_H_
