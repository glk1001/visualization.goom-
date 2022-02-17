#include "strutils.h"

#if __cplusplus > 201703L
#include <ranges>
#endif
#include <string>
#include <vector>

namespace GOOM::UTILS
{

auto bool_to_string(const bool value) -> std::string
{
  return value ? "true" : "false";
}

auto StringJoin(const std::vector<std::string>& strings, const std::string_view& delim)
    -> std::string
{
  std::string joinedStr{};

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
#if __cplusplus <= 201703L
  std::vector<std::string> vec;
  std::string token;
  std::string copyOfStr = str;
  size_t pos;
  while ((pos = copyOfStr.find(delim)) != std::string::npos)
  {
    token = copyOfStr.substr(0, pos);
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
