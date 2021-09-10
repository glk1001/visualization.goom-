#ifndef VISUALIZATION_GOOM_LIB_GOOMUTILS_STRUTILS_H_
#define VISUALIZATION_GOOM_LIB_GOOMUTILS_STRUTILS_H_

#include <string>
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

[[nodiscard]] auto bool_to_string(bool value) -> std::string;

[[nodiscard]] auto StringSplit(const std::string& str, const std::string& delim)
    -> std::vector<std::string>;

[[nodiscard]] auto StringJoin(const std::vector<std::string>& strings, const std::string& delim)
    -> std::string;

#if __cplusplus <= 201402L
} // namespace UTILS
} // namespace GOOM
#else
} // namespace GOOM::UTILS
#endif
#endif
